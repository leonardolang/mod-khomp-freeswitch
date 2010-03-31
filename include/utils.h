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

#ifndef _UTILS_H_
#define _UTILS_H_

#include <bitset>
#include <refcounter.hpp>
#include <ringbuffer.hpp>
#include <simple_lock.hpp>
#include <saved_condition.hpp>
#include <thread.hpp>
#include "globals.h"
#include "logger.h"
#include "defs.h"

/******************************************************************************/
/***************** Abstraction for defining channel flags *********************/

struct Kflags
{
    #define KFLAG_NUMBER 64 /* increment this when change below */

    typedef enum
    {
        CONNECTED = 0x1,
        REALLY_CONNECTED,

        IS_OUTGOING,
        IS_INCOMING,

        STREAM_UP,
        LISTEN_UP,

        GEN_CO_RING,
        GEN_PBX_RING,

        HAS_PRE_AUDIO,
        HAS_CALL_FAIL,

        INVALID_FLAG,

        DROP_COLLECT,
        FILTER_COLLECT,

        NEEDS_RINGBACK_CMD,

        INDICA_RING,
        INDICA_BUSY,
        INDICA_FAST_BUSY,

        PLAY_VM_TONE,
        PLAY_PBX_TONE,
        PLAY_PUB_TONE,
        PLAY_RINGBACK,
        PLAY_FASTBUSY
    }
    FlagType;

    Kflags(): _flags(0) {};

    inline bool  check(FlagType bit)  { return _flags[bit];     }

    inline void set(FlagType bit)     { _flags.set(bit,  true); }
    inline void clear(FlagType bit)   { _flags.set(bit, false); }

 protected:
    std::bitset<KFLAG_NUMBER> _flags;
};

/******************************************************************************/
/************************* Commands and Events Handler ************************/
struct CommandRequest
{
    typedef enum
    {
        NONE = 0,
        COMMAND,
        ACTION
    }
    ReqType;

    typedef enum
    {
        CNONE = 0,

        /* Commands */
        CMD_CALL,
        CMD_ANSWER,
        CMD_HANGUP,
        
        /* Actions */
        FLUSH_REC_STREAM,
        FLUSH_REC_BRIDGE,
        START_RECORD,
        STOP_RECORD

    }
    CodeType;

    typedef enum
    {
        RFA_CLOSE,
        RFA_KEEP_OPEN,
        RFA_REMOVE
    }
    RecFlagType;

    /* "empty" constructor */
    CommandRequest() : 
        _type(NONE),
        _code(CNONE),
        _obj(-1)
    {}

    CommandRequest(ReqType type, CodeType code, int obj) : 
            _type(type),
            _code(code),
            _obj(obj)
    {}

    CommandRequest(const CommandRequest & cmd) : 
            _type(cmd._type), 
            _code(cmd._code), 
            _obj(cmd._obj) 
    {}

    ~CommandRequest() {}

    void operator=(const CommandRequest & cmd)
    {
        _type = cmd._type;
        _code = cmd._code;
        _obj = cmd._obj;
    }

    void mirror(const CommandRequest & cmd_request)
    {
        _type = cmd_request._type;
        _code = cmd_request._code;
        _obj = cmd_request._obj;
    }

    short type() { return _type; }
    
    short code() { return _code; }

    int obj() { return _obj; }

private:
    short _type;
    short _code;
    int   _obj;
};

struct EventRequest
{
    /* "empty" constructor */
    EventRequest(bool can_delete = true) : 
        _delete(can_delete), 
        _obj(-1)
    {
        if(can_delete)
            _event = new K3L_EVENT();
    }

    /* Temporary constructor */
    EventRequest(int obj, K3L_EVENT * ev) :
            _delete(false),
            _obj(obj),
            _event(ev)
    {}
    
    //EventRequest(const EventRequest & ev) : _obj(ev._obj) {}

    ~EventRequest()
    {

        if(!_delete || !_event)
            return;

        if(_event->ParamSize)
            free(_event->Params);

        delete _event;
    }

    void operator=(const EventRequest & ev)
    {
        _delete = false;
        _obj = ev._obj;
        _event = ev._event;

    }

