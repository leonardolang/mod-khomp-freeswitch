/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2009, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Joao Mesquita <mesquita (at) khomp.com.br>
 * Raul Fragoso <raulfragoso (at) gmail.com>
 *
 *
 * mod_khomp.c -- Khomp board Endpoint Module
 *
 */

/**
 * @file mod_khomp.cpp
 * @brief Khomp Endpoint Module
 * @see mod_khomp
 */


#define KHOMP_SYNTAX "USAGE:\n"\
                     "\tkhomp help\n"\
                     "\tkhomp show [info|links|channels|conf]\n\n"

#include <string>

#include "mod_khomp.h"
#include "spec.h"
#include "lock.h"
#include "logger.h"
#include "opt.h"
#include "utils.h"
#include "globals.h"

/*!
 \brief Callback generated from K3L API for every new event on the board.
 \param[in] obj Object ID (could be a channel or a board, depends on device type) which generated the event.
 \param[in] e The event itself.
 \return ksSuccess if the event was treated
 \see K3L_EVENT Event specification
 */
extern "C" int32 Kstdcall khomp_event_callback(int32 obj, K3L_EVENT * e);

/*!
 \brief Callback generated from K3L API everytime audio is available on the board.
 @param[in] deviceid Board on which we get the event
 @param[in] objectid The channel we are getting the audio from
 @param[out] read_buffer The audio buffer itself (RAW)
 @param[in] read_size The buffer size, meaning the amount of data to be read
 \return ksSuccess if the event was treated
 */
extern "C" void Kstdcall khomp_audio_listener(int32 deviceid, int32 objectid,
                                          byte * read_buffer, int32 read_size);

/*!
  \brief Load the module. Expadend by a FreeSWITCH macro.
  Things we do here:
  \li Initialize a static structure on KhompPvt
  \li Load the configuration
  \li Start the K3L API, responsible for connecting to KServer
  \li Register mod APIs and APPs
  \li Register audio callback for KServer
  \li Register event callback for KServer
  \see Opt Where all the configs are handled
  \see khomp_event_callback To where we bind the event handler
  \see khomp_audio_listener To where we bind the audio handlers
  */
SWITCH_MODULE_LOAD_FUNCTION(mod_khomp_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_khomp_shutdown);
SWITCH_MODULE_DEFINITION(mod_khomp, mod_khomp_load, mod_khomp_shutdown, NULL);

switch_state_handler_table_t khomp_state_handlers = {
    /*.on_init */ channel_on_init,
    /*.on_routing */ channel_on_routing,
    /*.on_execute */ channel_on_execute,
    /*.on_hangup */ channel_on_hangup,
    /*.on_exchange_media */ channel_on_exchange_media,
    /*.on_soft_execute */ channel_on_soft_execute,
    /*.on_consume_media */ NULL,
    /*.on_hibernate */ NULL,
    /*.on_reset */ NULL,
    /*.on_park*/ NULL,
    /*.on_reporting*/ NULL,
    /*.on_destroy*/ channel_on_destroy
};

/* Callbacks for FreeSWITCH */
switch_call_cause_t channel_outgoing_channel(
        switch_core_session_t *session,
        switch_event_t *var_event,
        switch_caller_profile_t *outbound_profile,
        switch_core_session_t **new_session,
        switch_memory_pool_t **pool,
        switch_originate_flag_t flags,
        switch_call_cause_t *cancel_cause);
switch_status_t channel_read_frame(switch_core_session_t *session,
        switch_frame_t **frame,
        switch_io_flag_t flags,
        int stream_id);
switch_status_t channel_write_frame(switch_core_session_t *session,
        switch_frame_t *frame,
        switch_io_flag_t flags,
        int stream_id);
switch_status_t channel_kill_channel(switch_core_session_t *session,
        int sig);
switch_status_t channel_send_dtmf(switch_core_session_t *session,
        const switch_dtmf_t *dtmf);
switch_status_t channel_receive_message(switch_core_session_t *session,
        switch_core_session_message_t *msg);
switch_status_t channel_receive_event(switch_core_session_t *session,
        switch_event_t *event);

/*!
 \ingroup fs_states
 */
switch_io_routines_t khomp_io_routines = {
    /*.outgoing_channel */ channel_outgoing_channel,
    /*.read_frame */ channel_read_frame,
    /*.write_frame */ channel_write_frame,
    /*.kill_channel */ channel_kill_channel,
    /*.send_dtmf */ channel_send_dtmf,
    /*.receive_message */ channel_receive_message,
    /*.receive_event */ channel_receive_event
};

/* Macros to define specific API functions */
SWITCH_STANDARD_API(apiKhomp);

/*!
 \brief Print a system summary for all the boards. [khomp show info]
 */
void printSystemSummary(switch_stream_handle_t* stream);
/*!
 \brief Print link status. [khomp show links]
 */
void apiPrintLinks(switch_stream_handle_t* stream, unsigned int device,
        unsigned int link);
/*!
 \brief Print board channel status. [khomp show channels]
 */
void apiPrintChannels(switch_stream_handle_t* stream);

