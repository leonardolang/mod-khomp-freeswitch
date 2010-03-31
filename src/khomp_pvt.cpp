/*******************************************************************************

    KHOMP generic endpoint/channel library.
    Copyright (C) 2007-2010 Khomp Ind. & Com.

  The contents of this file are subject to the Mozilla Public License 
  Version 1.1 (the "License"); you may not use this file except in compliance 
  with the License. You may obtain a copy of the License at 
  http://www.mozilla.org/MPL/ 

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.

  Alternatively, the contents of this file may be used under the terms of the
  "GNU Lesser General Public License 2.1" license (the “LGPL" License), in which
  case the provisions of "LGPL License" are applicable instead of those above.

  If you wish to allow use of your version of this file only under the terms of
  the LGPL License and not to allow others to use your version of this file 
  under the MPL, indicate your decision by deleting the provisions above and 
  replace them with the notice and other provisions required by the LGPL 
  License. If you do not delete the provisions above, a recipient may use your 
  version of this file under either the MPL or the LGPL License.

  The LGPL header follows below:

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library; if not, write to the Free Software Foundation, 
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*******************************************************************************/

#include "khomp_pvt.h"
#include "lock.h"
#include "khomp_pvt_kxe1.h"

Board::VectorBoard  Board::_boards;
switch_mutex_t *    Board::_pvts_mutex;
char                Board::_cng_buffer[128];

Board::KhompPvt::KhompPvt(K3LAPI::target & target) :
  _target(target),
  _mutex(Globals::module_pool),
  _session(NULL),
  _caller_profile(NULL),
  _reader_frames(&_read_codec),
  _writer_frames(&_write_codec) {}

bool Board::initializeK3L(void)
{
    K::Logger::Logg(C_MESSAGE,"starting K3L API ..."); 

    /* Start the API and connect to KServer */
    k3lSetGlobalParam (klpResetFwOnStartup, 1);
    k3lSetGlobalParam (klpDisableInternalVoIP, 1);

    try
    {
        Globals::k3lapi.start();
    }
    catch (K3LAPI::start_failed & e)
    {
        K::Logger::Logg(C_ERROR,FMT("loading K3L API failed: %s") % e.msg);
        return false;
    }

    K::Logger::Logg(C_MESSAGE,"the K3L API have been started!"); 
    
    return true;
}

bool Board::finalizeK3L(void)
{
    /* Stop the API and disconnect to KServer */
    try
    {
        Globals::k3lapi.stop();
    }
    catch(...)
    {
        K::Logger::Logg(C_ERROR,"K3L not stopped");
        return false;
    }
    K::Logger::Logg(C_MESSAGE,"K3L stopped.."); 
    return true;
}

bool Board::initializeHandlers(void)
{
    if (Globals::k3lapi.device_count() == 0)
        return false;

    for (VectorBoard::iterator it_dev = _boards.begin();
                               it_dev != _boards.end();
                               it_dev++)
    {
        Board * device = *it_dev;
        device->_event_handler = new ChanEventHandler(device->id(), &eventThread);
        device->_command_handler = new ChanCommandHandler(device->id(), &commandThread);
    }

    k3lRegisterEventHandler( khomp_event_callback );
    k3lRegisterAudioListener( NULL, khomp_audio_listener );

    K::Logger::Logg(C_MESSAGE,"K3l event and audio handlers registered."); 

    return true;
}

bool Board::finalizeHandlers(void)
{
    if (Globals::k3lapi.device_count() == 0)
        return false;

    k3lRegisterEventHandler( NULL );
    k3lRegisterAudioListener( NULL, NULL );


    for (VectorBoard::iterator it_dev = _boards.begin();
                               it_dev != _boards.end();
                               it_dev++)
    {
        Board * device = *it_dev;
        // stop event handler for device
        ChanEventHandler * evt_handler = device->_event_handler;
        evt_handler->fifo()->_shutdown = true;
        evt_handler->signal();
        delete evt_handler;
        device->_event_handler = NULL;
        // stop command handler for device
        ChanCommandHandler * cmd_handler = device->_command_handler;
        cmd_handler->fifo()->_shutdown = true;
        cmd_handler->signal();
        delete cmd_handler;
        device->_command_handler = NULL;
    }

    /* wait every thread to finalize */
    sleep(1);

    K::Logger::Logg(C_MESSAGE,"K3l event and audio handlers unregistered."); 

    return true;

}

void Board::initializeBoards(void)
{

    for (unsigned dev = 0; dev < Globals::k3lapi.device_count(); dev++)
    {
        K::Logger::Logg(C_MESSAGE,FMT("loading device %d..." ) % dev); 

        switch (Globals::k3lapi.device_type(dev))
        {
            case kdtE1:
            case kdtE1Spx:
                _boards.push_back(new BoardE1(dev));
                break;
            default:
                _boards.push_back(new Board(dev));
                K::Logger::Logg(C_ERROR,FMT("device type %d unknown" ) %  Globals::k3lapi.device_type(dev)); 
                break;
        }
        
        _boards.back()->initializeChannels();
    }
}

void Board::initializeChannels(void)
{
    K::Logger::Logg(C_MESSAGE,"loading channels ..."); 

    for (unsigned obj = 0; obj < Globals::k3lapi.channel_count(_device_id); obj++)
    {
        K3LAPI::target tgt(Globals::k3lapi, K3LAPI::target::CHANNEL, _device_id, obj);

        KhompPvt * pvt;

		switch (Globals::k3lapi.channel_config(_device_id, obj).Signaling)
		{
            CASE_RDSI_SIG:
                pvt = new BoardE1::KhompPvtISDN(tgt);
                pvt->_call = new BoardE1::KhompPvtISDN::CallISDN();
                DBG(FUNC, "ISDN channel"); 
                break;
            CASE_R2_SIG:
                pvt = new BoardE1::KhompPvtR2(tgt);
                pvt->_call = new BoardE1::KhompPvtR2::CallR2();
                pvt->command(KHOMP_LOG, CM_DISCONNECT);
                DBG(FUNC, "R2 channel"); 
                break;
            default:
                pvt = new Board::KhompPvt(tgt);
                pvt->_call = new Board::KhompPvt::Call();
                K::Logger::Logg(C_ERROR,FMT("signaling %d unknown") % Globals::k3lapi.channel_config(_device_id, obj).Signaling);
                break;
        }
        
        _channels.push_back(pvt);

        pvt->cleanup();
    }
}


