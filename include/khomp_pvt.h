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

#ifndef _KHOMP_PVT_H_
#define _KHOMP_PVT_H_

#include "globals.h"
#include "mod_khomp.h"
#include "frame.h"
#include "utils.h"
#include "opt.h"
#include "logger.h"
#include "defs.h"

extern "C" int32 Kstdcall khomp_event_callback (int32, K3L_EVENT *);
extern "C" void Kstdcall khomp_audio_listener (int32, int32, byte *, int32);

struct Board
{

/******************************************************************************/
/*!
 \brief This struct holds a static linked list representing all the Khomp
 channels found in the host. It's also a place holder for session objects
 and some other opaque members used by the module.
 */
struct KhompPvt
{

    typedef SimpleNonBlockLock<25,100>      ChanLockType;

    typedef enum
    {
        CI_MESSAGE_BOX        = 0x01,
        CI_HUMAN_ANSWER       = 0x02,
        CI_ANSWERING_MACHINE  = 0x04,
        CI_CARRIER_MESSAGE    = 0x08,
        CI_UNKNOWN            = 0x10,
        CI_FAX                = 0x20,
    }
    CallInfoType;

    struct Call
    {
        Call() : _is_progress_sent(false) {}
        virtual ~Call() {}

        virtual bool process(std::string name, std::string value = "")
        {
            if (name == "pre_answer")
            {
                _pre_answer = true;
            }
            else if (name == "usr_xfer")
            {
                _user_xfer_digits = value;
            }
            /*
            //TODO: FXS
            else if (name == "ring")
            {
                Strings::vector_type ring_item;
                Strings::tokenize (value, ring_item, ".");

                if (ring_item.size() != 2)
                {
                    //K::logger::logg(C_ERROR, FMT("invalid values on ring string: two numbers, dot separated, are needed."));
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,"invalid values on ring string: two numbers, dot separated, are needed.\n");
                    return false;
                }

                try
                {
                    unsigned long int time_on = Strings::toulong (ring_item[0]);
                    unsigned long int time_off = Strings::toulong (ring_item[1]);

                    ring_on = time_on;
                    ring_off = time_off;

                    //DBG(FUNC, D("ring values adjusted (%i,%i).") % time_on % time_off);
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,"ring values adjusted (%i,%i).\n",time_on,time_off);
                }
                catch (...)
                {
                    //K::logger::logg(C_ERROR, FMT("invalid number on ring string."));
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,"invalid number on ring string.\n");
                }

            }
            else if (name == "ring_ext")
            {
                if (ring_on == -1) // so ring was not set.
                {
                    //K::logger::logg(C_ERROR, FMT("ring_ext only make sense if ring values are set."));
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,"ring_ext only make sense if ring values are set.\n");
                    return false;
                }

                Strings::vector_type ring_item;
                Strings::tokenize (value, ring_item, ".");

                if (ring_item.size() != 2)
                {
                    //K::logger::logg(C_ERROR, FMT("invalid values on ring_ext string: two numbers, dot separated, are needed."));
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,"invalid values on ring_ext string: two numbers, dot separated, are needed.\n");
                    return false;
                }

                try
                {
                    unsigned long int time_on = Strings::toulong (ring_item[0]);
                    unsigned long int time_off = Strings::toulong (ring_item[1]);

                    ring_on_ext = time_on;
                    ring_off_ext = time_off;

                    //DBG(FUNC, D("ring_ext values adjusted (%i,%i).") % time_on % time_off);
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,"ring_ext values adjusted (%i,%i).\n",time_on,time_off);
                }
                catch (...)
                {
                    //K::logger::logg(C_ERROR, FMT("invalid number on ring_ext string."));
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,"invalid number on ring_ext string.\n");
                }

            }
            else if (name == "ring_cadence")
            {
                Opt::CadencesMapType::iterator i = Opt::_cadences.find(value);

                if (i != Opt::_cadences.end())
                {
                    CadenceType cadence = (*i).second;

                    ring_on      = cadence.ring;
                    ring_off     = cadence.ring_s;
                    ring_on_ext  = cadence.ring_ext;
                    ring_off_ext = cadence.ring_ext_s;

                    //DBG(FUNC, D("cadence adjusted (%i,%i,%i,%i).") % cadence.ring
                    //  % cadence.ring_s % cadence.ring_ext % cadence.ring_ext_s);
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,"cadence adjusted (%i,%i,%i,%i).\n",
                                      cadence.ring,cadence.ring_s,cadence.ring_ext,cadence.ring_ext_s);
                }
                else
                {
                    //K::logger::logg(C_ERROR, FMT("unable to find cadence '%s'!") % value);
                    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,"unable to find cadence '%s'!.\n",value.c_str());
                }

            }
            //TODO: FXO
            else if (name == "pre")
            {
                //DBG(FUNC, D("pre digits adjusted (%s).") % value);
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,"pre digits adjusted (%s).\n",value.c_str());
                pre_digits = value;
            }
            */
            else if (name == "orig")
            {
                DBG(FUNC, FMT("orig addr adjusted (%s).") % value.c_str());
                _orig_addr = value;
            }
            else if (name == "dest")
            {
                _dest_addr = value;
            }

            else
            {
                return false;
            }

            return true;
        }

