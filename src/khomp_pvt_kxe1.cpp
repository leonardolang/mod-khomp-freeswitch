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

#include "khomp_pvt_kxe1.h"
#include "lock.h"
#include "logger.h"

//BoardE1::KhompPvtE1::KhompPvtE1(K3LAPI::target & target) : KhompPvt(target) {};

bool BoardE1::KhompPvtE1::isOK()
{
    try
    {
        ScopedPvtLock lock(this);

        K3L_CHANNEL_STATUS status;

        if (k3lGetDeviceStatus (_target.device, _target.object + ksoChannel, &status, sizeof (status)) != ksSuccess)
            return false;

        return   ((status.AddInfo == kecsFree) ||
                (!(status.AddInfo & kecsLocalFail) &&
                 !(status.AddInfo & kecsRemoteLock)));
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "unable to lock %s!") % err._msg.c_str() );
    }

    return false;
}

void BoardE1::KhompPvtE1::onChannelRelease(K3L_EVENT *e)
{
    DBG(FUNC, PVT_FMT(_target, "(E1) c"));

    try
    {
        ScopedPvtLock lock(this);

        call()->_flags.clear(Kflags::HAS_PRE_AUDIO);

        command(KHOMP_LOG, CM_ENABLE_CALL_ANSWER_INFO);
    }
    catch(ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "(E1) r (unable to lock %s!)") % err._msg.c_str() );
        return;
    }
    
    KhompPvt::onChannelRelease(e);
    
    DBG(FUNC, PVT_FMT(_target, "(E1) r"));   
}

bool BoardE1::KhompPvtE1::onCallSuccess(K3L_EVENT *e)
{
    DBG(FUNC, PVT_FMT(_target, "(E1) c"));

    try
    {
        ScopedPvtLock lock(this);


        if (call()->_pre_answer)
        {
            start_listen();
            start_stream();
        }
        else
        {
            //TODO: Gerar Ringback caso necessario
            call()->_flags.set(Kflags::GEN_PBX_RING);
            //idx.pbx_ring = pvt->pvt_timer.add(K::opt::ringback_pbx_delay,
            //                &K::timers::pbx_ring_gen, pvt, TM_VAL_CALL);
        }


    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,FMT("(E1) r (unable to lock %s!)") % err._msg.c_str() );
        return ksFail;
    }
    
    KhompPvt::onCallSuccess(e);
    
    DBG(FUNC, PVT_FMT(_target, "(E1) r"));

    return ksSuccess;
}


void BoardE1::onLinkStatus(K3L_EVENT *e)
{
    DBG(FUNC, D("Link %02hu on board %02hu changed") % e->AddInfo % e->DeviceId);

    /* Fire a custom event about this */
    switch_event_t * event;
    if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, KHOMP_EVENT_MAINT) == SWITCH_STATUS_SUCCESS)
    {
        //khomp_add_event_board_data(e->AddInfo, event);
        Board::khomp_add_event_board_data(K3LAPI::target(Globals::k3lapi, K3LAPI::target::LINK, e->DeviceId, e->AddInfo), event);

        switch_event_add_header(event, SWITCH_STACK_BOTTOM, "EV_LINK_STATUS", "%d", e->AddInfo);
        switch_event_fire(&event);
    }
}

bool BoardE1::KhompPvtISDN::onIsdnProgressIndicator(K3L_EVENT *e)
{
    //TODO: Do we need return something ?
    try
    {
        ScopedPvtLock lock(this);

        switch (e->AddInfo)
        {
            case kq931pTonesMaybeAvailable:
            case kq931pTonesAvailable:
                if (!call()->_is_progress_sent)
                {
                    call()->_is_progress_sent = true;
                    //Sinaliza para o Freeswitch PROGRESS
                    switch_channel_t * channel = switch_core_session_get_channel(session());
                    if(channel)
                    {
                        DBG(FUNC, PVT_FMT(_target,"Pre answer"));

                        //pvt->signal_state(SWITCH_CONTROL_PROGRESS);
                        //switch_channel_pre_answer(channel);
                        switch_channel_mark_pre_answered(channel);
                    }
                    
                }
                break;
            case kq931pDestinationIsNonIsdn:
            case kq931pOriginationIsNonIsdn:
            case kq931pCallReturnedToIsdn:
            default:
                break;
        }

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "unable to lock %s!") % err._msg.c_str() );
    }
}