/*!
   \brief State methods they get called when the state changes to the specific state
   returning SWITCH_STATUS_SUCCESS tells the core to execute the standard state method next
   so if you fully implement the state you can return SWITCH_STATUS_FALSE to skip it.
*/
switch_status_t channel_on_init(switch_core_session_t *session)
{
    DBG(FUNC,"CHANNEL INIT")

    /*
    Board::KhompPvt * tech_pvt = static_cast< Board::KhompPvt* >(switch_core_session_get_private(session));
    if(!tech_pvt)
    {
        DBG(FUNC,"Init: pvt is NULL")
        return SWITCH_STATUS_FALSE;
    }
    */

    switch_channel_t *channel = switch_core_session_get_channel(session);
    if(!channel)
    {
        DBG(FUNC,"Init: channel is NULL")
        return SWITCH_STATUS_FALSE;
    }

    //switch_set_flag_locked(tech_pvt, TFLAG_IO);

    /* Move channel's state machine to ROUTING. This means the call is trying
       to get from the initial start where the call because, to the point
       where a destination has been identified. If the channel is simply
       left in the initial state, nothing will happen. */
    switch_channel_set_state(channel, CS_ROUTING);

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_routing(switch_core_session_t *session)
{
    DBG(FUNC,"CHANNEL ROUTING");
/*
    switch_channel_t *channel = NULL;
    Board::KhompPvt *tech_pvt = NULL;

    channel = switch_core_session_get_channel(session);
    if(!channel)
    {
        K::Logger::Logg(C_ERROR,"Routing: channel is NULL");
        return SWITCH_STATUS_FALSE;
    }

    tech_pvt = static_cast<Board::KhompPvt *>(switch_core_session_get_private(session));
    if(!tech_pvt)
    {
        K::Logger::Logg(C_ERROR,"Routing: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

    DBG(FUNC,FMT("%s CHANNEL ROUTING") % switch_channel_get_name(channel))
*/

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_execute(switch_core_session_t *session)
{

    DBG(FUNC,"CHANNEL EXECUTE");
/*
    switch_channel_t *channel = NULL;
    Board::KhompPvt *tech_pvt = NULL;

    channel = switch_core_session_get_channel(session);
    if(!channel)
    {
        K::Logger::Logg(C_ERROR,"Execute: channel is NULL");
        return SWITCH_STATUS_FALSE;
    }

    tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));
    if(!tech_pvt)
    {
        K::Logger::Logg(C_ERROR,"Execute: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

    DBG(FUNC,FMT("%s CHANNEL EXECUTE") % switch_channel_get_name(channel))

*/

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_hangup(switch_core_session_t *session)
{
    DBG(FUNC,"Hangup c")
    Board::KhompPvt *tech_pvt = NULL;

    if(!session)
    {
        K::Logger::Logg(C_ERROR,"Session is NULL in HANGUP");
        return SWITCH_STATUS_SUCCESS;
    }
    
    switch_channel_t *channel = switch_core_session_get_channel(session);
    if(!channel)
    {
        K::Logger::Logg(C_ERROR,"Kill: channel is NULL");
        return SWITCH_STATUS_FALSE;
    }

    tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));
    if(!tech_pvt)
    {
        DBG(FUNC, "Hangup: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

    try
    {
        //ScopedPvtLock lock(tech_pvt);

        //switch_channel_hangup(channel, SWITCH_CAUSE_NORMAL_CLEARING);
        //switch_clear_flag_locked(tech_pvt, TFLAG_IO);
        //switch_clear_flag_locked(tech_pvt, TFLAG_VOICE);
        //switch_thread_cond_signal(tech_pvt->_cond);

        //lock.unlock();

        CommandRequest c_req(CommandRequest::COMMAND, CommandRequest::CMD_HANGUP, tech_pvt->target().object);

        Board::board(tech_pvt->target().device)->chanCommandHandler()->write(c_req);

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,FMT("unable to lock: %s!") %  err._msg.c_str());
    }

    DBG(FUNC,"Hangup r")

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_destroy(switch_core_session_t *session)
{
    /* Doesn't do anything for now */

    DBG(FUNC,"CHANNEL DESTROY")

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_kill_channel(switch_core_session_t *session, int sig)
{
    DBG(FUNC,FMT("CHANNEL KILL, kill = %d") % sig) 
    Board::KhompPvt *tech_pvt = NULL;

    /*
    switch_channel_t *channel = NULL;
    channel = switch_core_session_get_channel(session);
    if(!channel)
    {
        DBG(FUNC,"Kill: channel is NULL")
        return SWITCH_STATUS_FALSE;
    }
    */

    tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));

    if(!tech_pvt)
    {
        //K::Logger::Logg(C_ERROR,"Kill: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

    switch (sig) {
    case SWITCH_SIG_KILL:
        DBG(FUNC,"CHANNEL KILL, SIGKILL!")
        //switch_clear_flag_locked(tech_pvt, TFLAG_IO);
        //switch_clear_flag_locked(tech_pvt, TFLAG_VOICE);
        //switch_channel_hangup(channel, SWITCH_CAUSE_NORMAL_CLEARING);
        //switch_thread_cond_signal(tech_pvt->_cond);
        break;
    case SWITCH_SIG_BREAK:
        DBG(FUNC,"CHANNEL KILL, BREAK!")
        switch_set_flag_locked(tech_pvt, TFLAG_BREAK);
        break;
    default:
        DBG(FUNC,"CHANNEL KILL, WHAT?!")
        break;
    }

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_exchange_media(switch_core_session_t *session)
{
    DBG(FUNC,"CHANNEL LOOPBACK")
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_on_soft_execute(switch_core_session_t *session)
{
    DBG(FUNC,"CHANNEL TRANSMIT")
    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_send_dtmf(switch_core_session_t *session, const switch_dtmf_t *dtmf)
{
    DBG(FUNC,"CHANNEL SEND-DTMF")

    Board::KhompPvt *tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));
    if(!tech_pvt)
    {
        K::Logger::Logg(C_ERROR,"Send DTMF: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

    if(!tech_pvt->send_dtmf(dtmf->digit))
        return SWITCH_STATUS_FALSE;

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t channel_read_frame(switch_core_session_t *session, switch_frame_t **frame, switch_io_flag_t flags, int stream_id)
{
    Board::KhompPvt *tech_pvt = NULL;
    //switch_time_t started = switch_time_now();
    //unsigned int elapsed;
    switch_byte_t *data;

    tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));
    if(!tech_pvt)
    {
        DBG(FUNC,"Read Frame: pvt is NULL")
        return SWITCH_STATUS_FALSE;
    }
//    tech_pvt->_read_frame.flags = SFF_NONE;
    *frame = NULL;

//    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING,
//        "We are here to read things! (%u,%02u).\n",
//        tech_pvt->target().device, tech_pvt->target().object);

    //while (switch_test_flag(tech_pvt, TFLAG_IO))
    while (true)
    {
        if (switch_test_flag(tech_pvt, TFLAG_BREAK))
        {
            switch_clear_flag_locked(tech_pvt, TFLAG_BREAK);

            *frame = tech_pvt->_reader_frames.cng();
            return SWITCH_STATUS_SUCCESS;
        }

        if (tech_pvt->call()->_flags.check(Kflags::LISTEN_UP))
        {
            *frame = tech_pvt->_reader_frames.pick();

            if (!*frame)
            {
//                  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
//                    "Reader buffer empty, waiting... (%u,%02u).\n",
//                    tech_pvt->target().device, tech_pvt->target().object);

                switch_cond_next();
                continue;
            }
//            else
//            {
//                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING,
//                 "We are returning a new frame! (%u,%02u).\n",
//                    tech_pvt->target().device, tech_pvt->target().object);
//            }
        }
        else
        {
            *frame = tech_pvt->_reader_frames.cng();
        }

#ifdef BIGENDIAN
        if (switch_test_flag(tech_pvt, TFLAG_LINEAR))
        {
            switch_swap_linear((*frame)->data, (int) (*frame)->datalen / 2);
        }
#endif

        return SWITCH_STATUS_SUCCESS;
    }

    return SWITCH_STATUS_FALSE;

//  cng:
//    data = (switch_byte_t *) tech_pvt->_read_frame.data;
//    data[0] = 65;
//    data[1] = 0;
//    tech_pvt->_read_frame.datalen = 2;
//    tech_pvt->_read_frame.flags = SFF_CNG;
//    *frame = &tech_pvt->_read_frame;
//    return SWITCH_STATUS_SUCCESS;

}

switch_status_t channel_write_frame(switch_core_session_t *session, switch_frame_t *frame, switch_io_flag_t flags, int stream_id)
{
    Board::KhompPvt *tech_pvt = NULL;
    //switch_frame_t *pframe;

    tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));
    if(!tech_pvt)
    {
        DBG(FUNC, "Write Frame: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

/*
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING,
        "We are here to write things! (%u,%02u).\n",
        tech_pvt->target().device, tech_pvt->target().object);
*/
    /*
    if (!switch_test_flag(tech_pvt, TFLAG_IO))
    {
        return SWITCH_STATUS_FALSE;
    }
    */
#ifdef BIGENDIAN
    if (switch_test_flag(tech_pvt, TFLAG_LINEAR))
    {
        switch_swap_linear(frame->data, (int) frame->datalen / 2);
    }
#endif

    if (frame) // && frame->flags != SFF_CNG)
    {
        if (!tech_pvt->_writer_frames.give((const char *)frame->data, (size_t)frame->datalen))
        {
            /*
               switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
                "Writer buffer full! (%u,%02u) (len=%u).\n",
                tech_pvt->target().device, tech_pvt->target().object, frame->datalen);
             */
        }
    }

    return SWITCH_STATUS_SUCCESS;

}

switch_status_t channel_answer_channel(switch_core_session_t *session)
{
    Board::KhompPvt *tech_pvt;
    switch_channel_t *channel = NULL;

    channel = switch_core_session_get_channel(session);

    if(!channel)
    {
        K::Logger::Logg(C_ERROR,"Answer: channel is NULL");
        return SWITCH_STATUS_FALSE;
    }

    tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));

    if(!tech_pvt)
    {
        K::Logger::Logg(C_ERROR,"Answer: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

    DBG(FUNC,"CHANNEL ANSWER")

    /* Start listening for audio */
    DBG(FUNC,"Starting audio callbacks ...")

    tech_pvt->start_stream();
    tech_pvt->start_listen();

    DBG(FUNC,"Audio callbacks initialized successfully")

    switch_channel_mark_answered(channel);

    return SWITCH_STATUS_SUCCESS;
}

switch_status_t khomp_answer(switch_core_session_t *session)
{
    Board::KhompPvt *tech_pvt;
    switch_channel_t *channel = NULL;

    channel = switch_core_session_get_channel(session);

    if(!channel)
    {
        K::Logger::Logg(C_ERROR,"Answer: channel is NULL");
        return SWITCH_STATUS_FALSE;
    }

    tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));
    
    if(!tech_pvt)
    {
        K::Logger::Logg(C_ERROR,"Answer: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }

    DBG(FUNC,"CHANNEL ANSWER")

    try
    {
        ScopedPvtLock lock(tech_pvt);

        if(!tech_pvt->session() || !tech_pvt->call()->_flags.check(Kflags::IS_INCOMING))
        {
            K::Logger::Logg(C_ERROR,"Channel is not connected");
            return SWITCH_STATUS_FALSE;
        }

        //set_collectcall(pvt,c);

        if (!tech_pvt->call()->_flags.check(Kflags::CONNECTED))
        {
            /* we can unlock it now */
            lock.unlock();

            CommandRequest c_req(CommandRequest::COMMAND, CommandRequest::CMD_ANSWER, tech_pvt->target().object);

            Board::board(tech_pvt->target().device)->chanCommandHandler()->write(c_req);

        }

        //TODO: Esperar o atendimento EV_CONNECT
        //if (!khomp_pvt::loop_while_flag_timed(pvt, c, kflags::REALLY_CONNECTED, timeout, &lock, false)) { return SWITCH_STATUS_FALSE; }


    }
    catch (K3LAPI::invalid_device & err)
    {
        K::Logger::Logg(C_ERROR,FMT("unable to get device: %d!") % err.device);
        return SWITCH_STATUS_FALSE;
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,FMT("unable to lock: %s!") % err._msg.c_str());
        return SWITCH_STATUS_FALSE;

    }

    //switch_channel_mark_answered(channel);
    
    return SWITCH_STATUS_SUCCESS;
}


switch_status_t channel_receive_message(switch_core_session_t *session, switch_core_session_message_t *msg)
{
    Board::KhompPvt *tech_pvt;
    
    if(!session)
    {
        K::Logger::Logg(C_ERROR,"session is NULL");
        return SWITCH_STATUS_FALSE;
    }

    if(!msg)
    {
        K::Logger::Logg(C_ERROR,"msg is NULL");
        return SWITCH_STATUS_FALSE;
    }

    DBG(FUNC,FMT("Received message %d from [%s].") % msg->message_id % (msg->from ? msg->from : "NULL")); 

    tech_pvt = (Board::KhompPvt *) switch_core_session_get_private(session);
    if(!tech_pvt)
    {
        K::Logger::Logg(C_ERROR,"Receive Message: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }
    
    DBG(FUNC, PVT_FMT(tech_pvt->_target, "Received message %d from [%s].") % msg->message_id % (msg->from ? msg->from : "NULL")); 

    switch (msg->message_id) {
    case SWITCH_MESSAGE_INDICATE_ANSWER:
        {
            //channel_answer_channel(session);
            return khomp_answer(session);
         }
        break;
    case SWITCH_MESSAGE_INDICATE_PROGRESS:
        tech_pvt->indicateProgress();
        break;
    default:
        break;
    }

    return SWITCH_STATUS_SUCCESS;
}

/*!
  \brief Make sure when you have 2 sessions in the same scope that you pass
  the appropriate one to the routines that allocate memory or you will have
  1 channel with memory allocated from another channel's pool!
*/
switch_call_cause_t channel_outgoing_channel
        (switch_core_session_t *session,
         switch_event_t *var_event,
         switch_caller_profile_t *outbound_profile,
         switch_core_session_t **new_session,
         switch_memory_pool_t **pool,
         switch_originate_flag_t flags,
         switch_call_cause_t *cancel_cause)
{

    if (!outbound_profile)
    {
        K::Logger::Logg(C_ERROR,FMT("No caller profile"));
        //switch_core_session_destroy(new_session);
        return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
    }

    Board::KhompPvt *tech_pvt;
    int cause = (int)SWITCH_CAUSE_SUCCESS;

    // got the pvt
    tech_pvt = process_dial_string(outbound_profile->destination_number,&cause);

    if(tech_pvt == NULL || cause != SWITCH_CAUSE_SUCCESS)
    {
        K::Logger::Logg(C_ERROR,"unable to find free channel");
        return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
    }

    try
    {
        ScopedPvtLock lock(tech_pvt);

        if(tech_pvt->justAlloc(false, pool) != SWITCH_STATUS_SUCCESS)
        {
            K::Logger::Logg(C_ERROR,"Initilization Error!");
            return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
        }

        // get the session to FS
        *new_session = tech_pvt->session();

        if(tech_pvt->justStart(outbound_profile) != SWITCH_STATUS_SUCCESS)
        {
            *new_session = NULL;
            K::Logger::Logg(C_ERROR,"unable to justStart");
            return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
        }
        
        if(tech_pvt->makeCall() != ksSuccess)
        {
            *new_session = NULL;
            K::Logger::Logg(C_ERROR,"unable to makeCall");
            return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
        }

    }
    catch(ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,FMT("unable to lock: %s!") % err._msg.c_str());
        return SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER;
    }

    return SWITCH_CAUSE_SUCCESS;
}

switch_status_t channel_receive_event(switch_core_session_t *session, switch_event_t *event)
{
    DBG(FUNC,"Receive Event")
    struct Board::KhompPvt *tech_pvt = static_cast<Board::KhompPvt*>(switch_core_session_get_private(session));
    char *body = switch_event_get_body(event);
    //switch_assert(tech_pvt != NULL);

    if(!tech_pvt)
    {
        K::Logger::Logg(C_ERROR,"Receive Event: pvt is NULL");
        return SWITCH_STATUS_FALSE;
    }
    
    DBG(FUNC,FMT("Receive Event id[%d] name[%s] body=[%s]") % event->event_id % event->headers->name % body); 

    if (!body) {
        body = (char *)"";
    }

    return SWITCH_STATUS_SUCCESS;
}

bool start_log()
{
    /* we love shortcuts! */
    typedef K::LogManager::Option LogOpt; 

    typedef LogOpt::Flags     Flags;
    typedef LogOpt::InitFlags FL;  

    /* configures default log levels */
    K::Logger::Logg.classe(C_ERROR)
        & LogOpt(O_CONSOLE, "ERROR: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::ENABLED)))
        & LogOpt(O_GENERIC, "E: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_WARNING)
        & LogOpt(O_CONSOLE, "WARNING: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::ENABLED)))
        & LogOpt(O_GENERIC, "W: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_MESSAGE)
        & LogOpt(O_CONSOLE, Flags(FL(LogOpt::ENABLED)))
        & LogOpt(O_GENERIC, "M: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    //k3l messages 
    K::Logger::Logg.classe(C_COMMAND)
        & LogOpt(O_CONSOLE, Flags(FL(LogOpt::DATETIME)))
        & LogOpt(O_GENERIC, "c: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID)));

    K::Logger::Logg.classe(C_EVENT)
        & LogOpt(O_CONSOLE, Flags(FL(LogOpt::DATETIME)))
        & LogOpt(O_GENERIC, "e: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID)));

    K::Logger::Logg.classe(C_AUDIO_EV)
        & LogOpt(O_CONSOLE, Flags(FL(LogOpt::DATETIME)))
        & LogOpt(O_GENERIC, "a: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID)));

    K::Logger::Logg.classe(C_MODEM_EV)
        & LogOpt(O_CONSOLE, Flags(FL(LogOpt::DATETIME)))
        & LogOpt(O_GENERIC, "m: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_LINK_STT)
        & LogOpt(O_CONSOLE, Flags(FL(LogOpt::DATETIME) & FL(LogOpt::ENABLED)))
        & LogOpt(O_GENERIC, "s: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_CAS_MSGS)
        & LogOpt(O_CONSOLE, Flags(FL(LogOpt::DATETIME) & FL(LogOpt::ENABLED)))
        & LogOpt(O_GENERIC, "p: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    // channel debug 
    K::Logger::Logg.classe(C_DBG_FUNC)
        & LogOpt(O_GENERIC, "F: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_DBG_LOCK)
        & LogOpt(O_GENERIC, "L: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_DBG_THRD)
        & LogOpt(O_GENERIC, "T: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_DBG_STRM)
        & LogOpt(O_GENERIC, "S: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_DBG_CONF)
        & LogOpt(O_GENERIC, "C: ", Flags(FL(LogOpt::DATETIME) & FL(LogOpt::THREADID) & FL(LogOpt::ENABLED)));

    K::Logger::Logg.classe(C_DBG_FUNC).enabled(true);
    K::Logger::Logg.classe(C_DBG_LOCK).enabled(true);
    K::Logger::Logg.classe(C_DBG_THRD).enabled(true);
    K::Logger::Logg.classe(C_DBG_STRM).enabled(true);

     /* adds a prefix to the cli messages */
    K::Logger::Logg.classe(C_CLI).prefix("<K> ");

    /* inserts default console log before opening files */
    K::Logger::Logg.add(O_CONSOLE,K::SwitchConsoleLog(), "mod_khomp: ");

    time_t      tv;
    struct tm   lt;

    /* get local time! */
    time (&tv);
    localtime_r (&tv, &lt);

    Globals::base_path = STG(FMT("/var/log/khomp%d.%d/mod_khomp-%04d%02d%02d_%02d%02d%02d/")
        % k3lApiMajorVersion % k3lApiMinorVersion % (lt.tm_year + 1900) % (lt.tm_mon + 1)
        % lt.tm_mday % lt.tm_hour % lt.tm_min % lt.tm_sec );

    if (mkdir(Globals::base_path.c_str(), 493 /*755*/) < 0 && errno != EEXIST)
    {
        K::Logger::Logg(C_ERROR, FMT("unable to create log directory '%s': %s!") % Globals::base_path % strerror(errno));
        return false;
    }

    std::string gen_tmp = Globals::base_path + std::string("generic.log");
    Globals::generic_file.open(gen_tmp.c_str());

    if (!Globals::generic_file.good())
    {
        K::Logger::Logg(C_ERROR, FMT("could not open file '%s': %s") % gen_tmp % strerror(errno));
        return false;
    }

    // inserts other file descriptors (TODO: delete this when stopping logs) 
    K::Logger::Logg.add(O_GENERIC, &Globals::generic_file);
    return true;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_khomp_load)
{
    if(!start_log())
    {
        return SWITCH_STATUS_FALSE;
    };

    Globals::module_pool = pool;

    /* start config system! */
    Opt::initialize();

    /* read configuration first */
    Opt::obtain();

    /*
       Spawn our k3l global var that will be used along the module
       for sending info to the boards
    */

    if(!Board::initialize())
    {
        K::Logger::Logg(C_ERROR,"Error while initialize Board struct");
        return SWITCH_STATUS_TERM;
    }

    *module_interface = switch_loadable_module_create_module_interface(pool, "mod_khomp");

    Globals::khomp_endpoint_interface = static_cast<switch_endpoint_interface_t*>(switch_loadable_module_create_interface(*module_interface, SWITCH_ENDPOINT_INTERFACE));
    Globals::khomp_endpoint_interface->interface_name = "khomp";
    Globals::khomp_endpoint_interface->io_routines = &khomp_io_routines;
    Globals::khomp_endpoint_interface->state_handler = &khomp_state_handlers;

    /* Add all the specific API functions */
    SWITCH_ADD_API(Globals::api_interface, "khomp", "Khomp Menu", apiKhomp, KHOMP_SYNTAX);
    switch_console_set_complete("add khomp help");
    switch_console_set_complete("add khomp show");
    switch_console_set_complete("add khomp show info");
    switch_console_set_complete("add khomp show links");
    switch_console_set_complete("add khomp show channels");
    switch_console_set_complete("add khomp show conf");

    Board::initializeHandlers();

    /* indicate that the module should continue to be loaded */
    return SWITCH_STATUS_SUCCESS;
}

/*
SWITCH_MODULE_RUNTIME_FUNCTION(mod_khomp_runtime)
{
    return SWITCH_STATUS_TERM;
}
*/

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_khomp_shutdown)
{
    int x = 0;

    Globals::running = -1;

    while (Globals::running) {
        if (x++ > 100) {
            break;
        }
        switch_yield(20000);
    }

    /* Free dynamically allocated strings */
//    switch_safe_free(Opt::_dialplan);
//    switch_safe_free(Opt::_codec_string);
//    switch_safe_free(Opt::_codec_rates_string);
//    switch_safe_free(Opt::_ip);

    /* Finnish him! */
    DBG(FUNC,"Unloading mod_khomp...")

    Board::finalizeHandlers();

    Board::finalize();

    DBG(FUNC,"Successfully Unloaded mod_khomp")

    return SWITCH_STATUS_SUCCESS;
}

/*!
   \brief khomp API definition
   TODO: Add as xml modifier
*/
SWITCH_STANDARD_API(apiKhomp)
{
    char *argv[10] = { 0 };
    int argc = 0;
    void *val;
    char *myarg = NULL;
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    /* We should not ever get a session here */
    if (session) return SWITCH_STATUS_FALSE;

    if (zstr(cmd))
    {
        stream->write_function(stream, "%s", KHOMP_SYNTAX);
        return status;
    }

    if (!(myarg = strdup(cmd))) return SWITCH_STATUS_MEMERR;


    if ((argc = switch_separate_string(myarg, ' ', argv, (sizeof(argv) / sizeof(argv[0])))) < 1) {
        stream->write_function(stream, "%s", KHOMP_SYNTAX);
        goto done;
    }

    /* Below show ... */
    if (argv[0] && !strncasecmp(argv[0], "show", 4)) {
        /* Show the API summary and information */
        if (argv[1] && !strncasecmp(argv[1], "info", 4)) {
            printSystemSummary(stream);
        }
        /* Show all the links and their status */
        if (argv[1] && !strncasecmp(argv[1], "links", 5)) {
            /* TODO: Let show specific boards/links */
            apiPrintLinks(stream, NULL, NULL);
        }
        /* Output span configuration */
        if (argv[1] && !strncasecmp(argv[1], "conf", 4)) {
            Opt::printConfiguration(stream);
        }
        // Show all channels from all boards and all links
        if (argv[1] && !strncasecmp(argv[1], "channels", 8)) {
            /* TODO: Let show specific channels */
            apiPrintChannels(stream);
            //printChannels(stream, NULL, NULL);
        }

    } else {
        stream->write_function(stream, "%s", KHOMP_SYNTAX);
    }

done:
    switch_safe_free(myarg);
    return status;

}


void printChannels(switch_stream_handle_t* stream, unsigned short device)
{
    for (unsigned short channel = 0 ;
            channel < Globals::k3lapi.channel_count(device) ; channel++)
    {
        stream->write_function(stream,
            "| %d,%02d |   unused   | %11s | %-36s |\n",
            device, channel,
            Globals::k3lutil.callStatus(device, channel).c_str(),
            Globals::k3lutil.channelStatus(device, channel).c_str());
    }

}


void apiPrintChannels(switch_stream_handle_t* stream)
{

    stream->write_function(stream, "\n"
" ------------------------------------------------------------------------\n");
    stream->write_function(stream,
"|--------------------- Khomp Channels and Connections -------------------|\n");
    stream->write_function(stream,
"|------------------------------------------------------------------------|\n");
    stream->write_function(stream,
"|  hw  | freeswitch |  khomp call |             khomp channel            |\n");
    stream->write_function(stream,
"|  id  |   status   |    status   |                status                |\n");
    stream->write_function(stream,
" ------------------------------------------------------------------------\n");

    for (unsigned short dev = 0 ; dev < Globals::k3lapi.device_count() ; dev++)
    {
        printChannels(stream, dev);
    }


    stream->write_function(stream,
" ------------------------------------------------------------------------\n");

}


void printLinks(switch_stream_handle_t* stream, unsigned int device)
{

    stream->write_function(stream,
"|------------------------------------------------------------------------|\n");

    switch (Globals::k3lutil.physicalLinkCount(device, true))
    {
        case 1:
        {
            std::string str_link0 = Globals::k3lutil.linkStatus(device, 0);

            try
            {
                K3L_LINK_CONFIG & conf0 = Globals::k3lapi.link_config(device,0);

                if (conf0.ReceivingClock & 0x01)
                    str_link0 += " (sync)";
            }
            catch(K3LAPI::invalid_channel & e)
            {}

                stream->write_function(stream,
                        "| Link '0' on board '%d': %-47s |\n",
                        device, str_link0.c_str());
            break;
        }

        case 2:
        {
            std::string str_link0 = Globals::k3lutil.linkStatus(device, 0);
            std::string str_link1 = Globals::k3lutil.linkStatus(device, 1);

            try
            {

                K3L_LINK_CONFIG & conf0 = Globals::k3lapi.link_config(device,0);
                K3L_LINK_CONFIG & conf1 = Globals::k3lapi.link_config(device,1);

                if (conf0.ReceivingClock & 0x01)
                    str_link0 += " (sync)";

                if (conf1.ReceivingClock & 0x01)
                    str_link1 += " (sync)";
            }
            catch(K3LAPI::invalid_channel & e)
            {}

                stream->write_function(stream,
                        "|------ Link '0' on board '%d' ------|"
                        "|------ Link '1' on board '%d' ------|\n",
                        device, device);

                stream->write_function(stream,
                        "| %-33s || %-33s |\n",
                        str_link0.c_str(), str_link1.c_str());

            break;
        }
        default:
        {
                stream->write_function(stream,
                    "| Board '%d': %-59s |\n",
                    device, "No links available.");
            break;
        }
    }
}

void apiPrintLinks(switch_stream_handle_t* stream, unsigned int device,
        unsigned int link)
{

    stream->write_function(stream, "\n"
" ------------------------------------------------------------------------\n");
    stream->write_function(stream,
"|--------------------------- Khomp Links List ---------------------------|\n");


    for(device = 0; device < Globals::k3lapi.device_count() ; device++)
    {
        printLinks(stream, device);
    }

    stream->write_function(stream,
" ------------------------------------------------------------------------\n");
}


void printSystemSummary(switch_stream_handle_t* stream) {

    K3L_API_CONFIG apiCfg;

    stream->write_function(stream," ------------------------------------------------------------------\n");
    stream->write_function(stream, "|---------------------- Khomp System Summary ----------------------|\n");
    stream->write_function(stream, "|------------------------------------------------------------------|\n");

    if (k3lGetDeviceConfig(-1, ksoAPI, &apiCfg, sizeof(apiCfg)) == ksSuccess)
    {
        stream->write_function(stream, "| K3L API %d.%d.%d [m.VPD %d] - %-38s |\n"
                     , apiCfg.MajorVersion , apiCfg.MinorVersion , apiCfg.BuildVersion
                     , apiCfg.VpdVersionNeeded , apiCfg.StrVersion);
    }

    for (unsigned int i = 0; i < Globals::k3lapi.device_count(); i++)
    {
        K3L_DEVICE_CONFIG & devCfg = Globals::k3lapi.device_config(i);

        stream->write_function(stream, " ------------------------------------------------------------------\n");

        switch (Globals::k3lapi.device_type(i))
        {
            /* E1 boards */
            case kdtE1:
            case kdtConf:
            case kdtPR:
            case kdtE1GW:
            case kdtE1IP:
            case kdtE1Spx:
            case kdtGWIP:
            case kdtFXS:
            case kdtFXSSpx:
            {
                K3L_E1600A_FW_CONFIG dspAcfg;
                K3L_E1600B_FW_CONFIG dspBcfg;

                if ((k3lGetDeviceConfig(i, ksoFirmware + kfiE1600A, &dspAcfg, sizeof(dspAcfg)) == ksSuccess) &&
                    (k3lGetDeviceConfig(i, ksoFirmware + kfiE1600B, &dspBcfg, sizeof(dspBcfg)) == ksSuccess))
                {
                    stream->write_function(stream, "| [[ %02d ]] %s, serial '%04d', %02d channels, %d links.|\n"
                                , i , "E1" , atoi(devCfg.SerialNumber) , devCfg.ChannelCount , devCfg.LinkCount);
                    stream->write_function(stream, "| * DSP A: %s, DSP B: %s - PCI bus: %02d, PCI slot: %02d %s|\n"
                                , dspAcfg.DspVersion , dspBcfg.DspVersion , devCfg.PciBus , devCfg.PciSlot
                                , std::string(18 - strlen(dspAcfg.DspVersion) - strlen(dspBcfg.DspVersion), ' ').c_str());
                    stream->write_function(stream, "| * %-62s |\n" , dspAcfg.FwVersion);
                    stream->write_function(stream, "| * %-62s |\n" , dspBcfg.FwVersion);

                }

                break;
            }

            /* analog boards */
            case kdtFXO:
            case kdtFXOVoIP:
            {
                K3L_FXO80_FW_CONFIG dspCfg;

                if (k3lGetDeviceConfig(i, ksoFirmware + kfiFXO80, &dspCfg, sizeof(dspCfg)) == ksSuccess)
                {
                    stream->write_function(stream, "| [[ %02d ]] %s, serial '%04d', %02d channels. %s|\n"
                                , i , "KFXO80" , atoi(devCfg.SerialNumber) , devCfg.ChannelCount
                                , std::string(26 - 6, ' ').c_str());
                    stream->write_function(stream, "| * DSP: %s - PCI bus: %02d, PCI slot: %02d%s|\n"
                                , dspCfg.DspVersion , devCfg.PciBus , devCfg.PciSlot
                                , std::string(30 - strlen(dspCfg.DspVersion), ' ').c_str());
                    stream->write_function(stream, "| * %-62s |\n" , dspCfg.FwVersion);
                }

            }

            case kdtGSM:
            case kdtGSMSpx:
            {
                K3L_GSM40_FW_CONFIG dspCfg;

                if (k3lGetDeviceConfig(i, ksoFirmware + kfiGSM40, &dspCfg, sizeof(dspCfg)) == ksSuccess)
                {
                    stream->write_function(stream, "| [[ %02d ]] %s, serial '%04d', %02d channels. %s|\n"
                        , i , "KGSM" , atoi(devCfg.SerialNumber) , devCfg.ChannelCount
                        , std::string(26 - 4, ' ').c_str());

                    stream->write_function(stream, "| * DSP: %s - PCI bus: %02d, PCI slot: %02d%s|\n"
                                , dspCfg.DspVersion , devCfg.PciBus , devCfg.PciSlot
                                , std::string(30 - strlen(dspCfg.DspVersion), ' ').c_str());

                    stream->write_function(stream, "| * %-62s |\n" , dspCfg.FwVersion);
                }

                break;
            }
            default:
                stream->write_function(stream, "| [[ %02d ]] Unknown type '%02d'! Please contact Khomp support for help! |\n"
                    , i , Globals::k3lapi.device_type(i));
                break;
        }
    }

    stream->write_function(stream, " ------------------------------------------------------------------\n");
}
/* End of helper functions */


extern "C" void Kstdcall khomp_audio_listener (int32 deviceid, int32 objectid, byte * read_buffer, int32 read_size)
{
    Board::KhompPvt * pvt = Board::get(deviceid, objectid);

    if (!pvt)
        return;

    /* add listener audio to the read buffer */
    if (!pvt->_reader_frames.give((const char *)read_buffer, read_size))
    {
        DBG(FUNC, OBJ_FMT(deviceid,objectid, "Reader buffer full (read_size: %d)") % read_size);
    }

    /* push audio from the write buffer */
    switch_frame_t * fr = pvt->_writer_frames.pick();

    if (!fr)
    {
        /*
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,
            "Writer buffer empty! (%u,%02u).\n",
            pvt->target().device, pvt->target().object);
        */
        return;
    }

    if (!pvt->call()->_flags.check(Kflags::STREAM_UP))
    {
        DBG(FUNC, PVT_FMT(pvt->target(), "Stream not enabled, skipping write..."));
        return;
    }

    /* will be used below for CM_ADD_STREAM_BUFFER */
    struct
    {
        const byte * buff;
        size_t       size;
    }
    write_packet = { (const byte *)0, 0 };

    /* what is the frame type? */
    switch (fr->flags)
    {
        case SFF_NONE:
        {
            write_packet.buff = (const byte *) fr->data;
            write_packet.size = (size_t)       fr->datalen;


            pvt->command(KHOMP_LOG, CM_ADD_STREAM_BUFFER,
                    (const char *)&write_packet);

            break;
        }

        case SFF_CNG:
        {
            write_packet.buff = (const byte *) Board::_cng_buffer;
            write_packet.size = (size_t)       Globals::cng_buffer_size;

            pvt->command(KHOMP_LOG, CM_ADD_STREAM_BUFFER,
                    (const char *)&write_packet);

            break;
        }

        default:
            DBG(FUNC,"DROPPING AUDIO...")
            /* TODO: log something here... */
            break;
    }
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 expandtab:
 */