        virtual bool clear()
        {
            _orig_addr = "";
            _dest_addr = "";
            _pre_answer = false;
            //_is_progress_sent = false;
            _user_xfer_digits = "";
            _collect_call = false;
            _hangup_cause = 0;
            _cleanup_upon_hangup = false;
            //_flags = ...;

            return true;
        }

        /* used while answering calls */
        std::string _orig_addr;
        std::string _dest_addr;

        /*
        long int     ring_on;
        long int     ring_off;
        long int     ring_on_ext;
        long int     ring_off_ext;
        */

        /* what should we dial to trigger an user-signaled transfer? */
        std::string _user_xfer_digits;

        /* should freeswitch answer before connect event?  */
        bool _pre_answer;

        bool _is_progress_sent;

        /* is a collect call? */
        bool _collect_call;

        int _hangup_cause;

        bool _cleanup_upon_hangup;

        Kflags _flags;
    };

    struct InitFailure {};

public:

    KhompPvt(K3LAPI::target & target);

    virtual ~KhompPvt()
    {
        _session = NULL;
    }
    
    typedef enum
    {
        CLN_HARD,
        CLN_SOFT,
        CLN_FAIL,
    }
    CleanupType;


    /**************************************************************************/
    /*!
     \defgroup KhompEvents
                Callbacks that boards can implement to produce the expected
                particular behaviour. Refer to Khomp documentation for a
                detailed description of each method.
     */
    /*@{*/
    virtual void on_ev_no_answer(K3L_EVENT *);
    virtual void on_ev_call_hold_start(K3L_EVENT *);
    virtual void on_ev_call_hold_stop(K3L_EVENT *);
    virtual void on_ev_call_answer_info(K3L_EVENT *);
    virtual void on_ev_ss_transfer_fail(K3L_EVENT *);
    virtual void on_ev_ring_detected(K3L_EVENT *);
    virtual void on_ev_prolarity_reversal(K3L_EVENT *);
    virtual void on_ev_collect_call(K3L_EVENT *);
    virtual void on_ev_cas_mfc_recv(K3L_EVENT *);
    virtual void on_ev_cas_line_stt_changed(K3L_EVENT *);
    virtual void on_ev_dtmf_detected(K3L_EVENT *);
    virtual void on_ev_pulse_detected(K3L_EVENT *);
    virtual void on_ev_flash(K3L_EVENT *);
    virtual void on_ev_billing_pulse(K3L_EVENT *);
    virtual void on_ev_cadence_recognized(K3L_EVENT *);
    virtual void on_ev_dtmf_send_finnish(K3L_EVENT *);
    virtual void on_ev_end_of_stream(K3L_EVENT *);
    virtual void on_ev_user_information(K3L_EVENT *);
    virtual void on_ev_isdn_subaddress(K3L_EVENT *);
    virtual void on_ev_dialed_digit(K3L_EVENT *);
    virtual void on_ev_recv_from_modem(K3L_EVENT *);
    virtual void on_ev_new_sms(K3L_EVENT *);
    virtual void on_ev_sms_info(K3L_EVENT *);
    virtual void on_ev_sms_data(K3L_EVENT *);
    virtual void on_ev_sms_send_result(K3L_EVENT *);
    virtual void on_ev_reference_fail(K3L_EVENT *);
    virtual void on_ev_channel_fail(K3L_EVENT *);
    virtual void on_ev_internal_fail(K3L_EVENT *);
    virtual void on_ev_client_reconnect(K3L_EVENT *);
    virtual void on_ev_untreated(K3L_EVENT *);
    /*@}*/

