/*  
    KHOMP endpoint driver for FreeSWITCH library.
    Copyright (C) 2007-2009 Khomp Ind. & Com.  
  
  The contents of this file are subject to the Mozilla Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.
  
*/

#include "khomp_pvt.h"
#include "khomp_pvt_kxe1.h"


CBaseKhompPvt::VectorType  CBaseKhompPvt::_pvts;
switch_mutex_t *           CBaseKhompPvt::_pvts_mutex;
char                       CBaseKhompPvt::_cng_buffer[128];

CBaseKhompPvt::CBaseKhompPvt(K3LAPI::target & target)
: _target(target), _session(NULL),
  _reader_frames(&_read_codec),
  _writer_frames(&_write_codec)
{

}

CBaseKhompPvt::~CBaseKhompPvt()
{
    _session = NULL;
};

bool CBaseKhompPvt::initialize_k3l(void)
{
    /* Start the API and connect to KServer */
    try
    {
        Globals::k3lapi.start();
    }
    catch (K3LAPI::start_failed & e)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "K3L not started. Reason:%s.\n", e.msg.c_str());
        return false;
    }
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "K3L started.\n");
    return true;
}

bool CBaseKhompPvt::initialize_handlers(void)
{
    if (Globals::k3lapi.device_count() == 0)
        return false;

    k3lRegisterEventHandler( khomp_event_callback );
    k3lRegisterAudioListener( NULL, khomp_audio_listener );

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "K3l event and audio handlers registered.\n");

    return true;

}

void CBaseKhompPvt::initialize_channels(void)
{
    
    for (unsigned dev = 0; dev < Globals::k3lapi.device_count(); dev++)
    {
        _pvts.push_back(InnerVectorType());

        for (unsigned obj = 0; obj < Globals::k3lapi.channel_count(dev); obj++)
        {
            K3LAPI::target tgt(Globals::k3lapi, K3LAPI::target::CHANNEL, dev, obj);

            /* TODO; We have to initialize a proper Pvt here */
            CBaseKhompPvt * pvt = new CKhompPvtE1(tgt);
            _pvts.back().push_back(pvt);

            /* TODO: remove this from here */
            pvt->command(KHOMP_LOG, CM_DISCONNECT);
        }
    }
}

void CBaseKhompPvt::initialize_cng_buffer(void)
{
    bool turn = true;

    for (unsigned int i = 0; i < Globals::cng_buffer_size; i++)
    {
        _cng_buffer[i] = (turn ? 0xD5 : 0xD4);
        turn = !turn;
    }
}

bool CBaseKhompPvt::initialize(void)
{

    if(!initialize_k3l())
        return false;

    switch_mutex_init(&_pvts_mutex, SWITCH_MUTEX_NESTED, Globals::module_pool);

    initialize_cng_buffer();
    initialize_channels();


    return true;
}
void CBaseKhompPvt::terminate(void)
{
    switch_mutex_lock(_pvts_mutex);

    for (VectorType::iterator it_dev = _pvts.begin(); it_dev != _pvts.end(); it_dev++)
    {
        InnerVectorType & obj_vec = *it_dev;

        for (InnerVectorType::iterator it_obj = obj_vec.begin(); it_obj != obj_vec.end(); it_obj++)
        {
            CBaseKhompPvt * pvt = *it_obj;
            delete pvt;
        }
    }
}

void CBaseKhompPvt::khomp_add_event_board_data(switch_event_t *event)
{

    //if (!event) {
        //TODO: RAISE!
    //}

    if (_session)
    {
        switch_channel_t *channel = switch_core_session_get_channel(_session);
        switch_channel_event_set_data(channel, event);
    }

    switch_event_add_header(event, SWITCH_STACK_BOTTOM, "Khomp-DeviceId", "%u", _target.device);
    switch_event_add_header(event, SWITCH_STACK_BOTTOM, "Khomp-Object", "%u", _target.object);
}