void Board::finalizeBoards(void)
{
    K::Logger::Logg(C_MESSAGE,"finalizing boards ..."); 
    switch_mutex_lock(_pvts_mutex);

    for (VectorBoard::iterator it_dev = _boards.begin();
                               it_dev != _boards.end();
                               it_dev++)
    {
        Board * & device_ref = *it_dev;
        Board *   device_ptr = device_ref;

        device_ptr->finalizeChannels();

        device_ref = (Board *) NULL;

        delete device_ptr;
    }

    switch_mutex_unlock(_pvts_mutex);
}

void Board::finalizeChannels()
{
    K::Logger::Logg(C_MESSAGE,"finalizing channels ..."); 
    for (VectorChannel::iterator it_obj = _channels.begin();
         it_obj != _channels.end();
         it_obj++)
    {
        KhompPvt * & pvt_ref = *it_obj;
        KhompPvt *   pvt_ptr = pvt_ref;

        if(!pvt_ptr)
            continue;

        try
        {
            ScopedPvtLock lock(pvt_ptr);

            if(pvt_ptr->session())
            {
                //TODO: Tratamento para desconectar do canal do FreeSwitch.
                pvt_ptr->cleanup();
            }

            delete pvt_ptr->_call;
            pvt_ptr->_call = NULL;

            pvt_ref = (KhompPvt *) NULL;
        }
        catch(...)
        {
            /* keep walking! */
        }

        delete pvt_ptr;
    }
}

void Board::initializeCngBuffer(void)
{
    bool turn = true;

    for (unsigned int i = 0; i < Globals::cng_buffer_size; i++)
    {
        _cng_buffer[i] = (turn ? 0xD5 : 0xD4);
        turn = !turn;
    }
}

bool Board::initialize(void)
{
    //K::Logger::Logg(C_MESSAGE,""); 

    if(!initializeK3L())
        return false;

    switch_mutex_init(&_pvts_mutex, SWITCH_MUTEX_NESTED, Globals::module_pool);

    initializeCngBuffer();

    initializeBoards();

    return true;
}

bool Board::finalize(void)
{
    finalizeBoards();

    switch_mutex_destroy(_pvts_mutex);

    return finalizeK3L();
}

void Board::khomp_add_event_board_data(const K3LAPI::target target, switch_event_t *event)
{

    //if (!event) {
        //TODO: RAISE!
    //}

    if (target.type == K3LAPI::target::CHANNEL)
    {
        switch_core_session_t * s = get(target.device, target.object)->session();
        if(s)
        {
            switch_channel_t *chan = switch_core_session_get_channel(s);
            switch_channel_event_set_data(chan, event);
        }
    }

    switch_event_add_header(event, SWITCH_STACK_BOTTOM, "Khomp-DeviceId", "%u", target.device);
    switch_event_add_header(event, SWITCH_STACK_BOTTOM, "Khomp-Object", "%u", target.object);
}

switch_status_t Board::KhompPvt::justAlloc(bool is_answering, switch_memory_pool_t **pool)
{
    DBG(FUNC, PVT_FMT(target(), "c"));

    if(is_answering)
    {
        /* Create a new session on incoming call */
        call()->_flags.set(Kflags::IS_INCOMING);
        session(switch_core_session_request(Globals::khomp_endpoint_interface, SWITCH_CALL_DIRECTION_INBOUND, NULL));
    }
    else
    {
        /* Create a new session on outgoing call */
        call()->_flags.set(Kflags::IS_OUTGOING);
        //session(switch_core_session_request(Globals::khomp_endpoint_interface, SWITCH_CALL_DIRECTION_OUTBOUND, &Globals::module_pool));
        session(switch_core_session_request(Globals::khomp_endpoint_interface, SWITCH_CALL_DIRECTION_OUTBOUND, pool));
    }

    if(!session())
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (Initilization Error, session not created!)"));
        return SWITCH_STATUS_FALSE;
    }

    switch_core_session_add_stream(session(), NULL);

    if (switch_core_codec_init(&_read_codec, "PCMA", NULL, 8000, Globals::switch_packet_duration, 1,
            SWITCH_CODEC_FLAG_ENCODE | SWITCH_CODEC_FLAG_DECODE, NULL,
                Globals::module_pool) != SWITCH_STATUS_SUCCESS)
    {
        switch_core_session_destroy(&_session);
        session(NULL);

        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (Error while init read codecs)"));
        return SWITCH_STATUS_FALSE;
    }

    if (switch_core_codec_init(&_write_codec, "PCMA", NULL, 8000, Globals::switch_packet_duration, 1,
            SWITCH_CODEC_FLAG_ENCODE | SWITCH_CODEC_FLAG_DECODE, NULL,
                Globals::module_pool) != SWITCH_STATUS_SUCCESS)
    {
        switch_core_session_destroy(&_session);
        session(NULL);
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (Error while init write codecs)"));
        return SWITCH_STATUS_FALSE;
    }

    //TODO: Retirar daqui
    switch_mutex_init(&flag_mutex, SWITCH_MUTEX_NESTED,
                switch_core_session_get_pool(_session));

    switch_core_session_set_private(_session, this);

    if((switch_core_session_set_read_codec(_session, &_read_codec) !=
                SWITCH_STATUS_SUCCESS) ||
       (switch_core_session_set_write_codec(_session, &_write_codec) !=
                SWITCH_STATUS_SUCCESS))
    {
        switch_core_session_destroy(&_session);
        session(NULL);
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (Error while set read codecs)"));
        return SWITCH_STATUS_FALSE;
    }

    //switch_set_flag_locked(this, (TFLAG_CODEC | TFLAG_IO));

    DBG(FUNC, PVT_FMT(target(), "r"));
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t Board::KhompPvt::justStart(switch_caller_profile_t *profile)
{
    DBG(FUNC, PVT_FMT(target(), "c"));

    switch_channel_t *channel = NULL;

    if(!session())
        return SWITCH_STATUS_FALSE;

    channel = switch_core_session_get_channel(session());

    if(!channel)
    {
        DBG(FUNC, PVT_FMT(target(), "r (channel == NULL)"));
        return SWITCH_STATUS_FALSE;
    }

    if(call()->_flags.check(Kflags::IS_INCOMING))
    {

        _caller_profile = switch_caller_profile_new(switch_core_session_get_pool(_session),
                "Khomp",
                "XML", // TODO: Dialplan module to use?
                NULL,
                NULL,
                NULL,
                _call->_orig_addr.c_str(),
                NULL,
                NULL,
                (char *) "mod_khomp",//modname, TODO
                "default", // TODO: Context to look for on the dialplan?
                _call->_dest_addr.c_str());

        if(!_caller_profile)
        {
            switch_core_session_destroy(&_session);
            session(NULL);
            return SWITCH_STATUS_FALSE;
        }

        std::string name = STG(FMT("Khomp/%hu/%hu/%s")
                % target().device
                % target().object
                % _call->_dest_addr);

        K::Logger::Logg(C_MESSAGE, PVT_FMT(target(), "Connect inbound channel %s") % name.c_str());

        switch_channel_set_name(channel, name.c_str());
        switch_channel_set_caller_profile(channel, _caller_profile);

        switch_channel_set_state(channel, CS_INIT);


        if (switch_core_session_thread_launch(_session) != SWITCH_STATUS_SUCCESS)
        {
            //TODO: Destruir aqui?
            switch_core_session_destroy(&_session);
            session(NULL);
            
            K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (Error spawning thread)"));
            return SWITCH_STATUS_FALSE;
        }

    }
    else if(call()->_flags.check(Kflags::IS_OUTGOING))
    {
        _call->_orig_addr = profile->caller_id_number;

        switch_channel_set_name(channel, STG(FMT("Khomp/%hu/%hu/%s")
            % target().device
            % target().object
            % _call->_dest_addr.c_str()).c_str());
        _caller_profile = switch_caller_profile_clone(_session, profile);
        switch_channel_set_caller_profile(channel, _caller_profile);

        switch_channel_set_flag(channel, CF_OUTBOUND);
        switch_channel_set_state(channel, CS_INIT);

    }
    else
    {
        DBG(FUNC, PVT_FMT(target(), "r (Not INCOMING or OUTGOING)"));
        return SWITCH_STATUS_FALSE;
    }

    DBG(FUNC, PVT_FMT(target(), "r"));
    return SWITCH_STATUS_SUCCESS;
}
        
