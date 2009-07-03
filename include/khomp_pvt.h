#ifndef _KHOMP_PVT_H_
#define _KHOMP_PVT_H_

#include "globals.h"
#include "mod_khomp.h"
#include "frame.h"

/*!
 \brief This struct holds a static linked list representing all the Khomp 
 channels found in the host. It's also a place holder for session objects 
 and some other opaque members used by the module.
 */
struct KhompPvt
{
    typedef std::vector < KhompPvt * >      InnerVectorType;  /*!< Collection of pointers of KhompPvts */
    typedef std::vector < InnerVectorType > VectorType;  /*!< Collection of InnerVectorType */

    struct InitFailure {};

     KhompPvt(K3LAPI::target & target);
    ~KhompPvt();

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

    void clear()
    {
        flags = 0;

        _reader_frames.clear();
        _writer_frames.clear();

        session(NULL);
    }

    /*!
     \brief Will init part of our private structure and setup all the read/write
     buffers along with the proper codecs. Right now, only PCMA.
    */
    switch_status_t init(switch_core_session_t *new_session);

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


    static KhompPvt * get(int32 device, int32 object)
    {
        if (!Globals::k3lapi.valid_channel(device, object))
            throw K3LAPI::invalid_channel(device, object);

        return _pvts[device][object];
    }

    static KhompPvt * get(K3LAPI::target & target)
    {
        return _pvts[target.device][target.object];
    }

    /*!
      \brief Lookup channels and boards when dialed.
      \param allocation_string The dialstring as put on Dialplan. [Khomp/[a|A|0-board_high]/[a|A|0-channel_high]/dest].
      \param new_session Session allocated for this call.
      \param[out] Cause returned. Returns NULL if suceeded if not, the proper cause.
      \return KhompPvt to be used on the call.
      */
    static KhompPvt * find_channel(char* allocation_string, switch_core_session_t * new_session, switch_call_cause_t * cause);
    
    static void initialize_channels(void)
    {
        
        for (unsigned dev = 0; dev < Globals::k3lapi.device_count(); dev++)
        {
            _pvts.push_back(InnerVectorType());

            for (unsigned obj = 0; obj < Globals::k3lapi.channel_count(dev); obj++)
            {
                K3LAPI::target tgt(Globals::k3lapi, K3LAPI::target::CHANNEL, dev, obj);

                KhompPvt * pvt = new KhompPvt(tgt);
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
    
    static void initialize(void)
    {
        switch_mutex_init(&_pvts_mutex, SWITCH_MUTEX_NESTED, Globals::module_pool);

        initialize_cng_buffer();
        initialize_channels();
    }

    static void terminate()
    {
        switch_mutex_lock(_pvts_mutex);
        
        for (VectorType::iterator it_dev = _pvts.begin(); it_dev != _pvts.end(); it_dev++)
        {
            InnerVectorType & obj_vec = *it_dev;

            for (InnerVectorType::iterator it_obj = obj_vec.begin(); it_obj != obj_vec.end(); it_obj++)
            {
                KhompPvt * pvt = *it_obj;
                delete pvt;
            }
        }
    }
    
    /* static stuff */
    static switch_mutex_t *_pvts_mutex;
    static VectorType      _pvts; /*!< Static structure that contains all the pvts. Will be initialized by KhompPvt::initialize */

 public:
    static char            _cng_buffer[Globals::cng_buffer_size];
};

#endif /* _KHOMP_PVT_H_*/