switch_status_t CBaseKhompPvt::init(switch_core_session_t *new_session)
{
    session(new_session);

    if (switch_core_codec_init(&_read_codec, "PCMA", NULL, 8000, Globals::switch_packet_duration, 1,
            SWITCH_CODEC_FLAG_ENCODE | SWITCH_CODEC_FLAG_DECODE, NULL,
                Globals::module_pool) != SWITCH_STATUS_SUCCESS)
    {
        return SWITCH_STATUS_FALSE;
    }

    if (switch_core_codec_init(&_write_codec, "PCMA", NULL, 8000, Globals::switch_packet_duration, 1,
            SWITCH_CODEC_FLAG_ENCODE | SWITCH_CODEC_FLAG_DECODE, NULL,
                Globals::module_pool) != SWITCH_STATUS_SUCCESS)
    {
        return SWITCH_STATUS_FALSE;
    }
    
    switch_mutex_init(&flag_mutex, SWITCH_MUTEX_NESTED,
                switch_core_session_get_pool(_session));
    //switch_mutex_destroy, where???

    switch_core_session_set_private(_session, this);

    if((switch_core_session_set_read_codec(_session, &_read_codec) !=
                SWITCH_STATUS_SUCCESS) ||
       (switch_core_session_set_write_codec(_session, &_write_codec) !=
                SWITCH_STATUS_SUCCESS))
    {
        return SWITCH_STATUS_FALSE;
    }

    switch_set_flag_locked(this, (TFLAG_CODEC | TFLAG_IO));

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t CBaseKhompPvt::clear()
{
    flags = 0;

    _reader_frames.clear();
    _writer_frames.clear();

    if (_read_codec.implementation) 
    {
        switch_core_codec_destroy(&_read_codec);
    }

    if (_write_codec.implementation)
    {
        switch_core_codec_destroy(&_write_codec);
    }

    session(NULL);
}


CBaseKhompPvt * CBaseKhompPvt::find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause)
{
    char *argv[3] = { 0 };
    int argc = 0;
    
    int board_low = 0;
    int board_high = 0;

    int channel_low = 0;
    int channel_high = 0;
    
    bool first_channel_available = true;
    bool reverse_first_board_available = false;
    bool reverse_first_channel_available = false;

    *cause = SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;

    CBaseKhompPvt * pvt = NULL;
    
    /* Let's setup our own vars on tech_pvt */
    if ((argc = switch_separate_string(allocation_string, '/', argv, (sizeof(argv) / sizeof(argv[0])))) < 3)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
                "Invalid dial string (%s). Should be on the format:[Khomp/BoardID (or A for first free board)/CHANNEL (or A for first free channel)]\n",
                allocation_string);
        return NULL;
    }

    if(*argv[0] == 'A' || *argv[0] == 'a')
    {
        board_low = 0;
        board_high = Globals::k3lapi.device_count();
        // Lets make it reverse...
        if(*argv[0] == 'a')
        {
            reverse_first_board_available = true;
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Doing reverse board lookup!\n");
        }
    }
    else
    {
        board_low = atoi(argv[0]);
        board_high = board_low;
    }

    if(*argv[1] == 'A' || *argv[1] == 'a')
    {
        first_channel_available = true;
        // Lets make it reverse...
        if(*argv[1] == 'a')
        {
            reverse_first_channel_available = true;
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Doing reverse channel lookup!\n");
        }
    }
    else
    {
        first_channel_available = false;
        channel_low = atoi(argv[0]);
        channel_high = board_low;
    }

    /* Sanity checking */
    if(board_low < 0 || board_high > Globals::k3lapi.device_count())
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid board selection (%d-%d) !\n", board_low, board_high);
        return NULL;            
    }

    switch_mutex_lock(_pvts_mutex);

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Channel selection: board (%d-%d), channel (%d-%d)!\n", board_low, board_high, channel_low, channel_high);
    
    int board = board_low;
    for ((reverse_first_board_available == false ? board = board_low : board = board_high-1);
            (reverse_first_board_available == false ? board <= board_high : board >= board_low);
            (reverse_first_board_available == false ? board++ : board--))
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Checking board %d\n", board);
        
        if(pvt != NULL)
            break;
        
        if(first_channel_available)
        {
            channel_low = 0;
            channel_high = Globals::k3lapi.channel_count(board);
        }
        else
        {
            if(channel_low < 0 || channel_high > Globals::k3lapi.channel_count(board))
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid channel selection (%d-%d) !\n", channel_low, channel_high);
                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
        }

        int channel = channel_low;
        for ((reverse_first_channel_available == true ? channel = channel_high-1 : channel = channel_low);
                (reverse_first_channel_available == true ? channel >= channel_low : channel <= channel_high);
                (reverse_first_channel_available == true ? channel-- : channel++)) 
        {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
					OBJ_MSG(board, channel, "Checking if channel is free\n")) ;

            try 
            {
                K3L_CHANNEL_CONFIG channelConfig;
                channelConfig = Globals::k3lapi.channel_config( board, channel );
            }
            catch (...)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
                    OBJ_MSG(board, channel, "Exception while retrieving channel config!\n")); 

                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
            K3L_CHANNEL_STATUS status;
            if (k3lGetDeviceStatus( board, channel + ksoChannel, &status, sizeof(status) ) != ksSuccess)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
                        OBJ_MSG(board, channel, "k3lGetDeviceStatus failed to retrieve channel config!\n"));
                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
            if(status.CallStatus == kcsFree)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
                        OBJ_MSG(board, channel, "is free, checking if the session is available...\n")); 

                pvt = CBaseKhompPvt::get(board, channel);
                if(pvt != NULL && pvt->session() == NULL)
                {
                    pvt->session(new_session);
                    break;
                }
                pvt = NULL;
            }
        }
    }

    if(pvt != NULL)
    {
        *cause = SWITCH_CAUSE_SUCCESS;
    }
    else
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No channels available\n");
        *cause = SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION;
    }
    
    switch_mutex_unlock(_pvts_mutex);
    return pvt;
}

