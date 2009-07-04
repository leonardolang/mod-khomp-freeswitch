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

KhompPvt::VectorType  KhompPvt::_pvts;
switch_mutex_t *      KhompPvt::_pvts_mutex;
char                  KhompPvt::_cng_buffer[128];

KhompPvt::KhompPvt(K3LAPI::target & target)
: _target(target), _session(NULL),
  _reader_frames(&_read_codec),
  _writer_frames(&_write_codec)
{

}

KhompPvt::~KhompPvt()
{
    _session = NULL;
};


switch_status_t KhompPvt::init(switch_core_session_t *new_session)
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

switch_status_t KhompPvt::clear()
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


KhompPvt * KhompPvt::find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause)
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

    KhompPvt * pvt = NULL;
    
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
                pvt = KhompPvt::get(board, channel);
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

bool KhompPvt::start_stream(void)
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

bool KhompPvt::stop_stream(void)
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

bool KhompPvt::start_listen(bool conn_rx)
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

bool KhompPvt::stop_listen(void)
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

bool KhompPvt::send_dtmf(char digit)
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