int Board::KhompPvt::makeCall(std::string params)
{
    DBG(FUNC, PVT_FMT(target(), "Dialing to %s from %s")
        % _call->_dest_addr.c_str()
        % _call->_orig_addr.c_str());

    /* Lets make the call! */
    std::string full_params;
    
    if(!_call->_orig_addr.empty())
        full_params += STG(FMT("orig_addr=\"%s\"")
                % _call->_orig_addr);

    if(!_call->_dest_addr.empty())
        full_params += STG(FMT(" dest_addr=\"%s\"")
                % _call->_dest_addr);

    if(!params.empty())
        full_params += STG(FMT(" %s")
                % params);

    K::Logger::Logg(C_MESSAGE, PVT_FMT(target(), "We are calling with params: %s.") % full_params.c_str());

    int ret = commandState(KHOMP_LOG, CM_MAKE_CALL, (full_params != "" ? full_params.c_str() : NULL));

    if(ret != ksSuccess)
    {
        switch_core_session_destroy(&_session);
        session(NULL);
        //return SWITCH_STATUS_FALSE;
        //return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER; TODO
    }

    return ret;
}

bool Board::KhompPvt::indicateBusyUnlocked(int cause, bool sent_signaling)
{
    DBG(FUNC, PVT_FMT(target(), "c"));

    /* already playing! */
    if (call()->_flags.check(Kflags::INDICA_RING) ||
        call()->_flags.check(Kflags::INDICA_BUSY) ||
        call()->_flags.check(Kflags::INDICA_FAST_BUSY))
    {
        DBG(FUNC, PVT_FMT(target(), "r (already playing something)"));
        return false;
    }

    setHangupCause(cause, false);

    call()->_flags.set(Kflags::INDICA_BUSY);
        
    DBG(FUNC, PVT_FMT(target(), "r"));

    return true;
}

bool Board::KhompPvt::cadenceStart(Kflags::FlagType flag) 
{
    DBG(FUNC, PVT_FMT(target(), "c"));

    call()->_flags.set(flag);

    std::string tone("");

    /**/ if (call()->_flags.check(Kflags::PLAY_VM_TONE))  tone = "vm-dialtone";
    else if (call()->_flags.check(Kflags::PLAY_PBX_TONE)) tone = "pbx-dialtone";
    else if (call()->_flags.check(Kflags::PLAY_PUB_TONE)) tone = "co-dialtone";
    else if (call()->_flags.check(Kflags::PLAY_RINGBACK)) tone = "ringback";
    else if (call()->_flags.check(Kflags::PLAY_FASTBUSY)) tone = "fast-busy";


    if (tone != "")
    {    
        Opt::CadencesMapType::iterator i = Opt::_cadences.find(tone);
        std::string cmd_params;

        if (i != Opt::_cadences.end())
        {    
            CadenceType cadence = (*i).second;

            if (cadence.ring == 0 && cadence.ring_s == 0)
            {    
                cmd_params = "cadence_times=\"continuous\" mixer_track=1";
            }    
            else if (cadence.ring_ext == 0 && cadence.ring_ext_s == 0)
            {    
                cmd_params = STG(FMT("cadence_times=\"%d,%d\" mixer_track=1")
                    % cadence.ring % cadence.ring_s);
            }    
            else 
            {    
                cmd_params = STG(FMT("cadence_times=\"%d,%d,%d,%d\" mixer_track=1")
                    % cadence.ring % cadence.ring_s % cadence.ring_ext % cadence.ring_ext_s);
            }    

            command(KHOMP_LOG,CM_START_CADENCE,cmd_params.c_str());
        }
        else
        {
            K::Logger::Logg(C_ERROR, PVT_FMT(_target, "r (cadence '%s' not found)") % tone);
            return false;
        }
    }
    else
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target, "r (unknown cadence requested )"));
        return false;
    }

    DBG(FUNC, PVT_FMT(target(), "r"));
    return true;
}

bool Board::KhompPvt::cadenceStop()
{
    call()->_flags.clear(Kflags::PLAY_VM_TONE);
    call()->_flags.clear(Kflags::PLAY_PBX_TONE);
    call()->_flags.clear(Kflags::PLAY_PUB_TONE);
    call()->_flags.clear(Kflags::PLAY_RINGBACK);
    call()->_flags.clear(Kflags::PLAY_FASTBUSY);

    command(KHOMP_LOG,CM_STOP_CADENCE);
}