/* Helper functions - based on code from chan_khomp */

bool CBaseKhompPvt::start_stream(void)
{
    if (switch_test_flag(this, TFLAG_STREAM))
        return true;
    
    try
    {
        Globals::k3lapi.mixer(_target, 0, kmsPlay, _target.object);
        Globals::k3lapi.command(_target, CM_START_STREAM_BUFFER);
    }
    catch(...)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ERROR sending START_STREAM_BUFFER command!\n");
        return false;
    }

    switch_set_flag_locked(this, TFLAG_STREAM);
    
    return true;
}

bool CBaseKhompPvt::stop_stream(void)
{
    if (!switch_test_flag(this, TFLAG_STREAM))
        return true;
    
    try
    {
        Globals::k3lapi.mixer(_target, 0, kmsGenerator, kmtSilence);
        Globals::k3lapi.command(_target, CM_STOP_STREAM_BUFFER);
    }
    catch(...)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ERROR sending STOP_STREAM_BUFFER command!\n");
        return false;
    }

    switch_clear_flag_locked(this, TFLAG_STREAM);
    
    return true;
}

bool CBaseKhompPvt::start_listen(bool conn_rx)
{
    if (switch_test_flag(this, TFLAG_LISTEN))
        return true;
    
    const size_t buffer_size = Globals::boards_packet_duration;

    if (conn_rx)
    {
        try
        {
            Globals::k3lapi.mixerRecord(_target, 0, kmsNoDelayChannel, target().object);
            Globals::k3lapi.mixerRecord(_target, 1, kmsGenerator, kmtSilence);
        }
        catch(K3LAPI::failed_command & e)
        {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ERROR sending mixer record command!\n");
        }
    }

    if(!command(KHOMP_LOG, CM_LISTEN, (const char *) &buffer_size))
    {
        return false;
    }

    switch_set_flag_locked(this, TFLAG_LISTEN);
    
    return true;
}

bool CBaseKhompPvt::stop_listen(void)
{
    if(!switch_test_flag(this, TFLAG_LISTEN))
        return true;
   
    if(!command(KHOMP_LOG, CM_STOP_LISTEN))
    {
        return false;
    }

    switch_clear_flag_locked(this, TFLAG_LISTEN);

    return true;
}

bool CBaseKhompPvt::send_dtmf(char digit)
{
    return command(KHOMP_LOG, CM_SEND_DTMF, &digit);
        
}