bool BoardE1::KhompPvtISDN::onNewCall(K3L_EVENT *e)
{
    DBG(FUNC,PVT_FMT(_target,"(ISDN) c"));   

    bool isdn_reverse_charge = false;
    
    try
    {
        std::string isdn_reverse_charge_str =
            Globals::k3lapi.get_param(e, "isdn_reverse_charge");

        isdn_reverse_charge = Strings::toboolean(isdn_reverse_charge_str);

        if(isdn_reverse_charge)
        {
            ScopedPvtLock lock(this);
            call()->_collect_call = true;
        }
    }
    catch(K3LAPI::get_param_failed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target,"unable to get param '%s': %s") % err.name.c_str() % Verbose::status(err.rc).c_str());
    }
    catch (Strings::invalid_value & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target, "unable to get param '%s'") % err.value().c_str());
    }
    catch(ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "(ISDN) r (unable to lock %s!)") % err._msg.c_str() );
        return ksFail;
    }

    bool ret = KhompPvtE1::onNewCall(e); 

    DBG(FUNC, PVT_FMT(_target, "(ISDN) r"));   

    return ret;
}


bool BoardE1::KhompPvtISDN::onCallSuccess(K3L_EVENT *e)
{
    try
    {
        ScopedPvtLock lock(this);

        if(e->AddInfo > 0)
        {
            callISDN()->_isdn_cause = e->AddInfo;
        }

        lock.unlock();

        KhompPvtE1::onCallSuccess(e);

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    return ksSuccess;
}

bool BoardE1::KhompPvtISDN::onCallFail(K3L_EVENT *e)
{
    try
    {
        ScopedPvtLock lock(this);

        if(e->AddInfo > 0)
        {
            callISDN()->_isdn_cause = e->AddInfo;
        }

        setHangupCause(causeFromCallFail(e->AddInfo),true);

        lock.unlock();

        KhompPvtE1::onCallFail(e);

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    return ksSuccess;
}


RingbackDefs::RingbackStType BoardE1::KhompPvtISDN::sendRingBackStatus(int rb_value)
{
    DBG(FUNC, PVT_FMT(target(), "this is the rdsi ringback procedure"));


    std::string cause = (rb_value == -1 ? "" : STG(FMT("isdn_cause=\"%d\"") % rb_value));
    return (command(KHOMP_LOG, CM_RINGBACK, cause.c_str()) ?
            RingbackDefs::RBST_SUCCESS : RingbackDefs::RBST_FAILURE);
}

bool BoardE1::KhompPvtISDN::sendPreAudio(int rb_value)
{
    if(!KhompPvtE1::sendPreAudio(rb_value))
        return false;


    DBG(FUNC,PVT_FMT(_target, "doing the ISDN pre_connect"));   

    if (call()->_flags.check(Kflags::HAS_PRE_AUDIO))
    {
        DBG(FUNC, PVT_FMT(target(), "already pre_connect"));
        return true;
    }
    else
    {
        bool result = command(KHOMP_LOG, CM_PRE_CONNECT);

        if (result)
            call()->_flags.set(Kflags::HAS_PRE_AUDIO);

        return result;
    }
}


int BoardE1::KhompPvtE1::makeCall(std::string params)
{
    if(callE1()->_call_info_drop == 0 && !callE1()->_call_info_report)
    {
        command(KHOMP_LOG, CM_DISABLE_CALL_ANSWER_INFO);
    }

    int ret = KhompPvt::makeCall(params);

    if(ret == ksSuccess)
    {
        start_listen();
    }
    else
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "Fail on make call"));
    }   

    return ret;
}

int BoardE1::KhompPvtE1::doChannelAnswer(CommandRequest &msg)
{
    DBG(FUNC, PVT_FMT(_target, "(E1) c"));
    /*
    try
    {
        ScopedPvtLock lock(this);
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,PVT_FMT(_target,"unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }
    */
        
    KhompPvt::doChannelAnswer(msg); 

    DBG(FUNC, PVT_FMT(_target, "(E1) r"));
}

bool BoardE1::KhompPvtE1::indicateBusyUnlocked(int cause, bool sent_signaling)
{
    DBG(FUNC, PVT_FMT(_target, "(E1) c"));

    if(!KhompPvt::indicateBusyUnlocked(cause, sent_signaling))
    {
        DBG(FUNC, PVT_FMT(_target, "(E1) r (false)"));
        return false;
    }

    if(call()->_flags.check(Kflags::IS_INCOMING))
    {
        if(!call()->_flags.check(Kflags::CONNECTED) && !sent_signaling)
        {
            if(!call()->_flags.check(Kflags::HAS_PRE_AUDIO))
            {
                int rb_value = callFailFromCause(call()->_hangup_cause);
                DBG(FUNC, PVT_FMT(target(), "sending the busy status"));

                if (sendRingBackStatus(rb_value) == RingbackDefs::RBST_UNSUPPORTED)
                {
                    DBG(FUNC, PVT_FMT(target(), "falling back to audio indication!"));
                    /* stop the line audio */
                    stop_stream();

                    /* just pre connect, no ringback */
                    if (!sendPreAudio())
                        DBG(FUNC, PVT_FMT(target(), "everything else failed, just sending audio indication..."));

                    /* be very specific about the situation. */
                    mixer(KHOMP_LOG, 1, kmsGenerator, kmtBusy);
                }
            }
            else
            {
                DBG(FUNC, PVT_FMT(target(), "going to play busy"));

                /* stop the line audio */
                stop_stream();

                /* be very specific about the situation. */
                mixer(KHOMP_LOG, 1, kmsGenerator, kmtBusy);

            }
        }
        else
        {
            /* already connected or sent signaling... */
            mixer(KHOMP_LOG, 1, kmsGenerator, kmtBusy);
        }

    }
    else if(call()->_flags.check(Kflags::IS_OUTGOING))
    {
        /* already connected or sent signaling... */
        mixer(KHOMP_LOG, 1, kmsGenerator, kmtBusy);
    }

    DBG(FUNC,PVT_FMT(_target, "(E1) r"));
    
    return true; 
}