    virtual void onChannelRelease(K3L_EVENT *);
    virtual bool onNewCall(K3L_EVENT *);
    virtual bool onCallSuccess(K3L_EVENT *);
    virtual bool onCallFail(K3L_EVENT *);
    virtual bool onConnect(K3L_EVENT *);
    virtual bool onDisconnect(K3L_EVENT *);
    virtual bool onAudioStatus(K3L_EVENT *);
    virtual bool onCollectCall(K3L_EVENT *);
    virtual void onSeizureStart(K3L_EVENT *);

    virtual int eventHandler(K3L_EVENT *);
    /**************************************************************************/
    virtual int doChannelAnswer(CommandRequest &);
    virtual int doChannelHangup(CommandRequest &);
    
    int commandHandler(CommandRequest &);
    /**************************************************************************/
    virtual int makeCall(std::string params = "");

    virtual int causeFromCallFail(int fail)  { return SWITCH_CAUSE_USER_BUSY; };
    virtual int callFailFromCause(int cause) { return 0; };
    virtual void reportFailToReceive(int fail_code) { call()->_flags.set(Kflags::INDICA_FAST_BUSY); }
    virtual bool cadenceStart(Kflags::FlagType flag);
    virtual bool cadenceStop();

    virtual bool indicateBusyUnlocked(int cause, bool sent_signaling = false);

    virtual bool cleanup(CleanupType type = CLN_HARD);

    virtual RingbackDefs::RingbackStType sendRingBackStatus(int rb_value = RingbackDefs::RB_SEND_DEFAULT) 
    { 
        return RingbackDefs::RBST_UNSUPPORTED; 
    }

    virtual bool sendPreAudio(int rb_value = RingbackDefs::RB_SEND_NOTHING) 
    { 
        if (rb_value != RingbackDefs::RB_SEND_NOTHING)
        {
            if(sendRingBackStatus(rb_value) != RingbackDefs::RBST_SUCCESS)
                return false;
        }

        return true;
    }
    
    virtual bool isOK(void) { return false; }
    virtual bool isPhysicalFree() { return false; }

    virtual bool isFree(bool just_phy = false);

    /**************************************************************************/
    

    int setupConnection();

    int indicateProgress();

    K3LAPI::target & target()
    {
        return _target;
    }

    void session(switch_core_session_t * newSession)
    {
        _session = newSession;
    }

    switch_core_session_t * session()
    {
        return _session;
    }

    bool mixer(const char *file, const char *func, int line, 
            byte track, KMixerSource src, int32 index)
    {
        KMixerCommand mix;

        mix.Track = track;
        mix.Source = src;
        mix.SourceIndex = index;

        return command(file, func, line, CM_MIXER, (const char *)&mix);
    }

    /* Error handling for send command */
    bool command(const char *file, const char *func, int line, int code,
            const char *params = NULL)
    {
        try
        {
            Globals::k3lapi.command(_target, code, params);
        }
        catch(K3LAPI::failed_command & e)
        {
            K::Logger::Logg(C_ERROR,OBJ_FMT(e.dev,e.obj,"Command '%s' has failed with error '%s'")
               % Verbose::commandName(e.code).c_str()
               % Verbose::status((KLibraryStatus)e.rc).c_str());

            return false;
        }

        return true;
    }
    
    //TODO: Unir os dois metodos
    int commandState(const char *file, const char *func, int line, int code,
            const char *params = NULL)
    {
        try
        {
            Globals::k3lapi.command(_target, code, params);
        }
        catch(K3LAPI::failed_command & e)
        {
            K::Logger::Logg(C_ERROR,OBJ_FMT(e.dev,e.obj,"Command '%s' has failed with error '%s'")
               % Verbose::commandName(e.code).c_str()
               % Verbose::status((KLibraryStatus)e.rc).c_str());

            return e.rc;
        }

        return ksSuccess;
    }