void Board::KhompPvt::doHangup()
{
    if(!session())
        return;

    //switch_channel_t *channel = NULL;
    //channel = switch_core_session_get_channel(session());
    //switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s CHANNEL HANGUP\n", switch_channel_get_name(channel));

    DBG(FUNC, PVT_FMT(target(), "DO HANGUP"));

    //switch_clear_flag_locked(this, TFLAG_IO);
    //switch_clear_flag_locked(this, TFLAG_VOICE);

    //switch_core_session_destroy(&_session); //TODO
    /* Make the channel available again */

    //TODO: Sending the hangup cause, but the causes must change, how we do that 
    //switch_channel_t * channel = switch_core_session_get_channel(session());
    //switch_channel_hangup(channel, (switch_call_cause_t)call()->_hangup_cause);

    if(_caller_profile)
    {
        DBG(FUNC, PVT_FMT(target(), "Profile != NULL"));
        //TODO: Destruir
        //_caller_profile = NULL;

    }
    switch_core_session_set_private(_session, NULL);

    session(NULL);

}

bool Board::KhompPvt::cleanup(CleanupType type)
{
    flags = 0;

    _reader_frames.clear();
    _writer_frames.clear();
    
    call()->_flags.clear(Kflags::CONNECTED);
        
    call()->_flags.clear(Kflags::GEN_PBX_RING);

    call()->_flags.clear(Kflags::GEN_CO_RING);
    
    /*

    if (_read_codec.implementation)
    {
        switch_core_codec_destroy(&_read_codec);
    }

    if (_write_codec.implementation)
    {
        switch_core_codec_destroy(&_write_codec);
    }

    */

    switch (type)
    {
    case CLN_HARD:
    case CLN_FAIL:
        stop_stream();
        stop_listen();
        doHangup();
        call()->_flags.clear(Kflags::IS_INCOMING);
        call()->_flags.clear(Kflags::IS_OUTGOING);
        call()->_flags.clear(Kflags::REALLY_CONNECTED);
        call()->_flags.clear(Kflags::HAS_PRE_AUDIO);
        call()->_flags.clear(Kflags::HAS_CALL_FAIL);
        call()->_is_progress_sent = false;

        if (call()->_flags.check(Kflags::PLAY_VM_TONE) || 
            call()->_flags.check(Kflags::PLAY_PBX_TONE) || 
            call()->_flags.check(Kflags::PLAY_PUB_TONE) ||
            call()->_flags.check(Kflags::PLAY_RINGBACK) || 
            call()->_flags.check(Kflags::PLAY_FASTBUSY))
        {    
            /* pára cadências e limpa estado das flags */
            cadenceStop();
        }   

        if (call()->_flags.check(Kflags::INDICA_RING) ||
                call()->_flags.check(Kflags::INDICA_BUSY) ||
                call()->_flags.check(Kflags::INDICA_FAST_BUSY))
        {
            mixer(KHOMP_LOG, 1, kmsGenerator, kmtSilence);

            call()->_flags.clear(Kflags::INDICA_RING);
            call()->_flags.clear(Kflags::INDICA_BUSY);
            call()->_flags.clear(Kflags::INDICA_FAST_BUSY);
        }


        return _call->clear();
    case CLN_SOFT:
        if(call()->_flags.check(Kflags::PLAY_VM_TONE) || 
           call()->_flags.check(Kflags::PLAY_PBX_TONE) || 
           call()->_flags.check(Kflags::PLAY_PUB_TONE) ||
           call()->_flags.check(Kflags::PLAY_RINGBACK))
        {    
            /* pára cadências e limpa estado das flags */
            cadenceStop();
        }   
        //switch_core_session_destroy(&_session); TODO
        //session(NULL); //Sera setado para NULL no cleanup HARD
        break;
    }
        
    return true;

}

bool Board::KhompPvt::isFree(bool just_phy)
{
    //DBG(FUNC, DP(this, "c"));
	try
	{
        /*
	    K3L_CHANNEL_CONFIG & config = Globals::k3lapi.channel_config(_target.device,_target.object);
		K3L_CHANNEL_STATUS   status;

		if (k3lGetDeviceStatus (_target.device, _target.object + ksoChannel, &status, sizeof (status)) != ksSuccess)
			return false;

		bool physically_free = false;

		switch (config.Signaling)
		{
			CASE_E1_GRP:
				physically_free = (status.AddInfo == kecsFree);
				break;

#if K3L_AT_LEAST(1,6,0)
			case ksigGSM:
				physically_free = (status.AddInfo == kgsmIdle);
				break;
#endif
			case ksigAnalog:
				physically_free = (status.AddInfo == kfcsEnabled);
				break;

#if K3L_AT_LEAST(1,6,0)
			case ksigAnalogTerminal:
				physically_free = (status.AddInfo == kfxsOnHook);
				break;
#endif
			case ksigSIP:
                physically_free = true;
				break;

			case ksigInactive:
				physically_free = false;
				break;
		}

		// not free? return ASAP!
		if (status.CallStatus != kcsFree || !physically_free)
		{
//			DBG(FUNC, DP(this, "call status not free, or not physically free!"));
			return false;
		}
        */
        ScopedPvtLock lock(this);

        bool is_physical_free = isPhysicalFree();

		/* if we got here, is physically free */
		if (!is_physical_free || just_phy)
			return is_physical_free;


        if(session())
            return false;

        /*
		if (!is_gsm())
		{
			if (calls.at(0).owner != NULL)
			{
				DBG(FUNC, DP(this, "we have owner, not free!"));
				return false;
			}
		}
		else
		{
			for (int i = 0; i < 6; i++)
			{

				call_data_type & data = calls.at(i);

				if (data.owner != NULL)
					return false;
			}
		}
        */

		bool free_state = !(_call->_flags.check(Kflags::IS_INCOMING) || _call->_flags.check(Kflags::IS_OUTGOING));

		DBG(FUNC, PVT_FMT(target(), "[free = %s]") % (free_state ? "yes" : "no"));
		return free_state;
	}
    catch (ScopedLockFailed & err)
	{
		DBG(FUNC, PVT_FMT(target(), "unable to obtain lock: %s") % err._msg.c_str());
	}

	return false;
}