void CBaseKhompPvt::on_ev_new_call(K3L_EVENT * e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO,
                      PVT_MSG(_target, "New call to '%s' from '%s'.\n"),
                      Globals::k3lapi.get_param(e, "dest_addr").c_str(),
                      Globals::k3lapi.get_param(e, "orig_addr").c_str());
    
    if (!khomp_channel_from_event(_target, e))
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT,
                PVT_MSG(_target, "Something bad happened while getting channel session.\n"));
		return;
    }

    command(KHOMP_LOG, CM_RINGBACK);
   	command(KHOMP_LOG, CM_CONNECT);
}

void CBaseKhompPvt::on_ev_disconnect(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_NOTICE,
                      PVT_MSG(_target, "Called party disconnected.\n"));

    // TODO: before uncommenting: we need to check channel direction here. after that, we MAY ONLY
    //       send a CM_DISCONNECT, but should not clean any state associated with the call
#if 0
    switch_core_session_t * session = CBaseKhompPvt::get(e->DeviceId, obj)->session();

    if(session == NULL)
        break;

    if (channel_on_hangup(session) != SWITCH_STATUS_SUCCESS)
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not hangup channel: %u on board %u. Releasing board channel anyway. [EV_DISCONNECT]\n", obj, e->DeviceId);
#endif

    // TODO: handle KGSM Multiparty disconnect latter.
}

void CBaseKhompPvt::on_ev_connect(K3L_EVENT *e)
{
    switch_core_session_t * session = CBaseKhompPvt::get(_target.device, _target.object)->session();
    //switch_channel_t * channel = switch_core_session_get_channel(session);

    switch_log_printf(SWITCH_CHANNEL_LOG,
            SWITCH_LOG_INFO,
            PVT_MSG(_target, "Call will be answered.\n"));

    channel_answer_channel(session);
}

void CBaseKhompPvt::on_ev_call_success(K3L_EVENT *e)
{
    /* TODO: Should we bridge here? 
             Maybe check a certain variable if we should generate ringback?
     */
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      PVT_MSG(_target, "is ringing.\n"));
}

void CBaseKhompPvt::on_ev_channel_free(K3L_EVENT *e)
{
    if(!_session) 
    {
        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_WARNING,
                          "Session is invalid\n");
        return;
    }

    if (channel_on_hangup(_session) != SWITCH_STATUS_SUCCESS)
        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_CRIT,
                          PVT_MSG(_target, "Could not hangup channel.\n"));

    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      PVT_MSG(_target, "Now is free.\n"));
}

void CBaseKhompPvt::on_ev_no_answer(K3L_EVENT *e)
{
    /* TODO: Destroy sessions and channels */
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      PVT_MSG(_target, "No one answered the call.\n"));
}

void CBaseKhompPvt::on_ev_call_answer_info(K3L_EVENT *e)
{
    /* TODO: Set channel variable if we get this event */
    /* TODO: Fire an event so ESL can get it? */
    /* Call Analyser has to be enabled on k3lconfig */
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_DEBUG,
                      PVT_MSG(_target, "Detected: \"%s\".\n"),
                      Verbose::callStartInfo((KCallStartInfo)e->AddInfo).c_str());

    /* Fire a custom event about this */
    switch_event_t * event;
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, KHOMP_EVENT_MAINT) == SWITCH_STATUS_SUCCESS)
    {
        khomp_add_event_board_data(event);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, 
                "EV_CALL_ANSWER_INFO", 
                Verbose::callStartInfo((KCallStartInfo)e->AddInfo).c_str());
        switch_event_fire(&event);
    }
}

void CBaseKhompPvt::on_ev_dtmf_detected(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      PVT_MSG(_target, "Detected DTMF (%c).\n"),
                      e->AddInfo);

    if(!_session) 
    {
        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_DEBUG,
                          "Session is invalid\n");
        return;
    }

    switch_channel_t *channel = switch_core_session_get_channel(_session);

    if(channel)
    {
        switch_dtmf_t dtmf = { (char) e->AddInfo, switch_core_default_dtmf_duration(0) };
        switch_channel_queue_dtmf(channel, &dtmf);
    }
    else
    {
        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_ERROR,
                          PVT_MSG(_target, "Received a DTMF but the channel is invalid !\n"));
    }
}