int BoardE1::KhompPvtISDN::makeCall(std::string params)
{
    DBG(FUNC,PVT_FMT(_target, "(ISDN) c"));   

    CallISDN * call = callISDN();

    if(call->_uui_descriptor != -1)
    {
        DBG(FUNC,PVT_FMT(_target, "got userinfo"));   

        /* grab this information first, avoiding latter side-effects */
        const char * info_data = call->_uui_information.c_str();
        size_t       info_size = std::min(call->_uui_information.size(), (size_t)KMAX_USER_USER_LEN);

        KUserInformation info;

        info.ProtocolDescriptor = call->_uui_descriptor;
        info.UserInfoLength = info_size;

        memcpy((void *) info.UserInfo, (const void *) info_data, info_size);

        if (!command(KHOMP_LOG, CM_USER_INFORMATION, (const char *)&info))
        {
            DBG(FUNC,PVT_FMT(_target, "UUI could not be sent before dialing!"));   
        }

        call->_uui_descriptor = -1;
        call->_uui_information.clear();
    }

    int ret = KhompPvtE1::makeCall(params);

    call->_cleanup_upon_hangup = (ret == ksInvalidParams || ret == ksBusy);
    
    DBG(FUNC,PVT_FMT(_target, "(ISDN) r"));   

    return ret;
}

void BoardE1::KhompPvtISDN::reportFailToReceive(int fail_code)
{
    KhompPvt::reportFailToReceive(fail_code);

    if(fail_code != -1)
    {
        DBG(FUNC,PVT_FMT(_target,"sending a 'unknown number' message/audio")); 

        if(sendRingBackStatus(fail_code) == RingbackDefs::RBST_UNSUPPORTED)
        {
            sendPreAudio(RingbackDefs::RB_SEND_DEFAULT);
            cadenceStart(Kflags::PLAY_FASTBUSY);
        }
    }
    else
    {
        DBG(FUNC, PVT_FMT(_target, "sending fast busy audio directly"));

        sendPreAudio(RingbackDefs::RB_SEND_DEFAULT);
        cadenceStart(Kflags::PLAY_FASTBUSY);
    }
}