Board::KhompPvt * Board::queueFindFree(PriorityCallQueue &pqueue)
{
    for (PriorityCallQueue::iterator i = pqueue.begin(); i != pqueue.end(); i++)
    {
        KhompPvt *pvt = (*i);
        if (pvt && pvt->isFree())
        {
            return pvt;
        }
    }

    DBG(FUNC, D("found no free channel for fair allocation!"));
    return NULL;
}

void Board::queueAddChannel(PriorityCallQueue &pqueue, unsigned int board, unsigned int object)
{
    try
    {
        KhompPvt * pvt = get(board, object);
        pqueue.insert(pvt);
    }
    catch(K3LAPI::invalid_channel & err)
    {
        //...
    }
}

Board::KhompPvt * Board::findFree(unsigned int board, unsigned int object, bool fully_available)
{
    try
    {
        KhompPvt * pvt = get(board, object);
        return ((fully_available ? pvt->isFree() : pvt->isOK()) ? pvt : NULL);
    }
    catch(K3LAPI::invalid_channel & err)
    {
    }
    return NULL;
}


/* Helper functions - based on code from chan_khomp */
bool Board::KhompPvt::start_stream(void)
{
    if (call()->_flags.check(Kflags::STREAM_UP))
        return true;

    try
    {
        Globals::k3lapi.mixer(_target, 0, kmsPlay, _target.object);
        Globals::k3lapi.command(_target, CM_START_STREAM_BUFFER);
    }
    catch(...)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "ERROR sending START_STREAM_BUFFER command!"));
        return false;
    }

    call()->_flags.set(Kflags::STREAM_UP);

    return true;
}

bool Board::KhompPvt::stop_stream(void)
{
    if (!call()->_flags.check(Kflags::STREAM_UP))
        return true;

    try
    {
        Globals::k3lapi.mixer(_target, 0, kmsGenerator, kmtSilence);
        Globals::k3lapi.command(_target, CM_STOP_STREAM_BUFFER);
    }
    catch(...)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "ERROR sending STOP_STREAM_BUFFER command!"));
        return false;
    }

    call()->_flags.clear(Kflags::STREAM_UP);

    return true;
}

bool Board::KhompPvt::start_listen(bool conn_rx)
{
    if (call()->_flags.check(Kflags::LISTEN_UP))
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
            K::Logger::Logg(C_ERROR, PVT_FMT(target(), "ERROR sending mixer record command!"));
        }
    }

    if (!command(KHOMP_LOG, CM_LISTEN, (const char *) &buffer_size))
        return false;

    call()->_flags.set(Kflags::LISTEN_UP);

    return true;
}

bool Board::KhompPvt::stop_listen(void)
{
    if(!call()->_flags.check(Kflags::LISTEN_UP))
        return true;

    if(!command(KHOMP_LOG, CM_STOP_LISTEN))
    {
        return false;
    }

    call()->_flags.clear(Kflags::LISTEN_UP);

    return true;
}

bool Board::KhompPvt::obtainRX(bool with_delay)
{
    //TODO: Implementar direitinho
    try
    {
        Globals::k3lapi.mixerRecord(_target, 0, kmsNoDelayChannel, target().object);
        Globals::k3lapi.mixerRecord(_target, 1, kmsGenerator, kmtSilence);
    }
    catch(K3LAPI::failed_command & e)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "ERROR sending mixer record command!"));
    }

}

bool Board::KhompPvt::send_dtmf(char digit)
{
    return command(KHOMP_LOG, CM_SEND_DTMF, &digit);

}

void Board::KhompPvt::onChannelRelease(K3L_EVENT *e)
{
    DBG(FUNC, PVT_FMT(target(), "c"));

    try
    {
        ScopedPvtLock lock(this);

        if (e->Code == EV_CHANNEL_FAIL)
        {
            _has_fail = true;
            setHangupCause(SWITCH_CAUSE_NETWORK_OUT_OF_ORDER);
            cleanup(CLN_HARD);
        }
        else
        {
            setHangupCause(SWITCH_CAUSE_NORMAL_CLEARING);
            cleanup(CLN_HARD);
        }

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (unable to lock %s!)") % err._msg.c_str() );
        return;
    }

    DBG(FUNC, PVT_FMT(target(), "r"));
}

//TODO: This method must return more information about the channel allocation
bool Board::KhompPvt::onNewCall(K3L_EVENT *e)
{
    DBG(FUNC, PVT_FMT(target(), "c"));

    std::string orig_addr, dest_addr;

    Globals::k3lapi.get_param(e, "orig_addr", orig_addr);
    Globals::k3lapi.get_param(e, "dest_addr", dest_addr);

    try
    {
        ScopedPvtLock lock(this);

        _call->_orig_addr = orig_addr;
        _call->_dest_addr = dest_addr;

        if (justAlloc(true) != SWITCH_STATUS_SUCCESS)
        {
            int fail_code = callFailFromCause(SWITCH_CAUSE_UNALLOCATED_NUMBER);
            setHangupCause(SWITCH_CAUSE_UNALLOCATED_NUMBER);
            cleanup(CLN_FAIL);
            reportFailToReceive(fail_code);

            K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (Initilization Error on alloc!)"));
            return false;
        }

        if (justStart() != SWITCH_STATUS_SUCCESS)
        {
            int fail_code = callFailFromCause(SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL);
            setHangupCause(SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL);
            cleanup(CLN_FAIL);
            reportFailToReceive(fail_code);
            
            K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (Initilization Error on start!)"));
            return false;
        }
    }
    catch (ScopedLockFailed & err)
    {
        cleanup(CLN_FAIL);
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "r (unable to lock %s!)") % err._msg.c_str() );
        return false;
    }

    DBG(FUNC, PVT_FMT(target(), "r"));
    return true;
}



bool Board::KhompPvt::onDisconnect(K3L_EVENT *e)
{
    DBG(FUNC, PVT_FMT(_target, "c"));   

    try
    {
        ScopedPvtLock lock(this);

        if (call()->_flags.check(Kflags::IS_OUTGOING) ||
            call()->_flags.check(Kflags::IS_INCOMING))
        {
            DBG(FUNC, PVT_FMT(_target, "queueing disconnecting outgoing channel!"));   
            command(KHOMP_LOG, CM_DISCONNECT);//, SCE_HIDE); //Logs
        }
        else
        {
            /* in the case of a disconnect confirm is needed and a call 
            was not started e.g. just after a ev_seizure_start */
            command(KHOMP_LOG, CM_DISCONNECT);//, SCE_HIDE); //Logs
        }
            
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (unable to lock %s!)") % err._msg.c_str() );
        return ksFail;
    }

    DBG(FUNC,PVT_FMT(_target, "r"));   
    return ksSuccess;
}