    /*!
     \brief Will init part of our private structure and setup all the read/write
     buffers along with the proper codecs. Right now, only PCMA.
    */
    switch_status_t justAlloc(bool is_answering = true, switch_memory_pool_t **pool = NULL);
    switch_status_t justStart(switch_caller_profile_t *profile = NULL);

    void SignalState(int state);

    void doHangup();

    void setHangupCause(int cause, bool set_now = false)
    {
        if(_call->_hangup_cause) 
        { 

            DBG(FUNC,PVT_FMT(_target,"cause already set to %s") % switch_channel_cause2str((switch_call_cause_t) _call->_hangup_cause));
            return;
        }

        if(!session())
        {
            DBG(FUNC,PVT_FMT(_target,"session is null"));
            return;
        }

        switch_channel_t * channel = switch_core_session_get_channel(session());

        if(!channel)
        {
            DBG(FUNC,PVT_FMT(_target,"channnel is null"));
            return;
        }

        int cause_from_freeswitch = switch_channel_get_cause(channel);
        if(cause_from_freeswitch != SWITCH_CAUSE_NONE)
        {
            DBG(FUNC,PVT_FMT(_target,"cause already set to %s from freeswitch") % switch_channel_cause2str((switch_call_cause_t)cause_from_freeswitch));
            _call->_hangup_cause = cause_from_freeswitch;
            return;
        }
        
        if(!cause)
        {
            DBG(FUNC,PVT_FMT(_target,"cause not defined"));
        }
        else
        {
            DBG(FUNC,PVT_FMT(_target,"setting cause to '%s'") % switch_channel_cause2str((switch_call_cause_t) cause));
            _call->_hangup_cause = cause;

            // not set variable in channel owner
            if(set_now)
            {
                switch_channel_hangup(channel, (switch_call_cause_t)_call->_hangup_cause);
                //switch_channel_set_variable(channel,"hangup_cause",switch_channel_cause2str((switch_call_cause_t) cause)); 
            }
        }
    }

    bool start_stream(void);
    bool stop_stream(void);

    bool start_listen(bool conn_rx = true);
    bool stop_listen(void);

    bool obtainRX(bool with_delay = false);

    bool send_dtmf(char digit);

    Call * call() { return _call; }

    K3LAPI::target          _target;    /*!< The device/channel pair to bind this pvt to */
    ChanLockType            _mutex;     /*!< Used for *our* internal locking. */
    Call                  * _call;
    switch_core_session_t * _session;   /*!< The session to which this pvt is associated with */
    bool                    _has_fail;

    switch_caller_profile_t *_caller_profile;

    unsigned int flags;
    switch_mutex_t *flag_mutex;

    switch_codec_t _read_codec;
    switch_codec_t _write_codec;

    FrameSwitchManager _reader_frames;
    FrameBoardsManager _writer_frames;

};

/******************************************************************************/

    typedef std::vector < Board * >    VectorBoard;
    typedef std::vector < KhompPvt * > VectorChannel;  /*!< Collection of pointers of KhompPvts */

     /*
        these (below) are going to rule the elements ordering in our multiset
        (used as a "ordering-save priority queue").
     */
    struct PvtCallCompare
    {
        bool operator() (KhompPvt * pvt1, KhompPvt * pvt2) const
        {
            /* true if pvt1 precedes pvt2 */
            return (Board::getStats(pvt1->target().device,pvt1->target().object,kcsiOutbound) <
                    Board::getStats(pvt2->target().device,pvt2->target().object,kcsiOutbound));
            return true;
        }
    };

    typedef std::multiset< KhompPvt *, PvtCallCompare > PriorityCallQueue;


public:
    Board(int id) : _device_id(id) {}

    virtual ~Board() {}

    int id() { return _device_id; }

    KhompPvt * channel(int obj)
    {
        return _channels.at(obj);
    }