int BoardE1::KhompPvtISDN::doChannelAnswer(CommandRequest &cmd)
{
    try
    {
        ScopedPvtLock lock(this);

        // is this a collect call?
        bool has_recv_collect_call = _call->_collect_call;

        // do we have to drop collect calls?
        bool has_drop_collect_call = Opt::_drop_collect_call
            || _call->_flags.check(Kflags::DROP_COLLECT)
            || _call->_flags.check(Kflags::FILTER_COLLECT);

        // do we have to drop THIS call?
        bool do_drop_call = has_drop_collect_call && has_recv_collect_call;

        if(has_drop_collect_call) 
        {
            usleep(75000);

            DBG(FUNC, PVT_FMT(target(), "disconnecting collect call"));
            //TODO: Define what is SCE
            //        command(KHOMP_LOG,CM_DISCONNECT,SCE_HIDE);
            command(KHOMP_LOG,CM_DISCONNECT);

            // thou shalt not talk anymore!
            stop_listen();
            stop_stream();
            //TODO: Checar se retorna aqui
        }
    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(_target, "unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    KhompPvtE1::doChannelAnswer(cmd);
}

int BoardE1::KhompPvtR2::makeCall(std::string params)
{
    DBG(FUNC,PVT_FMT(_target, "(R2) c"));   

    if (callR2()->_r2_category != -1)
        params += STG(FMT("r2_categ_a=\"%ld\"")
                % callR2()->_r2_category);

    int ret = KhompPvtE1::makeCall(params);

    call()->_cleanup_upon_hangup = (ret == ksInvalidParams);
    
    DBG(FUNC,PVT_FMT(_target, "(R2) r"));   

    return ret;
}


int BoardE1::KhompPvtR2::doChannelAnswer(CommandRequest &cmd)
{
    try
    {
        ScopedPvtLock lock(this);

        // is this a collect call?
        bool has_recv_collect_call = _call->_collect_call;

        // do we have to drop collect calls?
        bool has_drop_collect_call = Opt::_drop_collect_call
            || _call->_flags.check(Kflags::DROP_COLLECT)
            || _call->_flags.check(Kflags::FILTER_COLLECT);

        // do we have to drop THIS call?
        bool do_drop_call = has_drop_collect_call && has_recv_collect_call;

        // do we have to send ringback? yes we need !!!
        if(call()->_flags.check(Kflags::NEEDS_RINGBACK_CMD))
        {       
            call()->_flags.clear(Kflags::NEEDS_RINGBACK_CMD);
            std::string cause = ( do_drop_call ? STG(FMT("r2_cond_b=\"%d\"") % kgbBusy) : "" );
            command(KHOMP_LOG,CM_RINGBACK,cause.c_str());

            usleep(75000);
        }

        if(has_drop_collect_call) 
        {
            DBG(FUNC, PVT_FMT(target(), "dropping collect call"));
            command(KHOMP_LOG, CM_DROP_COLLECT_CALL);
        }

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR,PVT_FMT(_target,"unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    KhompPvtE1::doChannelAnswer(cmd);
}

int BoardE1::KhompPvtISDN::causeFromCallFail(int fail)
{
    int switch_cause = SWITCH_CAUSE_USER_BUSY;

    if (fail <= 127) 
        switch_cause = fail;
    else 
        switch_cause = SWITCH_CAUSE_INTERWORKING;

    return switch_cause;
}

void BoardE1::KhompPvtR2::reportFailToReceive(int fail_code)
{
    KhompPvt::reportFailToReceive(fail_code);

    if (Opt::_r2_strict_behaviour && fail_code != -1)
    {
        DBG(FUNC,PVT_FMT(_target,"sending a 'unknown number' message/audio")); 

        if(sendRingBackStatus(fail_code) == RingbackDefs::RBST_UNSUPPORTED)
        {
            sendPreAudio(RingbackDefs::RB_SEND_DEFAULT);
            cadenceStart(Kflags::PLAY_FASTBUSY);
        }
    }
    else
    {
        DBG(FUNC, PVT_FMT(_target, "sending fast busy audio directly"));

        sendPreAudio(RingbackDefs::RB_SEND_DEFAULT);
        cadenceStart(Kflags::PLAY_FASTBUSY);
    }
}

int BoardE1::KhompPvtR2::causeFromCallFail(int fail)
{
    int switch_cause = SWITCH_CAUSE_USER_BUSY;

    try
    {
        bool handled = false;

        switch (_r2_country)
        {
            case Verbose::R2_COUNTRY_ARG:
                switch (fail)
                {
                    case kgbArBusy:
                        switch_cause = SWITCH_CAUSE_USER_BUSY;
                        break;
                    case kgbArNumberChanged:
                        switch_cause = SWITCH_CAUSE_NUMBER_CHANGED;
                        break;
                    case kgbArCongestion:
                        switch_cause = SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION;
                        break;
                    case kgbArInvalidNumber:
                        switch_cause = SWITCH_CAUSE_UNALLOCATED_NUMBER;
                        break;
                    case kgbArLineOutOfOrder:
                        switch_cause = SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL;
                        break;
                }
                handled = true;
                break;

            case Verbose::R2_COUNTRY_BRA:
                switch (fail)
                {
                    case kgbBrBusy:
                        switch_cause = SWITCH_CAUSE_USER_BUSY;
                        break;
                    case kgbBrNumberChanged:
                        switch_cause = SWITCH_CAUSE_NUMBER_CHANGED;
                        break;
                    case kgbBrCongestion:
                        switch_cause = SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION;
                        break;
                    case kgbBrInvalidNumber:
                        switch_cause = SWITCH_CAUSE_UNALLOCATED_NUMBER;
                        break;
                    case kgbBrLineOutOfOrder:
                        switch_cause = SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL;
                        break;
                }
                handled = true;
                break;
            case Verbose::R2_COUNTRY_CHI:
                    switch (fail)
                    {
                        case kgbClBusy:
                            switch_cause = SWITCH_CAUSE_USER_BUSY;
                            break;
                        case kgbClNumberChanged:
                            switch_cause = SWITCH_CAUSE_NUMBER_CHANGED;
                            break;
                        case kgbClCongestion:
                            switch_cause = SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION;
                            break;
                        case kgbClInvalidNumber:
                            switch_cause = SWITCH_CAUSE_UNALLOCATED_NUMBER;
                            break;
                        case kgbClLineOutOfOrder:
                            switch_cause = SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL;
                            break;
                    }
                handled = true;
                break;

            case Verbose::R2_COUNTRY_MEX:
                switch (fail)
                {
                    case kgbMxBusy:
                        switch_cause = SWITCH_CAUSE_USER_BUSY;
                        break;
                }
                handled = true;
                break;

            case Verbose::R2_COUNTRY_URY:
                switch (fail)
                {
                    case kgbUyBusy:
                        switch_cause = SWITCH_CAUSE_USER_BUSY;
                        break;
                    case kgbUyNumberChanged:
                        switch_cause = SWITCH_CAUSE_NUMBER_CHANGED;
                        break;
                    case kgbUyCongestion:
                        switch_cause = SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION;
                        break;
                    case kgbUyInvalidNumber:
                        switch_cause = SWITCH_CAUSE_UNALLOCATED_NUMBER;
                        break;
                    case kgbUyLineOutOfOrder:
                        switch_cause = SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL;
                        break;
                }
                handled = true;
                break;
            case Verbose::R2_COUNTRY_VEN:
                switch (fail)
                {
                    case kgbVeBusy:
                        switch_cause = SWITCH_CAUSE_USER_BUSY;
                        break;
                    case kgbVeNumberChanged:
                        switch_cause = SWITCH_CAUSE_NUMBER_CHANGED;
                        break;
                    case kgbVeCongestion:
                        switch_cause = SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION;
                        break;
                    case kgbVeLineBlocked:
                        switch_cause = SWITCH_CAUSE_OUTGOING_CALL_BARRED;
                        break;
                }
                handled = true;
                break;
        }

        if (!handled)
            throw std::runtime_error("");
    }
    catch (...)
    {
        K::Logger::Logg(C_ERROR,
                PVT_FMT(_target, "country signaling not found, unable to report R2 hangup code.."));
    }

    return switch_cause;

}

int BoardE1::KhompPvtR2::callFailFromCause(int cause)
{
    int k3l_fail = -1; // default

    try  
    {    
        bool handled = false;

        switch (_r2_country)
        {    
            case Verbose::R2_COUNTRY_ARG:
                switch (cause)
                {    
                    case SWITCH_CAUSE_UNALLOCATED_NUMBER:
                    case SWITCH_CAUSE_NO_ROUTE_TRANSIT_NET:
                    case SWITCH_CAUSE_NO_ROUTE_DESTINATION:
                    case SWITCH_CAUSE_INVALID_NUMBER_FORMAT:
                    case SWITCH_CAUSE_FACILITY_NOT_SUBSCRIBED:
                    case SWITCH_CAUSE_INCOMPATIBLE_DESTINATION:

                    case SWITCH_CAUSE_INCOMING_CALL_BARRED: /* ?? */
                    case SWITCH_CAUSE_OUTGOING_CALL_BARRED: /* ?? */
                        k3l_fail = kgbArInvalidNumber;
                        break;

                    case SWITCH_CAUSE_USER_BUSY:
                    case SWITCH_CAUSE_NO_USER_RESPONSE:
                    case SWITCH_CAUSE_CALL_REJECTED:
                        k3l_fail = kgbArBusy;
                        break;

                    case SWITCH_CAUSE_NUMBER_CHANGED:
                        k3l_fail = kgbArNumberChanged;
                        break;

                    case SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION:
                    case SWITCH_CAUSE_SWITCH_CONGESTION:

                    case SWITCH_CAUSE_NORMAL_CLEARING:
                    case SWITCH_CAUSE_NORMAL_UNSPECIFIED:
                    case SWITCH_CAUSE_CALL_AWARDED_DELIVERED: /* ?? */
                        // this preserves semantics..
                        k3l_fail = kgbArCongestion;
                        break;

                    case SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL:
                    case SWITCH_CAUSE_CHANNEL_UNACCEPTABLE:
                    case SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER:
                    case SWITCH_CAUSE_NETWORK_OUT_OF_ORDER:
                    case SWITCH_CAUSE_FACILITY_REJECTED:
                    case SWITCH_CAUSE_FACILITY_NOT_IMPLEMENTED:
                    case SWITCH_CAUSE_CHAN_NOT_IMPLEMENTED:
                    default:
                        k3l_fail = kgbArLineOutOfOrder;
                        break;
                }    
                handled = true;
                break;

            case Verbose::R2_COUNTRY_BRA:
                switch (cause)
                {    
                    case SWITCH_CAUSE_UNALLOCATED_NUMBER:
                    case SWITCH_CAUSE_NO_ROUTE_TRANSIT_NET:
                    case SWITCH_CAUSE_NO_ROUTE_DESTINATION:
                    case SWITCH_CAUSE_INVALID_NUMBER_FORMAT:
                    case SWITCH_CAUSE_FACILITY_NOT_SUBSCRIBED:
                    case SWITCH_CAUSE_INCOMPATIBLE_DESTINATION:

                    case SWITCH_CAUSE_INCOMING_CALL_BARRED: /* ?? */
                    case SWITCH_CAUSE_OUTGOING_CALL_BARRED: /* ?? */
                        k3l_fail = kgbBrInvalidNumber;
                        break;

                    case SWITCH_CAUSE_USER_BUSY:
                    case SWITCH_CAUSE_NO_USER_RESPONSE:
                    case SWITCH_CAUSE_CALL_REJECTED:
                        k3l_fail = kgbBrBusy;
                        break;

                    case SWITCH_CAUSE_NUMBER_CHANGED:
                        k3l_fail = kgbBrNumberChanged;
                        break;

                    case SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION:
                    case SWITCH_CAUSE_SWITCH_CONGESTION:

                    case SWITCH_CAUSE_NORMAL_CLEARING:
                    case SWITCH_CAUSE_NORMAL_UNSPECIFIED:
                    case SWITCH_CAUSE_CALL_AWARDED_DELIVERED: /* ?? */
                        // this preserves semantics..
                        k3l_fail = kgbBrCongestion;
                        break;

                    case SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL:
                    case SWITCH_CAUSE_CHANNEL_UNACCEPTABLE:
                    case SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER:
                    case SWITCH_CAUSE_NETWORK_OUT_OF_ORDER:
                    case SWITCH_CAUSE_FACILITY_REJECTED:
                    case SWITCH_CAUSE_FACILITY_NOT_IMPLEMENTED:
                    case SWITCH_CAUSE_CHAN_NOT_IMPLEMENTED:
                    default:
                        k3l_fail = kgbBrLineOutOfOrder;
                        break;
                }
                handled = true;
                break;

            case Verbose::R2_COUNTRY_CHI:
                switch (cause)
                {
                    case SWITCH_CAUSE_UNALLOCATED_NUMBER:
                    case SWITCH_CAUSE_NO_ROUTE_TRANSIT_NET:
                    case SWITCH_CAUSE_NO_ROUTE_DESTINATION:
                    case SWITCH_CAUSE_INVALID_NUMBER_FORMAT:
                    case SWITCH_CAUSE_FACILITY_NOT_SUBSCRIBED:
                    case SWITCH_CAUSE_INCOMPATIBLE_DESTINATION:

                    case SWITCH_CAUSE_INCOMING_CALL_BARRED: /* ?? */
                    case SWITCH_CAUSE_OUTGOING_CALL_BARRED: /* ?? */
                        k3l_fail = kgbClInvalidNumber;
                        break;

                    case SWITCH_CAUSE_USER_BUSY:
                    case SWITCH_CAUSE_NO_USER_RESPONSE:
                    case SWITCH_CAUSE_CALL_REJECTED:
                        k3l_fail = kgbClBusy;
                        break;

                    case SWITCH_CAUSE_NUMBER_CHANGED:
                        k3l_fail = kgbClNumberChanged;
                        break;

                    case SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION:
                    case SWITCH_CAUSE_SWITCH_CONGESTION:

                    case SWITCH_CAUSE_NORMAL_CLEARING:
                    case SWITCH_CAUSE_NORMAL_UNSPECIFIED:
                    case SWITCH_CAUSE_CALL_AWARDED_DELIVERED: /* ?? */
                        // this preserves semantics..
                        k3l_fail = kgbClCongestion;
                        break;

                    case SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL:
                    case SWITCH_CAUSE_CHANNEL_UNACCEPTABLE:
                    case SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER:
                    case SWITCH_CAUSE_NETWORK_OUT_OF_ORDER:
                    case SWITCH_CAUSE_FACILITY_REJECTED:
                    case SWITCH_CAUSE_FACILITY_NOT_IMPLEMENTED:
                    case SWITCH_CAUSE_CHAN_NOT_IMPLEMENTED:
                    default:
                        k3l_fail = kgbClLineOutOfOrder;
                        break;
                }
                handled = true;
                break;


            case Verbose::R2_COUNTRY_MEX:
                k3l_fail = kgbMxBusy;
                handled = true;
                break;


            case Verbose::R2_COUNTRY_URY:
                switch (cause)
                {
                    case SWITCH_CAUSE_UNALLOCATED_NUMBER:
                    case SWITCH_CAUSE_NO_ROUTE_TRANSIT_NET:
                    case SWITCH_CAUSE_NO_ROUTE_DESTINATION:
                    case SWITCH_CAUSE_INVALID_NUMBER_FORMAT:
                    case SWITCH_CAUSE_FACILITY_NOT_SUBSCRIBED:
                    case SWITCH_CAUSE_INCOMPATIBLE_DESTINATION:

                    case SWITCH_CAUSE_INCOMING_CALL_BARRED: /* ?? */
                    case SWITCH_CAUSE_OUTGOING_CALL_BARRED: /* ?? */
                        k3l_fail = kgbUyInvalidNumber;
                        break;

                    case SWITCH_CAUSE_USER_BUSY:
                    case SWITCH_CAUSE_NO_USER_RESPONSE:
                    case SWITCH_CAUSE_CALL_REJECTED:
                        k3l_fail = kgbUyBusy;
                        break;

                    case SWITCH_CAUSE_NUMBER_CHANGED:
                        k3l_fail = kgbUyNumberChanged;
                        break;

                    case SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION:
                    case SWITCH_CAUSE_SWITCH_CONGESTION:

                    case SWITCH_CAUSE_NORMAL_CLEARING:
                    case SWITCH_CAUSE_NORMAL_UNSPECIFIED:
                    case SWITCH_CAUSE_CALL_AWARDED_DELIVERED: /* ?? */
                        // this preserves semantics..
                        k3l_fail = kgbUyCongestion;
                        break;

                    case SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL:
                    case SWITCH_CAUSE_CHANNEL_UNACCEPTABLE:
                    case SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER:
                    case SWITCH_CAUSE_NETWORK_OUT_OF_ORDER:
                    case SWITCH_CAUSE_FACILITY_REJECTED:
                    case SWITCH_CAUSE_FACILITY_NOT_IMPLEMENTED:
                    case SWITCH_CAUSE_CHAN_NOT_IMPLEMENTED:
                    default:
                        k3l_fail = kgbUyLineOutOfOrder;
                        break;
                }
                handled = true;
                break;


            case Verbose::R2_COUNTRY_VEN:
                switch (cause)
                {
                    case SWITCH_CAUSE_INCOMING_CALL_BARRED:
                    case SWITCH_CAUSE_OUTGOING_CALL_BARRED:
                        k3l_fail = kgbVeLineBlocked;
                        break;

                    case SWITCH_CAUSE_NUMBER_CHANGED:
                        k3l_fail = kgbVeNumberChanged;
                        break;

                    case SWITCH_CAUSE_NORMAL_CIRCUIT_CONGESTION:
                    case SWITCH_CAUSE_SWITCH_CONGESTION:

                    case SWITCH_CAUSE_NORMAL_CLEARING:
                    case SWITCH_CAUSE_NORMAL_UNSPECIFIED:
                    case SWITCH_CAUSE_CALL_AWARDED_DELIVERED: /* ?? */
                        // this preserves semantics..
                        k3l_fail = kgbVeCongestion;
                        break;

                    case SWITCH_CAUSE_UNALLOCATED_NUMBER:
                    case SWITCH_CAUSE_NO_ROUTE_TRANSIT_NET:
                    case SWITCH_CAUSE_NO_ROUTE_DESTINATION:
                    case SWITCH_CAUSE_INVALID_NUMBER_FORMAT:
                    case SWITCH_CAUSE_FACILITY_NOT_SUBSCRIBED:
                    case SWITCH_CAUSE_INCOMPATIBLE_DESTINATION:

                    case SWITCH_CAUSE_REQUESTED_CHAN_UNAVAIL:
                    case SWITCH_CAUSE_CHANNEL_UNACCEPTABLE:
                    case SWITCH_CAUSE_DESTINATION_OUT_OF_ORDER:
                    case SWITCH_CAUSE_NETWORK_OUT_OF_ORDER:
                    case SWITCH_CAUSE_FACILITY_REJECTED:
                    case SWITCH_CAUSE_FACILITY_NOT_IMPLEMENTED:
                    case SWITCH_CAUSE_CHAN_NOT_IMPLEMENTED:

                    case SWITCH_CAUSE_USER_BUSY:
                    case SWITCH_CAUSE_NO_USER_RESPONSE:
                    case SWITCH_CAUSE_CALL_REJECTED:
                    default:
                        k3l_fail = kgbVeBusy;
                        break;
                }

                handled = true;
                break;

        }
    
        if (!handled)
            throw std::runtime_error("");
    }
    catch(...)
    {
        K::Logger::Logg(C_ERROR,PVT_FMT(_target,"country signaling not found, unable to report R2 hangup code."));
    }

    return k3l_fail;
}

int BoardE1::KhompPvtISDN::callFailFromCause(int cause)
{
    int k3l_fail = -1; // default

    if (cause <= 127) 
        k3l_fail = cause;
    else 
        k3l_fail = kq931cInterworking;

    return k3l_fail;
}

RingbackDefs::RingbackStType BoardE1::KhompPvtR2::sendRingBackStatus(int rb_value)
{
    DBG(FUNC,PVT_FMT(_target, "(p=%p) this is the r2 ringback procedure") % this);   

    std::string cause = (rb_value == -1 ? "" : STG(FMT("r2_cond_b=\"%d\"") % rb_value));
    return (command(KHOMP_LOG, CM_RINGBACK, cause.c_str()) ?
            RingbackDefs::RBST_SUCCESS : RingbackDefs::RBST_FAILURE);
}

bool BoardE1::KhompPvtR2::sendPreAudio(int rb_value)
{
    DBG(FUNC,PVT_FMT(_target, "must send R2 preaudio ?"));   
    if(!KhompPvtE1::sendPreAudio(rb_value))
        return false;


    DBG(FUNC,PVT_FMT(_target, "doing the R2 pre_connect wait..."));   

    /* wait some ms, just to be sure the command has been sent. */
    usleep(Opt::_r2_preconnect_wait * 1000);

    if (call()->_flags.check(Kflags::HAS_PRE_AUDIO))
    {
        DBG(FUNC, PVT_FMT(target(), "(p=%p) already pre_connect") % this);
        return true;
    }
    else
    {
        bool result = command(KHOMP_LOG, CM_PRE_CONNECT);

        if (result)
            call()->_flags.set(Kflags::HAS_PRE_AUDIO);

        return result;
    }
}


bool BoardE1::KhompPvtR2::onNewCall(K3L_EVENT *e)
{
    DBG(FUNC,PVT_FMT(_target, "(R2) c"));   

    std::string r2_categ_a;
    
    int status = Globals::k3lapi.get_param(e, "r2_categ_a", r2_categ_a);


    if (status == ksSuccess && !r2_categ_a.empty())
    {
        try
        {
            ScopedPvtLock lock(this);

            try 
            { 
                callR2()->_r2_category = Strings::toulong(r2_categ_a); 
            }
            catch (Strings::invalid_value e) 
            { 
            /* do nothing */ 
            };

            /* channel will know if is a collect call or not */
            if (callR2()->_r2_category == kg2CollectCall)
                call()->_collect_call = true;

        }
        catch(ScopedLockFailed & err)
        {
            K::Logger::Logg(C_ERROR, PVT_FMT(target(), "(R2) r (unable to lock %s!)") % err._msg.c_str() );
            return ksFail;
        }
    }

    //TODO: The variable ret 
    bool ret = KhompPvtE1::onNewCall(e);

    if(ret)
    {
        try
        {
            ScopedPvtLock lock(this);

            if (!Opt::_r2_strict_behaviour)
            {
                // any collect call ?
                //TODO: Must decide, how to configure
                //K::util::set_collectcall(pvt,NULL);

                // keeping the hardcore mode
                if (Opt::_drop_collect_call && call()->_collect_call)
                {
                    // kill, kill, kill!!!
                    sendRingBackStatus(callFailFromCause(SWITCH_CAUSE_CALL_REJECTED));
                    usleep(75000);
                }
                else
                {
                    // send ringback too! 
                    sendPreAudio(RingbackDefs::RB_SEND_DEFAULT);
                    start_stream();
                }
            }
            else
            {
                _call->_flags.set(Kflags::NEEDS_RINGBACK_CMD);
            }
        }
        catch(ScopedLockFailed & err)
        {
            K::Logger::Logg(C_ERROR, PVT_FMT(target(), "(R2) r (unable to lock %s!)") % err._msg.c_str() );
            return ksFail;
        }
    }

    DBG(FUNC,PVT_FMT(_target, "(R2) r"));   

    return ret;
}

bool BoardE1::KhompPvtR2::onCallSuccess(K3L_EVENT *e)
{
    try
    {
        ScopedPvtLock lock(this);

        if(e->AddInfo > 0)
        {
            callR2()->_r2_condition = e->AddInfo;
        }

        lock.unlock();

        KhompPvtE1::onCallSuccess(e);

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    return ksSuccess;
}

bool BoardE1::KhompPvtR2::onCallFail(K3L_EVENT *e)
{
    try
    {
        ScopedPvtLock lock(this);

        if(e->AddInfo > 0)
        {
            callR2()->_r2_condition = e->AddInfo;
        }

        setHangupCause(causeFromCallFail(e->AddInfo),true);

        lock.unlock();

        KhompPvtE1::onCallFail(e);

    }
    catch (ScopedLockFailed & err)
    {
        K::Logger::Logg(C_ERROR, PVT_FMT(target(), "unable to lock %s!") % err._msg.c_str() );
        return ksFail;
    }

    return ksSuccess;
}