bool Board::KhompPvt::onAudioStatus(K3L_EVENT *e)
{
    try
    {
        if(e->AddInfo == kmtFax)
        {
            DBG(FUNC, PVT_FMT(_target, "Fax detected"));   
        }
        if(e->AddInfo != kmtSilence)
        {
            if(call()->_flags.check(Kflags::GEN_PBX_RING))
            {
                DBG(FUNC,PVT_FMT(_target, "PBX ringback being disabled..."));   
                ScopedPvtLock lock(this);

                call()->_flags.clear(Kflags::GEN_PBX_RING);

                if (call()->_flags.check(Kflags::PLAY_PBX_TONE) ||
                    call()->_flags.check(Kflags::PLAY_PUB_TONE) ||
                    call()->_flags.check(Kflags::PLAY_RINGBACK) ||
                    call()->_flags.check(Kflags::PLAY_FASTBUSY))
                {    
                    cadenceStop();
                }    

                if (!call()->_flags.check(Kflags::CONNECTED))
                {
                    //obtainRX(K::opt::suppression_delay);
                    obtainRX();
                }
            }
            
            if (!call()->_is_progress_sent && call()->_flags.check(Kflags::HAS_CALL_FAIL))
            {

                ScopedPvtLock lock(this);

                DBG(FUNC,PVT_FMT(_target, "Audio status progress"));   

                call()->_is_progress_sent = true;
                //Sinaliza para o Freeswitch PROGRESS
                switch_channel_t * channel = switch_core_session_get_channel(session());

                if(channel)
                {
                    DBG(FUNC,PVT_FMT(_target, "Pre answer"));   
                    
                    //pvt->signal_state(AST_CONTROL_PROGRESS);
                    switch_channel_pre_answer(channel);

                }
            }

        }
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,PVT_FMT(_target,"unable to lock %s!") % err._msg.c_str() );
        return ksFail;

    }

    return ksSuccess;

}

bool Board::KhompPvt::onCollectCall(K3L_EVENT *e)
{
    try  
    {    
        ScopedPvtLock lock(this);

        //TODO: AMI ? 
        //K::internal::ami_event(pvt, EVENT_FLAG_CALL, "CollectCall",
          //      STG(FMT("Channel: Khomp/B%dC%d\r\n") % pvt->boardid % pvt->objectid));

        if (Opt::_drop_collect_call || _call->_flags.check(Kflags::DROP_COLLECT)
                || _call->_flags.check(Kflags::FILTER_COLLECT))
        {     
            /* disconnect! */
            //TODO: SCE_HIDE !?
            command(KHOMP_LOG,CM_DISCONNECT);
//           command(KHOMP_LOG,CM_DISCONNECT,SCE_HIDE);

        }    
    }    
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,PVT_FMT(_target,"unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    return ksSuccess;
}

void Board::KhompPvt::onSeizureStart(K3L_EVENT *e)
{
    try
    {
        ScopedPvtLock lock(this);
        _call->_user_xfer_digits = Opt::_user_xfer;

        //ami_event !?
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,PVT_FMT(_target,"unable to lock %s!") % err._msg.c_str() );
    }
}

int Board::KhompPvt::setupConnection()
{
    if(!call()->_flags.check(Kflags::IS_INCOMING) && !call()->_flags.check(Kflags::IS_OUTGOING))
    {
        DBG(FUNC,PVT_FMT(_target, "Channel already disconnected"));   
        return false;
    }

    if (call()->_flags.check(Kflags::PLAY_PBX_TONE) || 
        call()->_flags.check(Kflags::PLAY_PUB_TONE) || 
        call()->_flags.check(Kflags::PLAY_RINGBACK) || 
        call()->_flags.check(Kflags::PLAY_FASTBUSY))
    {    
        cadenceStop();
    }   
    
    if(call()->_flags.check(Kflags::GEN_PBX_RING))
        call()->_flags.clear(Kflags::GEN_PBX_RING);

    if(call()->_flags.check(Kflags::GEN_CO_RING))
        call()->_flags.clear(Kflags::GEN_CO_RING);
    

    if (!call()->_flags.check(Kflags::REALLY_CONNECTED))
    {
        //obtainRX(res_out_of_band_dtmf); //TODO: Verificar: com delay?
        /* esvazia buffers de leitura/escrita */ //TODO: Verificar a limpeza
        //pvt->cleanup_buffers();
        //_reader_frames.clear();
        //_writer_frames.clear();

        /* Start listening for audio */
        DBG(FUNC,PVT_FMT(_target, "Starting audio callbacks ..."));   

        /* grab audio, so we can record it! */
        start_listen();

        start_stream();

        DBG(FUNC,PVT_FMT(_target, "Audio callbacks initialized successfully"));   

    }

    if (!call()->_flags.check(Kflags::CONNECTED))
        call()->_flags.set(Kflags::CONNECTED);

    if (!call()->_flags.check(Kflags::REALLY_CONNECTED))
        call()->_flags.set(Kflags::REALLY_CONNECTED);

    //Sinalizar para o Freeswitch o atendimento
    switch_channel_t * channel = switch_core_session_get_channel(session());

    if(channel)
    {

        DBG(FUNC,PVT_FMT(_target, "Call will be answered."));   

        if(call()->_flags.check(Kflags::IS_INCOMING))
        {
            switch_channel_answer(channel);
        }   
        else if(call()->_flags.check(Kflags::IS_OUTGOING))
        {
            switch_channel_mark_answered(channel);
        }

    }

    return ksSuccess;
}


