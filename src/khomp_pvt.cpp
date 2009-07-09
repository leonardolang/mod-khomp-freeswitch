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
            try 
            {
                Globals::k3lapi.command(dev, obj, CM_DISCONNECT, NULL); 
            }
            catch(...) {}
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
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Checking if (%d-%d) is free\n", board, channel);
            try 
            {
                K3L_CHANNEL_CONFIG channelConfig;
                channelConfig = Globals::k3lapi.channel_config( board, channel );
            }
            catch (...)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Exception while retrieving channel config for board %d, channel%d!\n", board, channel);
                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
            K3L_CHANNEL_STATUS status;
            if (k3lGetDeviceStatus( board, channel + ksoChannel, &status, sizeof(status) ) != ksSuccess)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "k3lGetDeviceStatus failed to retrieve channel config for board %d, channel%d!\n", board, channel);
                switch_mutex_unlock(_pvts_mutex);
                return NULL;
            }
            if(status.CallStatus == kcsFree)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Channel (%d-%d) is free, let's check if the session is available ...\n", board, channel);
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
        Globals::k3lapi.mixerRecord(_target, 0, kmsNoDelayChannel, target().object);
        Globals::k3lapi.mixerRecord(_target, 1, kmsGenerator, kmtSilence);
	}

    try
    {
        Globals::k3lapi.command(_target, CM_LISTEN, (const char *) &buffer_size);
    }
    catch(...)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ERROR sending LISTEN command!\n");
        return false;
    }

    switch_set_flag_locked(this, TFLAG_LISTEN);
    
	return true;
}

bool CBaseKhompPvt::stop_listen(void)
{
    if(!switch_test_flag(this, TFLAG_LISTEN))
        return true;
    
    try
    {
        Globals::k3lapi.command(_target, CM_STOP_LISTEN);
    }
    catch(...)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ERROR sending STOP_LISTEN command!\n");
        return false;
    }

	switch_clear_flag_locked(this, TFLAG_LISTEN);

	return true;
}

bool CBaseKhompPvt::send_dtmf(char digit)
{
    try
    {
        Globals::k3lapi.command(_target, CM_SEND_DTMF, &digit);
    }
    catch(...)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ERROR sending SEND_DTMF command!\n");
        return false;
    }

    return true;
}

void CBaseKhompPvt::on_ev_new_call(K3L_EVENT * e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO,
                      "New call on %u to %s from %s. [EV_NEW_CALL]\n",
                      _target.object,
                      Globals::k3lapi.get_param(e, "dest_addr").c_str(),
                      Globals::k3lapi.get_param(e, "orig_addr").c_str());
    
    if (khomp_channel_from_event(_target.device, _target.object, e) != ksSuccess )
    {
        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_CRIT,
                          "Something bad happened while getting channel session. Device:%u/Channel:%u. [EV_NEW_CALL]\n",
                          _target.device, _target.object);
    }
    try 
    {
        Globals::k3lapi.command(_target.device, _target.object, CM_RINGBACK, NULL);
        Globals::k3lapi.command(_target.device, _target.object, CM_CONNECT, NULL); 
    }
    catch (K3LAPI::failed_command & err)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not set board channel status! [EV_NEW_CALL]\n");
    }
}

void CBaseKhompPvt::on_ev_disconnect(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_NOTICE,
                      "Called party disconnected: Khomp/%u/%u. [EV_DISCONNECT]\n",
                      _target.device,
                      _target.object);

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
    try
    {
        switch_core_session_t * session = CBaseKhompPvt::get(_target.device, _target.object)->session();
        switch_channel_t * channel = switch_core_session_get_channel(session);

        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_INFO,
                          "Call will be answered on Khomp/%u/%u. [EV_CONNECT]\n",
                          _target.device,
                          _target.object);

        channel_answer_channel(session);

    }
    catch (K3LAPI::failed_command & err)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_CRIT,
                          "Something bad happened while getting channel session. Khomp/%u/%u. [EV_CONNECT]\n",
                          _target.device,
                          _target.object);
    }
}

void CBaseKhompPvt::on_ev_call_success(K3L_EVENT *e)
{
    /* TODO: Should we bridge here? 
             Maybe check a certain variable if we should generate ringback?
     */
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      "Khomp/%u/%u is ringing. [EV_CALL_SUCESS]\n",
                      _target.device,
                      _target.object);
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
                          "Could not hangup channel: Khomp/%u/%u.\n",
                          _target.device,
                          _target.object);

    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      "Khomp/%u/%u is now free. [EV_CHANNEL_FREE]\n",
                      _target.device,
                      _target.object);
}

void CBaseKhompPvt::on_ev_no_answer(K3L_EVENT *e)
{
    /* TODO: Destroy sessions and channels */
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      "No one answered the call on Khomp/%u/%u. [EV_NO_ANSWER]\n",
                      _target.object,
                      _target.device);
}