void CBaseKhompPvt::on_ev_internal_fail(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
            "This is a fatal error and we will not recover. Reason: %s.\n", 
            Verbose::internalFail((KInternalFail)e->AddInfo).c_str());
}

void CBaseKhompPvt::on_ev_seizure_start(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_call_hold_start(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_call_hold_stop(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_ss_transfer_fail(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_ring_detected(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_prolarity_reversal(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_collect_call(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_cas_mfc_recv(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_cas_line_stt_changed(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_pulse_detected(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_flash(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_billing_pulse(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_audio_status(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_cadence_recognized(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_dtmf_send_finnish(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_end_of_stream(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_user_information(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_isdn_progess_indicator(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_isdn_subaddress(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_dialed_digit(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_recv_from_modem(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_new_sms(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_sms_info(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_sms_data(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_sms_send_result(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_call_fail(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_reference_fail(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_channel_fail(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_client_reconnect(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_link_status(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, 
                      "Link %02hu on board %02hu changed.\n", 
                      e->AddInfo, e->DeviceId);
    /* Fire a custom event about this */
    switch_event_t * event;
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, KHOMP_EVENT_MAINT) == SWITCH_STATUS_SUCCESS)
    {
        khomp_add_event_board_data(event);
        switch_event_add_header(event, SWITCH_STACK_BOTTOM, "EV_LINK_STATUS", "%d", e->AddInfo);
        switch_event_fire(&event);
    }
}
void CBaseKhompPvt::on_ev_physical_link_down(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_physical_link_up(K3L_EVENT *e){}

void CBaseKhompPvt::on_ev_untreated(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_DEBUG,
                      PVT_MSG(_target, "New Event has just arrived with untreated code: %x\n"),
                      e->Code);
}

extern "C" int32 Kstdcall khomp_event_callback(int32 obj, K3L_EVENT * e)
{                

    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_DEBUG,
                      "%s\n",
                      Globals::verbose.event(obj, e).c_str());
    try
    {

        switch(e->Code)
        {
        case EV_NEW_CALL:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_new_call(e);
            break;

        case EV_DISCONNECT:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_disconnect(e);
            break;

        case EV_CONNECT:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_connect(e);
            break;

        case EV_CALL_SUCCESS:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_call_success(e);
            break;

        case EV_CHANNEL_FREE:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_channel_free(e);
            break;

        case EV_NO_ANSWER:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_no_answer(e);
            break;

        case EV_CALL_ANSWER_INFO:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_call_answer_info(e);
            break;

        case EV_DTMF_DETECTED:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_dtmf_detected(e);
            break;

        case EV_DTMF_SEND_FINISH:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, 
                    OBJ_MSG(e->DeviceId, obj, "has sucessfully generated DTMF.\n"));
            break;

        case EV_CALL_FAIL:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, 
                    OBJ_MSG(e->DeviceId, obj, "reported call fail.\n"));
            break;

        case EV_CHANNEL_FAIL:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
                    OBJ_MSG(e->DeviceId, obj, "reported failure.\n"));
            break;

        case EV_LINK_STATUS:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_link_status(e);
            break;

        case EV_PHYSICAL_LINK_DOWN:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, 
                    "Link %02hu on board %02hu is DOWN.\n", 
                    e->AddInfo, e->DeviceId);
            break;

        case EV_PHYSICAL_LINK_UP:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, 
                    "Link %02hu on board %02hu is UP\n", 
                    e->AddInfo, e->DeviceId);
            break;

        case EV_INTERNAL_FAIL:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_internal_fail(e);
            break;

        case EV_AUDIO_STATUS: 
            break;

        default:
            CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_untreated(e);
            break;
        }
    }
    catch (K3LAPI::invalid_channel & invalid)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG,
        SWITCH_LOG_CRIT,
        OBJ_MSG(e->DeviceId, obj, "invalid channel on event '%s'.\n"),
        Verbose::eventName(e->Code).c_str());
    }

    return ksSuccess;
}