    void mirror(const EventRequest & ev_request)
    {
        //Checar o _event

        if(_event->ParamSize)
        {
            free(_event->Params);
        }

        _event->Params = NULL;

        _obj = ev_request._obj;

        K3L_EVENT * ev     = ev_request._event;

        if(!ev)
        {
            clearEvent();
            return;
        }

        _event->Code       = ev->Code;       // API code
        _event->AddInfo    = ev->AddInfo;    // Parameter 1
        _event->DeviceId   = ev->DeviceId;   // Hardware information
        _event->ObjectInfo = ev->ObjectInfo; // Additional object information
        _event->ParamSize  = ev->ParamSize;  // Size of parameter buffer
        _event->ObjectId   = ev->ObjectId;   // KEventObjectId: Event thrower object id

        if(ev->ParamSize)
        {
            // Pointer to the parameter buffer
            _event->Params = malloc(ev->ParamSize+1);
            memcpy(_event->Params, ev->Params, ev->ParamSize);
            ((char *)(_event->Params))[ev->ParamSize] = 0;
        }
    }

    bool clearEvent()
    {
        _event->Code       = -1;
        _event->AddInfo    = -1;
        _event->DeviceId   = -1;
        _event->ObjectInfo = -1;
        _event->ParamSize  = 0;
        _event->ObjectId   = -1;
    }

    int obj() { return _obj; }

    K3L_EVENT * event() { return _event; }


private:
    bool        _delete;
    int         _obj;
    K3L_EVENT * _event;
};

template < typename R, int S >
struct GenericFifo
{
    typedef R RequestType;
    typedef SimpleNonBlockLock<25,100>  LockType;

    GenericFifo(int device) : 
            _device(device), 
            _shutdown(false), 
            _buffer(S), 
            _mutex(Globals::module_pool), 
            _cond(Globals::module_pool)
    {};

    int                         _device;
    bool                        _shutdown;
    Ringbuffer < RequestType >  _buffer;
    LockType                    _mutex; /* to sync write acess to event list */
    SavedCondition              _cond;
    Thread                     *_thread;

};

typedef GenericFifo < CommandRequest, 250 > CommandFifo;
typedef GenericFifo < EventRequest, 500 >   EventFifo;

/* Used inside KhompPvt to represent an command handler */
struct ChanCommandHandler: NEW_REFCOUNTER(ChanCommandHandler)
{
    typedef int (HandlerType)(void *);

    ChanCommandHandler(int device, HandlerType * handler)
    {
        _fifo = new CommandFifo(device);
        /* device event handler */
        _fifo->_thread = new Thread(handler, (void *)this, Globals::module_pool);
        if(_fifo->_thread->start())
        {
            DBG(FUNC,"Device command handler started");
        }
        else
        {
            DBG(FUNC,"Device command handler error");
        }
    }

    ChanCommandHandler(const ChanCommandHandler & cmd)
    : INC_REFCOUNTER(cmd, ChanCommandHandler),
      _fifo(cmd._fifo)
    {};

    void unreference();

    CommandFifo * fifo()
    {
        return _fifo;
    }

    void signal()
    {
        _fifo->_cond.signal();
    };

    bool writeNoSignal(const CommandRequest &);
    bool write(const CommandRequest &);

protected:
    CommandFifo * _fifo;
};

/* Used inside KhompPvt to represent an event handler */
struct ChanEventHandler: NEW_REFCOUNTER(ChanEventHandler)
{
    typedef int (HandlerType)(void *);

    ChanEventHandler(int device, HandlerType * handler)
    {
        _fifo = new EventFifo(device);
        /* device event handler */
        _fifo->_thread = new Thread(handler, (void *)this, Globals::module_pool);
        if(_fifo->_thread->start())
        {
            DBG(FUNC,"Device event handler started");
        }
        else
        {
            DBG(FUNC,"Device event handler error");
        }
    }

    ChanEventHandler(const ChanEventHandler & evt)
    : INC_REFCOUNTER(evt, ChanEventHandler),
      _fifo(evt._fifo)
    {};

    void unreference();

    EventFifo * fifo()
    {
        return _fifo;
    }

    void signal()
    {
        _fifo->_cond.signal();
    };

    bool provide(const EventRequest &);
    bool writeNoSignal(const EventRequest &);
    bool write(const EventRequest &);

protected:
    EventFifo * _fifo;
};


/******************************************************************************/
/****************************** Internal **************************************/
struct RingbackDefs
{
    enum
    {
        RB_SEND_DEFAULT = -1,
        RB_SEND_NOTHING = -2,
    };

    typedef enum
    {
        RBST_SUCCESS,
        RBST_UNSUPPORTED,
        RBST_FAILURE,
    }
    RingbackStType;
};

/******************************************************************************/
/******************************* Others ***************************************/
static std::string fxs_pad_orig(std::string orig_base, unsigned int padding)
{
    unsigned int orig_size = orig_base.size();
    unsigned int orig_numb = Strings::toulong(orig_base);
    return STG(FMT(STG(FMT("%%0%dd") % orig_size)) % (orig_numb + padding));
}
/******************************************************************************/
/******************************************************************************/


#endif  /* _UTILS_H_ */