bool Board::KhompPvt::onConnect(K3L_EVENT *e)
{
    //DBG(FUNC,PVT_FMT(_target, "c"));   

    try
    {
        ScopedPvtLock lock(this);

        return setupConnection();
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,PVT_FMT(_target,"unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    return ksSuccess;
}

bool Board::KhompPvt::onCallSuccess(K3L_EVENT *e)
{
    DBG(FUNC,PVT_FMT(_target, "c"));   
 
    try
    {  
        switch_channel_t * channel = switch_core_session_get_channel(session());

        if(channel)
        {
            switch_channel_mark_ring_ready(channel);
            //switch_channel_ring_ready(channel);
        }
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (unable to lock %s!)") % err._msg.c_str() );
        return ksFail;
    }

//    switch_log_printf(SWITCH_CHANNEL_LOG,
//                      SWITCH_LOG_INFO,
//                      PVT_MSG(_target, "r\n"));
    DBG(FUNC,PVT_FMT(_target, "r"));

    return ksSuccess;
}

bool Board::KhompPvt::onCallFail(K3L_EVENT *e)
{
   DBG(FUNC, PVT_FMT(_target, "c"));
   try
   {
       ScopedPvtLock lock(this);

        call()->_flags.set(Kflags::HAS_CALL_FAIL);
    
       //TODO: Notificar o Freeswitch: call fail

        cleanup(CLN_SOFT);
   }
   catch (ScopedLockFailed & err)
   {
       K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (unable to lock %s!)") % err._msg.c_str() );
       return ksFail;
   }
   
   DBG(FUNC, PVT_FMT(_target, "r"));

    return ksSuccess;
}

void Board::KhompPvt::on_ev_no_answer(K3L_EVENT *e)
{
    /* TODO: Destroy sessions and channels */
    DBG(FUNC,PVT_FMT(_target, "No one answered the call."));   
}

void Board::KhompPvt::on_ev_call_answer_info(K3L_EVENT *e)
{
    /* TODO: Set channel variable if we get this event */
    /* TODO: Fire an event so ESL can get it? */
    /* Call Analyser has to be enabled on k3lconfig */
    DBG(FUNC,PVT_FMT(_target, "Detected: \"%s\"") %  Verbose::callStartInfo((KCallStartInfo)e->AddInfo).c_str());   

    /* Fire a custom event about this */
    switch_event_t * event;
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, KHOMP_EVENT_MAINT) == SWITCH_STATUS_SUCCESS)
    {
        Board::khomp_add_event_board_data(_target, event);
        switch_event_add_header_string(event, SWITCH_STACK_BOTTOM,
                "EV_CALL_ANSWER_INFO",
                Verbose::callStartInfo((KCallStartInfo)e->AddInfo).c_str());

        switch_event_fire(&event);
    }
}

void Board::KhompPvt::on_ev_dtmf_detected(K3L_EVENT *e)
{
    DBG(FUNC, PVT_FMT(_target, "c (dtmf=%c).") %  e->AddInfo);

    if(!_session)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (Session is invalid)"));
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
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (Received a DTMF but the channel is invalid)"));
        return;
    }
    
    DBG(FUNC, PVT_FMT(_target, "r"));
}

void Board::KhompPvt::on_ev_internal_fail(K3L_EVENT *e)
{
    K::Logger::Logg(C_ERROR,PVT_FMT(_target,"This is a fatal error and we will not recover. Reason: %s.") 
        % Verbose::internalFail((KInternalFail)e->AddInfo).c_str()
    );
}