    ChanEventHandler * chanEventHandler() { return _event_handler; }
    
    ChanCommandHandler * chanCommandHandler() { return _command_handler; }

    void initializeChannels(void);
    void finalizeChannels(void);

    virtual int eventHandler(const int obj, K3L_EVENT *e)
    {
        DBG(FUNC, D("(Generic Board) c"));

        int ret = ksSuccess;

        switch(e->Code)
        {
        case EV_REQUEST_DEVICE_SECURITY_KEY:
            break;
        default:
            try
            {
                ret = channel(obj)->eventHandler(e);
            }
            catch (K3LAPI::invalid_channel & invalid)
            {
                K::Logger::Logg(C_ERROR, OBJ_FMT(_device_id,obj,"r (invalid channel on event '%s'.)") 
                % Verbose::eventName(e->Code).c_str());

                return ksFail;
            }

            break;
        }
        
        DBG(FUNC, D("(Generic Board) r"));
        return ret;
    }

protected:
    const int            _device_id;
    ChanEventHandler   * _event_handler; /* The device event handler */
    ChanCommandHandler * _command_handler; /* The device command handler */
    VectorChannel        _channels;


public:
    /* static stuff */
    static bool initializeK3L(void);
    static bool finalizeK3L(void);
    static bool initializeHandlers(void);
    static bool finalizeHandlers(void);
    static void initializeBoards(void);
    static void finalizeBoards(void);
    static void initializeCngBuffer(void);
    static bool initialize(void);
    static bool finalize(void);

    /* Thread Device Event Handler */
    static int eventThread(void *);
    
    /* Thread Device Command Handler */
    static int commandThread(void *);

    /*!
      \brief Lookup channels and boards when dialed.
      \param allocation_string The dialstring as put on Dialplan. [Khomp/[a|A|0-board_high]/[a|A|0-channel_high]/dest].
      \param new_session Session allocated for this call.
      \param[out] cause Cause returned. Returns NULL if suceeded if not, the proper cause.
      \return KhompPvt to be used on the call.
      */
    static KhompPvt * find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause);
    static void khomp_add_event_board_data(const K3LAPI::target target, switch_event_t *event);

    static Board * board(int dev)
    {
        try
        {
            return _boards.at(dev);
        }
        catch(...)
        {
            throw K3LAPI::invalid_device(dev);
        }
    }

    static KhompPvt * get(int32 device, int32 object)
    {
        if (!Globals::k3lapi.valid_channel(device, object))
            throw K3LAPI::invalid_channel(device, object);

        try
        {
            return board(device)->channel(object);
            //return KhompPvt::_pvts[device][object];
        }
        catch(...)
        {
            throw K3LAPI::invalid_channel(device, object);
        }
    }

    static KhompPvt * get(K3LAPI::target & target)
    {
        if (!Globals::k3lapi.valid_channel(target.device, target.object))
            throw K3LAPI::invalid_channel(target.device, target.object);

        try
        {
            return board(target.device)->channel(target.object);
            //return KhompPvt::_pvts[target.device][target.object];
        }
        catch(...)
        {
            throw K3LAPI::invalid_channel(target.device, target.object);
        }
    }

    static unsigned int getStats(int32 device, int32 object, uint32 index)
    {
        unsigned int stats = (unsigned int)-1;

        try
        {
            stats = Globals::k3lapi.channel_stats(device, object, index);
        }
        catch(K3LAPI::invalid_channel & err)
        {
        //K::logger::logg(C_WARNING, B(dev,channel, "Command get_stats has failed with error '%s'.") %
        //                          Verbose::status((KLibraryStatus) stt_res));
        }

        return stats;

    }

    static KhompPvt * queueFindFree(PriorityCallQueue &pqueue);
    static void queueAddChannel(PriorityCallQueue &pqueue, unsigned int board, unsigned int object);
    static KhompPvt * findFree(unsigned int board, unsigned int object, bool fully_available);


public:

    static VectorBoard     _boards;
    static switch_mutex_t *_pvts_mutex;
    static char            _cng_buffer[Globals::cng_buffer_size];

};

#endif /* _KHOMP_PVT_H_*/
