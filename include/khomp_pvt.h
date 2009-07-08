#ifndef _KHOMP_PVT_H_
#define _KHOMP_PVT_H_

#include "globals.h"
#include "mod_khomp.h"
#include "frame.h"


extern "C" int32 Kstdcall khomp_event_callback (int32, K3L_EVENT *);
extern "C" void Kstdcall khomp_audio_listener (int32, int32, byte *, int32);

/*!
 \brief This struct holds a static linked list representing all the Khomp 
 channels found in the host. It's also a place holder for session objects 
 and some other opaque members used by the module.
 */
struct CBaseKhompPvt
{
    typedef std::vector < CBaseKhompPvt * > InnerVectorType;  /*!< Collection of pointers of KhompPvts */
    typedef std::vector < InnerVectorType > VectorType;  /*!< Collection of InnerVectorType */

    struct InitFailure {};

     CBaseKhompPvt(K3LAPI::target & target);
    ~CBaseKhompPvt();

    /*!
     \defgroup KhompEvents
                Callbacks that boards can implement to produce the expected
                particular behaviour. Refer to Khomp documentation for a
                detailed description of each method.
     */
    /*@{*/
    virtual void on_ev_new_call(K3L_EVENT *);
    virtual void on_ev_seizure_start(K3L_EVENT *);
    virtual void on_ev_connect(K3L_EVENT *);
    virtual void on_ev_disconnect(K3L_EVENT *);
    virtual void on_ev_call_success(K3L_EVENT *);
    virtual void on_ev_channel_free(K3L_EVENT *);
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
    virtual void on_ev_audio_status(K3L_EVENT *);
    virtual void on_ev_cadence_recognized(K3L_EVENT *);
    virtual void on_ev_dtmf_send_finnish(K3L_EVENT *);
    virtual void on_ev_end_of_stream(K3L_EVENT *);
    virtual void on_ev_user_information(K3L_EVENT *);
    virtual void on_ev_isdn_progess_indicator(K3L_EVENT *);
    virtual void on_ev_isdn_subaddress(K3L_EVENT *);
    virtual void on_ev_dialed_digit(K3L_EVENT *);
    virtual void on_ev_recv_from_modem(K3L_EVENT *);
    virtual void on_ev_new_sms(K3L_EVENT *);
    virtual void on_ev_sms_info(K3L_EVENT *);
    virtual void on_ev_sms_data(K3L_EVENT *);
    virtual void on_ev_sms_send_result(K3L_EVENT *);
    virtual void on_ev_call_fail(K3L_EVENT *);
    virtual void on_ev_reference_fail(K3L_EVENT *);
    virtual void on_ev_channel_fail(K3L_EVENT *);
    virtual void on_ev_internal_fail(K3L_EVENT *);
    virtual void on_ev_client_reconnect(K3L_EVENT *);
    virtual void on_ev_link_status(K3L_EVENT *);
    virtual void on_ev_physical_link_down(K3L_EVENT *);
    virtual void on_ev_physical_link_up(K3L_EVENT *);
    virtual void on_ev_untreated(K3L_EVENT *);
    /*@}*/
    
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

    /*!
     \brief Will init part of our private structure and setup all the read/write
     buffers along with the proper codecs. Right now, only PCMA.
    */
    switch_status_t init(switch_core_session_t *new_session);

    
    switch_status_t clear();
    
    bool start_stream(void);
    bool stop_stream(void);

    bool start_listen(bool conn_rx = true);
    bool stop_listen(void);

    bool send_dtmf(char digit);
    

    K3LAPI::target          _target;    /*!< The device/channel pair to bind this pvt to */
    switch_core_session_t * _session;   /*!< The session to which this pvt is associated with */
    
    switch_caller_profile_t *_caller_profile;

    unsigned int flags;
    switch_mutex_t *flag_mutex;

    switch_codec_t _read_codec;
    switch_codec_t _write_codec;

    FrameSwitchManager _reader_frames;
    FrameBoardsManager _writer_frames;


    static CBaseKhompPvt * get(int32 device, int32 object)
    {
        if (!Globals::k3lapi.valid_channel(device, object))
            throw K3LAPI::invalid_channel(device, object);

        return _pvts[device][object];
    }

    static CBaseKhompPvt * get(K3LAPI::target & target)
    {
        return _pvts[target.device][target.object];
    }

    /*!
      \brief Lookup channels and boards when dialed.
      \param allocation_string The dialstring as put on Dialplan. [Khomp/[a|A|0-board_high]/[a|A|0-channel_high]/dest].
      \param new_session Session allocated for this call.
      \param[out] cause Cause returned. Returns NULL if suceeded if not, the proper cause.
      \return KhompPvt to be used on the call.
      */
    static CBaseKhompPvt * find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause);
    
    static bool initialize_k3l()
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
    
    static bool initialize_handlers()
    {
        if (Globals::k3lapi.device_count() == 0)
            return false;

        k3lRegisterEventHandler( khomp_event_callback );
        k3lRegisterAudioListener( NULL, khomp_audio_listener );
        
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "K3l event and audio handlers registered.\n");

        return true;

    }
    
    static void initialize_channels(void)
    {
        
        for (unsigned dev = 0; dev < Globals::k3lapi.device_count(); dev++)
        {
            _pvts.push_back(InnerVectorType());

            for (unsigned obj = 0; obj < Globals::k3lapi.channel_count(dev); obj++)
            {
                K3LAPI::target tgt(Globals::k3lapi, K3LAPI::target::CHANNEL, dev, obj);

                /* TODO; We have to initialize a proper Pvt here */
                CBaseKhompPvt * pvt = new CBaseKhompPvt(tgt);
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

    static void initialize_cng_buffer(void)
    {
        bool turn = true;

        for (unsigned int i = 0; i < Globals::cng_buffer_size; i++)
        {
            _cng_buffer[i] = (turn ? 0xD5 : 0xD4);
            turn = !turn;
        }
    }

    static bool initialize(void)
    {

        if(!initialize_k3l())
            return false;

        switch_mutex_init(&_pvts_mutex, SWITCH_MUTEX_NESTED, Globals::module_pool);

        initialize_cng_buffer();
        initialize_channels();


        return true;
    }

    static void terminate()
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
    
    /* static stuff */
    static switch_mutex_t *_pvts_mutex;
    static VectorType      _pvts; /*!< Static structure that contains all the pvts. Will be initialized by CBaseKhompPvt::initialize */

 public:
    static char            _cng_buffer[Globals::cng_buffer_size];
};

#endif /* _KHOMP_PVT_H_*/