void Board::KhompPvt::on_ev_call_hold_start(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_call_hold_stop(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_ss_transfer_fail(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_ring_detected(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_prolarity_reversal(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_collect_call(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_cas_mfc_recv(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_cas_line_stt_changed(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_pulse_detected(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_flash(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_billing_pulse(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_cadence_recognized(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_dtmf_send_finnish(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_end_of_stream(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_user_information(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_isdn_subaddress(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_dialed_digit(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_recv_from_modem(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_new_sms(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_sms_info(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_sms_data(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_sms_send_result(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_reference_fail(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_channel_fail(K3L_EVENT *e){}
void Board::KhompPvt::on_ev_client_reconnect(K3L_EVENT *e){}


void Board::KhompPvt::on_ev_untreated(K3L_EVENT *e)
{
    DBG(FUNC, D("New Event has just arrived with untreated code: %d") % e->Code);   
}


int Board::KhompPvt::eventHandler(K3L_EVENT *e)
{
    DBG(FUNC, D("c"));

    switch(e->Code)
    {
    case EV_CHANNEL_FREE:
    case EV_CHANNEL_FAIL:
        onChannelRelease(e);
        break;
    
    case EV_NEW_CALL:
        onNewCall(e);
        break;

    case EV_CALL_SUCCESS:
        onCallSuccess(e);
        break;

    case EV_CALL_FAIL:
        onCallFail(e);
        break;

    case EV_CONNECT:
        onConnect(e);
        break;

    case EV_DISCONNECT:
        onDisconnect(e);
        break;
    
    case EV_AUDIO_STATUS:
        onAudioStatus(e);
        break;

    case EV_NO_ANSWER:
        on_ev_no_answer(e);
        break;

    case EV_CALL_ANSWER_INFO:
        on_ev_call_answer_info(e);
        break;

    case EV_DTMF_DETECTED:
        on_ev_dtmf_detected(e);
        break;

    case EV_DTMF_SEND_FINISH:
        DBG(FUNC,PVT_FMT(_target,"has sucessfully generated DTMF"));
        break;

    case EV_INTERNAL_FAIL:
        on_ev_internal_fail(e);
        break;

    case EV_COLLECT_CALL:
        onCollectCall(e);
        break;

    case EV_SEIZURE_START:
        onSeizureStart(e);
        break;
    
    default:
        on_ev_untreated(e);
        break;
    }

    DBG(FUNC, D("r"));
    return ksSuccess;
}

int Board::KhompPvt::indicateProgress()
{
    DBG(FUNC, PVT_FMT(_target, "c")); 
    
    int res = ksFail;

    try
    {
        ScopedPvtLock lock(this);

        if (!call()->_flags.check(Kflags::CONNECTED))
        {
            bool has_audio = sendPreAudio(RingbackDefs::RB_SEND_NOTHING);

            if (has_audio)
            {
                /* start grabbing audio */
                start_listen();
                /* start stream if it is not already */
                start_stream();

                res = ksSuccess;
            }
        }
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (unable to lock %s!)") % err._msg.c_str() );
        return ksFail;
    }
    
    DBG(FUNC, PVT_FMT(_target, "r"));

    return res;
}


int Board::KhompPvt::doChannelAnswer(CommandRequest &cmd)
{
    DBG(FUNC, PVT_FMT(_target, "c"));

    try
    {
        ScopedPvtLock lock(this);

        command(KHOMP_LOG, CM_CONNECT);
        call()->_flags.set(Kflags::CONNECTED);

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (unable to lock %s!)") % err._msg.c_str() );
        return ksFail;
    }

    DBG(FUNC, PVT_FMT(_target, "r"));
    return ksSuccess;
}

int Board::KhompPvt::doChannelHangup(CommandRequest &cmd)
{
    DBG(FUNC, PVT_FMT(_target, "c"));

    bool answered     = true;
    bool disconnected = false;

    try
    {
        ScopedPvtLock lock(this);

        if (call()->_flags.check(Kflags::IS_INCOMING))
        {
            DBG(FUNC,PVT_FMT(_target, "disconnecting incoming channel"));   

            //disconnected = command(KHOMP_LOG, CM_DISCONNECT);
        }
        
        if (call()->_flags.check(Kflags::IS_OUTGOING))
        {
            if(call()->_cleanup_upon_hangup)
            {
                DBG(FUNC,PVT_FMT(_target, "disconnecting not allocated outgoing channel..."));   

                disconnected = command(KHOMP_LOG, CM_DISCONNECT);
                cleanup(KhompPvt::CLN_HARD);
                answered = false;

            }
            else
            {
                DBG(FUNC,PVT_FMT(_target, "disconnecting outgoing channel..."));   

                disconnected = command(KHOMP_LOG, CM_DISCONNECT);
            }
        }

        if(answered)
        {
            indicateBusyUnlocked(SWITCH_CAUSE_USER_BUSY, disconnected);
        }

        if (call()->_flags.check(Kflags::IS_INCOMING) && !call()->_flags.check(Kflags::NEEDS_RINGBACK_CMD))
        {
            DBG(FUNC,PVT_FMT(_target, "disconnecting incoming channel..."));
            disconnected = command(KHOMP_LOG, CM_DISCONNECT);
        }

        stop_stream();

        stop_listen();
        
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"r (unable to lock %s!)") % err._msg.c_str() );
        return ksFail;
    }
    

    DBG(FUNC, PVT_FMT(_target, "r"));
    return ksSuccess;
}

int Board::KhompPvt::commandHandler(CommandRequest &cmd)
{
    DBG(FUNC, PVT_FMT(target(), "c"));

    int ret = ksSuccess;

    switch(cmd.type())
    {
    case CommandRequest::COMMAND:
        switch(cmd.code())
        {
        case CommandRequest::CMD_CALL:
            break;
        case CommandRequest::CMD_ANSWER:
            doChannelAnswer(cmd);
            break;
        case CommandRequest::CMD_HANGUP:
            doChannelHangup(cmd);
            break;
        default:
            ret = ksFail;
        }
        break;

    case CommandRequest::ACTION:
        break;
    
    default:
        ret = ksFail;
    }
    
    DBG(FUNC, PVT_FMT(target(), "r"));
    return ret;
}

int Board::eventThread(void *void_evt)
{
    EventRequest evt(false);
    EventFifo * fifo = static_cast < ChanEventHandler * >(void_evt)->fifo();
    int devid = fifo->_device;

    for(;;)
    {
        DBG(FUNC, D("(d=%d) c") % devid);

        while(1)
        {
            try
            {
                evt = fifo->_buffer.consumer_start();
                break;
            }
            catch(...) //BufferEmpty & e
            {
                DBG(FUNC, D("(d=%d) buffer empty") % devid);

                fifo->_cond.wait();

                if (fifo->_shutdown)
                    return 0;

                DBG(FUNC, D("(d=%d) waked up!") % devid);
            }
        }

        /*while (!fifo->_buffer.consume(evt))
        {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "(d=%d) buffer empty\n", fifo->_device);

            fifo->_cond.wait();

            if (fifo->_shutdown)
                return 0;

            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "(d=%d) waked up!\n", fifo->_device);
        }*/

        DBG(FUNC, D("(d=%d) processing buffer...") % devid);

        try
        {
            if(board(devid)->eventHandler(evt.obj(), evt.event()) != ksSuccess)
            {
                DBG(FUNC, D("(d=%d) Error on event(%d)") % devid);
            }
        }
        catch (K3LAPI::invalid_device & invalid)
        {
            K::Logger::Logg(C_ERROR, D("invalid device on event '%s'") 
                % Verbose::eventName(evt.event()->Code).c_str());
        }

        fifo->_buffer.consumer_commit();

    }

    return 0;
}

int Board::commandThread(void *void_evt)
{
    CommandFifo * fifo = static_cast < ChanCommandHandler * >(void_evt)->fifo();
    int devid = fifo->_device;

    for(;;)
    {
        CommandRequest cmd;

        DBG(FUNC, D("(d=%d) Command c") % devid);

        while (!fifo->_buffer.consume(cmd))
        {
            DBG(FUNC, D("(d=%d) Command buffer empty") % devid);
            fifo->_cond.wait();

            if (fifo->_shutdown)
                return 0;

            DBG(FUNC, D("(d=%d) Command waked up!") % devid);
        }

        DBG(FUNC, D("(d=%d) Command processing buffer...") % devid);


        try
        {
            if(get(devid, cmd.obj())->commandHandler(cmd) != ksSuccess)
            {
                DBG(FUNC, D("(d=%d) Error on command(%d)") % devid % cmd.code());
            }
        }
        catch (K3LAPI::invalid_channel & invalid)
        {
            K::Logger::Logg(C_ERROR, OBJ_FMT(devid,cmd.obj(), "invalid device on command '%d'") %  cmd.code());
        }

    }


    return 0;
}

/* This is the callback function for API events. It selects the event *
 * on a switch and forwards to the real implementation.               */
extern "C" int32 Kstdcall khomp_event_callback(int32 obj, K3L_EVENT * e)
{
    DBG(FUNC, D("%s") % Globals::verbose.event(obj, e).c_str());

    switch(e->Code)
    {
    case EV_HARDWARE_FAIL:
    case EV_DISK_IS_FULL:
    case EV_CLIENT_RECONNECT:
    case EV_CLIENT_BUFFERED_AUDIOLISTENER_OVERFLOW:
        DBG(FUNC, D("Audio client buffered overflow"));
        break;
    case EV_CLIENT_AUDIOLISTENER_TIMEOUT:
        K::Logger::Logg(C_ERROR,"Timeout on audio listener, registering audio listener again");
        k3lRegisterAudioListener( NULL, khomp_audio_listener );
        break;
    default:
        EventRequest e_req(obj, e);
        Board::board(e->DeviceId)->chanEventHandler()->write(e_req);
        break;
    }

    return ksSuccess;
}