void CBaseKhompPvt::on_ev_call_answer_info(K3L_EVENT *e)
{
    /* TODO: Set channel variable if we get this event */
    /* TODO: Fire an event so ESL can get it? */
    /* Call Analyser has to be enabled on k3lconfig */
    const char * startInfo = "";
    switch (e->AddInfo)
    {
        case (kcsiHumanAnswer):
            startInfo = "kcsiHumanAnswer";
            break;
        case (kcsiAnsweringMachine):
            startInfo = "kcsiAnsweringMachine";
            break;
        case (kcsiCellPhoneMessageBox):
            startInfo = "kcsiCellPhoneMessageBox";
            break;
        case (kcsiUnknown):
            startInfo = "kcsiUnknown";
            break;
        case (kcsiCarrierMessage):
            startInfo = "kcsiCarrierMessage";
            break;
        default:
            startInfo = "Error or unknown code!";

    }
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_DEBUG,
                      "Detected: \"%s\" on Khomp/%u/%u. [EV_CALL_ANSWER_INFO]\n",
                      startInfo,
                      _target.device,
                      _target.object);
}

void CBaseKhompPvt::on_ev_dtmf_detected(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_INFO,
                      "Detected DTMF (%c) on Khomp/%u/%u. [EV_DTMF_DETECTED]\n",
                      e->AddInfo,
                      _target.device,
                      _target.object);

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
                          "Received a DTMF for (Khomp/%u/%u) but the channel is invalid !\n",
                          _target.device,
                          _target.object);
    }
}

void CBaseKhompPvt::on_ev_internal_fail(K3L_EVENT *e)
{
    // TODO: use Verbose for this.
    const char * msg = "";
    switch(e->AddInfo)
    {
        case kifInterruptCtrl:
            msg = "kifInterruptCtrl";
            break;
        case kifCommunicationFail:
            msg = "kifCommunicationFail";
            break;
        case kifProtocolFail:
            msg = "kifProtocolFail";
            break;
        case kifInternalBuffer:
            msg = "kifInternalBuffer";
            break;
        case kifMonitorBuffer:
            msg = "kifMonitorBuffer";
            break;
        case kifInitialization:
            msg = "kifInitialization";
            break;
        case kifInterfaceFail:
            msg = "kifInterfaceFail";
            break;
        case kifClientCommFail:
            msg = "kifClientCommFail";
            break;
        default:
            msg = "UnknownError";
    }
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "This is a fatal error and we will not recover. Reason: %s. [EV_INTERNAL_FAIL]\n", msg);
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
void CBaseKhompPvt::on_ev_link_status(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_physical_link_down(K3L_EVENT *e){}
void CBaseKhompPvt::on_ev_physical_link_up(K3L_EVENT *e){}

void CBaseKhompPvt::on_ev_untreated(K3L_EVENT *e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_DEBUG,
                      "New Event has just arrived on Khomp/%u/%u with untreated code: %x\n",
                      _target.device,
                      _target.object,
                      e->Code);
}

extern "C" int32 Kstdcall khomp_event_callback(int32 obj, K3L_EVENT * e)
{                

    switch_log_printf(SWITCH_CHANNEL_LOG,
                      SWITCH_LOG_DEBUG,
                      "New Khomp Event: on %u/%u: %x\n",
                      e->DeviceId,
                      obj,
                      e->Code);
    
    switch(e->Code)
    {
        case EV_NEW_CALL:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_new_call(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_NEW_CALL]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }

        case EV_DISCONNECT:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_disconnect(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_DISCONNECT]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }

        case EV_CONNECT:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_connect(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_CONNECT]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }

        case EV_CALL_SUCCESS:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_call_success(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_CALL_SUCCESS]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }         

        case EV_CHANNEL_FREE:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_channel_free(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_CHANNEL_FREE]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }  


        case EV_NO_ANSWER:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_no_answer(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_NO_ANSWER]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }

        case EV_CALL_ANSWER_INFO:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_call_answer_info(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_CALL_ANSWER_INFO]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }

        case EV_DTMF_DETECTED:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_dtmf_detected(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_DTMF_DETECTED]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }

        case EV_DTMF_SEND_FINISH:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Channel %u on board %u has sucessfully generated DTMF. [EV_DTMF_SEND_FINNISH]\n", obj, e->DeviceId);
            break;
        case EV_CALL_FAIL:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Channel %u on board %u reported call fail. [EV_CALL_FAIL]\n", obj, e->DeviceId);
            break;
        case EV_CHANNEL_FAIL:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Channel %u on board %u reported failure. [EV_CHANNEL_FAIL]\n", obj, e->DeviceId);
            break;
        case EV_LINK_STATUS:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Link %u on board %u changed. [EV_LINK_STATUS]\n", e->DeviceId, obj);
            break;
        case EV_PHYSICAL_LINK_DOWN:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Link %u on board %u is DOWN. [EV_PHYSICAL_LINK_DOWN]\n", e->DeviceId, obj);
            break;
        case EV_PHYSICAL_LINK_UP:
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Link %u on board %u is UP. [EV_PHYSICAL_LINK_UP]\n", e->DeviceId, obj);
            break;

        case EV_INTERNAL_FAIL:
        {
            //TODO: Does it make any sense for we to ::get like this?
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_internal_fail(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [EV_INTERNAL_FAIL]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }

        default:
        {
            try
            {
                CBaseKhompPvt::get(e->DeviceId, obj)->on_ev_untreated(e);
            }
            catch (K3LAPI::invalid_channel)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_CRIT,
                                  "Could not get channel session! Khomp/%u/%u is an invalid channel. [UNTREATED EVENT]\n",
                                  e->DeviceId,
                                  obj);
            }
            break;
        }
    }

    return ksSuccess;
}
