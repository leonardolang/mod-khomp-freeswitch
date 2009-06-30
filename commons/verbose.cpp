/*  
    KHOMP generic endpoint/channel library.
    Copyright (C) 2007-2009 Khomp Ind. & Com.  
  
  The contents of this file are subject to the Mozilla Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.

  Alternatively, the contents of this file may be used under the terms of the
  "GNU Lesser General Public License 2.1" license (the “LGPL" License), in which
  case the provisions of "LGPL License" are applicable instead of those above.

  If you wish to allow use of your version of this file only under the terms of
  the LGPL License and not to allow others to use your version of this file under
  the MPL, indicate your decision by deleting the provisions above and replace them
  with the notice and other provisions required by the LGPL License. If you do not
  delete the provisions above, a recipient may use your version of this file under
  either the MPL or the LGPL License.

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
    along with this library; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
  
*/

#include <strings.hpp>
#include <verbose.hpp>

/* util macros */
#define S(v) std::string(v)

/********************************************/

std::string Verbose::channelStatus(int32 dev, int32 obj, int32 cs)
{
    try
    {
        K3L_CHANNEL_CONFIG & config = _api.channel_config(dev, obj);
        return Verbose::channelStatus(config.Signaling, cs);
    }
    catch (...)
    {
        return std::string("<unknown>");
    }
}

std::string Verbose::event(int32 obj, K3L_EVENT *ev)
{
    try
    {
        K3L_CHANNEL_CONFIG & config = _api.channel_config(ev->DeviceId, obj);
        return Verbose::event(config.Signaling, obj, ev);
    }
    catch (...)
    {
        return Verbose::event(ksigInactive, obj, ev);
    }
}

/********************************************/

std::string Verbose::echoLocation(KEchoLocation ec)
{
    switch (ec)
    {
#if K3L_AT_LEAST(1,5,4)
        case kelNetwork: return S("kelNetwork");
#else
        case kelE1:      return S("kelE1");
#endif
        case kelCtBus:   return S("kelCtBus");
    };

    return S("<unknown>");
};

std::string Verbose::echoCancellerConfig(KEchoCancellerConfig ec)
{
    switch (ec)
    {
        case keccNotPresent:    return S("keccNotPresent");
        case keccOneSingleBank: return S("keccOneSingleBank");
        case keccOneDoubleBank: return S("keccOneDoubleBank");
        case keccTwoSingleBank: return S("keccTwoSingleBank");
        case keccTwoDoubleBank: return S("keccTwoDoubleBank");
        case keccFail:          return S("keccFail");
    };

    return S("<unknown>");
};

std::string Verbose::deviceName(KDeviceType dt, int32 model)
{
    std::string value;
    
    value += deviceType(dt);
    value += "-";
    value += deviceModel(dt, model);
    
    return value;
}

std::string Verbose::deviceType(KDeviceType dt)
{
    switch (dt)
    {
        case kdtE1:      return S("K2E1");

#if K3L_AT_LEAST(1,6,0)
        case kdtFXO:     return S("KFXO");
#else
        case kdtFX:      return S("KFXO");
#endif

        case kdtConf:    return S("KCONF");
        case kdtPR:      return S("KPR");
        case kdtE1GW:    return S("KE1GW");

#if K3L_AT_LEAST(1,6,0)
        case kdtFXOVoIP: return S("KFXVoIP");
#else
        case kdtFXVoIP:  return S("KFXVoIP");
#endif

#if K3L_AT_LEAST(1,5,0)
        case kdtE1IP:   return S("K2E1");
#endif
#if K3L_AT_LEAST(1,5,1)
        case kdtE1Spx:  return S("K2E1");
        case kdtGWIP:   return S("KGWIP");
#endif

#if K3L_AT_LEAST(1,6,0)
        case kdtFXS:     return S("KFXS");
        case kdtFXSSpx:  return S("KFXS");
        case kdtGSM:     return S("KGSM");
        case kdtGSMSpx:  return S("KGSM");
#endif
    }

    return STG(FMT("[deviceType='%d']") % (int)dt);
}

std::string Verbose::deviceModel(KDeviceType dt, int32 model)
{
    try
    {
        switch (dt)
        {
            case kdtE1:
                switch ((KE1DeviceModel)model)
                {
                    case kdmE1600:   return S("600");
                    case kdmE1600E:  return S("600E");
#if K3L_AT_LEAST(2,0,0)
                    case kdmE1600EX: return S("600EX");
#endif
                }

                throw internal_not_found();

#if K3L_AT_LEAST(1,6,0)
            case kdtFXO:
                switch ((KFXODeviceModel)model)
#else
            case kdtFX:
                switch ((KFXDeviceModel)model)
#endif
                {
#if K3L_AT_LEAST(1,6,0)
                    case kdmFXO80:    return S("80");
                    case kdmFXOHI:    return S("HI");
                    case kdmFXO160HI: return S("160HI");
#else
                    case kdmFXO80:    return S("80");
#endif
                }

                throw internal_not_found();

            case kdtConf:
                switch ((KConfDeviceModel)model)
                {
                    case kdmConf240:   return S("240");
                    case kdmConf120:   return S("120");
#if K3L_AT_LEAST(2,0,0)
                    case kdmConf240EX: return S("240EX");
                    case kdmConf120EX: return S("120EX");
#endif
                }

                throw internal_not_found();

            case kdtPR:
                switch ((KPRDeviceModel)model)
                {
#if K3L_AT_LEAST(1,6,0)
                    case kdmPR300v1:       return S("300v1");
                    case kdmPR300SpxBased: return S("300S");
#if K3L_AT_LEAST(2,0,0)
                    case kdmPR300EX:       return S("300EX");
#endif
#endif
                    case kdmPR300:         return S("300");
                }

                throw internal_not_found();

#if K3L_AT_LEAST(1,4,0)
            case kdtE1GW:
                switch ((KE1GWDeviceModel)model)
                {
#if K3L_AT_LEAST(1,6,0)
                    case kdmE1GW640:  return S("640");
#if K3L_AT_LEAST(2,0,0)
                    case kdmE1GW640EX:  return S("640EX");
#endif
#else
                    case kdmE1600V:  return S("600V");
                    case kdmE1600EV: return S("600EV");
#endif
                }

                throw internal_not_found();
#endif

#if K3L_AT_LEAST(1,6,0)
            case kdtFXOVoIP:
                switch ((KFXOVoIPDeviceModel)model)
                {
                    case kdmFXGW180:  return S("180");
                }

                throw internal_not_found();
                
#elif K3L_AT_LEAST(1,4,0)
            case kdtFXVoIP:
                switch ((KFXVoIPDeviceModel)model)
                {
                    case kdmFXO80V: return S("80V");
                }

                throw internal_not_found();
#endif

#if K3L_AT_LEAST(1,5,0)
            case kdtE1IP:
                switch ((KE1IPDeviceModel)model)
                {
#if K3L_AT_LEAST(1,6,0)
                    case kdmE1IP:  return S("E1IP");
#if K3L_AT_LEAST(2,0,0)
                    case kdmE1IPEX:  return S("E1IPEX");
#endif
#else
                    case kdmE1600EG: return S("600EG");
#endif
                }
                
                throw internal_not_found();
#endif

#if K3L_AT_LEAST(1,5,1)
            case kdtE1Spx:
                switch ((KE1SpxDeviceModel)model)
                {
                    case kdmE1Spx:    return S("SPX");
                    case kdm2E1Based: return S("SPX-2E1");
#if K3L_AT_LEAST(2,0,0)
                    case kdmE1SpxEX:    return S("SPXEX");
#endif
                }

                throw internal_not_found();

            case kdtGWIP:
                switch ((KGWIPDeviceModel)model)
                {
#if K3L_AT_LEAST(1,6,0)
                    case kdmGWIP:     return S("GWIP");
#if K3L_AT_LEAST(2,0,0)
                    case kdmGWIPEX:     return S("GWIPEX");
#endif
#else
                    case kdmGW600G:   return S("600G");
                    case kdmGW600EG:  return S("600EG");
#endif
                }

                throw internal_not_found();
#endif

#if K3L_AT_LEAST(1,6,0)
            case kdtFXS:
                switch ((KFXSDeviceModel)model)
                {
                    case kdmFXS300:   return S("300");
#if K3L_AT_LEAST(2,0,0)
                    case kdmFXS300EX:   return S("300EX");
#endif
                }

                throw internal_not_found();

            case kdtFXSSpx:
                switch ((KFXSSpxDeviceModel)model)
                {
                    case kdmFXSSpx300:       return S("SPX");
                    case kdmFXSSpx2E1Based:  return S("SPX-2E1");
#if K3L_AT_LEAST(2,0,0)
                    case kdmFXSSpx300EX:       return S("SPXEX");
#endif
                }

                throw internal_not_found();

            case kdtGSM:
                switch ((KGSMDeviceModel)model)
                {
                    case kdmGSM:       return S("40");
                }

                throw internal_not_found();

            case kdtGSMSpx:
                switch ((KGSMSpxDeviceModel)model)
                {
                    case kdmGSMSpx:    return S("SPX");
                }

                throw internal_not_found();

#endif
        }
    }
    catch (internal_not_found e)
    {
        /* this exception is used to break the control flow */
    }

    return STG(FMT("[model='%d']") % (int)model);
}

std::string Verbose::signaling(KSignaling sig)
{
    switch (sig)
    {
        case ksigInactive:      return S("ksigInactive");
        case ksigAnalog:        return S("ksigAnalog");
        case ksigContinuousEM:  return S("ksigContinuousEM");
        case ksigPulsedEM:      return S("ksigPulsedEM");
        case ksigOpenCAS:       return S("ksigOpenCAS");
        case ksigOpenR2:        return S("ksigOpenR2");
        case ksigR2Digital:     return S("ksigR2Digital");
        case ksigUserR2Digital: return S("ksigUserR2Digital");
#if K3L_AT_LEAST(1,4,0)
        case ksigSIP:           return S("ksigSIP");
#endif

#if K3L_AT_LEAST(1,5,1)
        case ksigOpenCCS:       return S("ksigOpenCCS");
        case ksigPRI_EndPoint:  return S("ksigPRI_EndPoint");
        case ksigPRI_Network:   return S("ksigPRI_Network");
        case ksigPRI_Passive:   return S("ksigPRI_Passive");
#endif
#if K3L_AT_LEAST(1,5,3)
        case ksigLineSide:        return S("ksigLineSide");
#endif
#if K3L_AT_LEAST(1,6,0)
        case ksigAnalogTerminal: return S("ksigAnalogTerminal");
        case ksigCAS_EL7:        return S("ksigCAS_EL7");
        case ksigGSM:            return S("ksigGSM");
        case ksigE1LC:           return S("ksigE1LC");
#endif
    }

    return STG(FMT("[KSignaling='%d']") % (int)sig);
}

std::string Verbose::systemObject(KSystemObject so)
{
    switch (so)
    {
        case ksoLink:     return S("ksoLink");
        case ksoLinkMon:  return S("ksoLinkMon");
        case ksoChannel:  return S("ksoChannel");
        case ksoH100:     return S("ksoH100");
        case ksoFirmware: return S("ksoFirmware");
        case ksoDevice:   return S("ksoDevice");
        case ksoAPI:      return S("ksoAPI");

        default:
            return STG(FMT("[KSystemObject='%d']") % (int)so);
    }
}

std::string Verbose::mixerTone(KMixerTone mt)
{
    switch (mt)
    {
        case kmtSilence:   return S("kmtSilence");
        case kmtDial:      return S("kmtDial");
        case kmtBusy:      return S("kmtBusy");
        case kmtFax:       return S("kmtFax");
        case kmtVoice:     return S("kmtVoice");
        case kmtEndOf425:  return S("kmtEndOf425");
#if K3L_AT_LEAST(1,5,0)
        case kmtCollect:   return S("kmtCollect");
#endif
#if K3L_AT_LEAST(1,5,1)
        case kmtEndOfDtmf: return S("kmtEndOfDtmf");
#endif
        default:
            return STG(FMT("[KMixerTone='%d']") % (int)mt);
    }
}

std::string Verbose::channelFeatures(int32 flags)
{
    if (0x00 != flags)
    {
        Strings::Merger strs;
        
        if (kcfDtmfSuppression & flags)   strs.add("DtmfSuppression");
        if (kcfCallProgress & flags)      strs.add("CallProgress");
        if (kcfPulseDetection & flags)    strs.add("PulseDetection");
        if (kcfAudioNotification & flags) strs.add("AudioNotification");
        if (kcfEchoCanceller & flags)     strs.add("EchoCanceller");
        if (kcfAutoGainControl & flags)   strs.add("AutoGainControl");
        if (kcfHighImpEvents & flags)     strs.add("HighImpEvents");
#if K3L_AT_LEAST(1,6,0)
        if (kcfCallAnswerInfo & flags)    strs.add("CallAnswerInfo");
        if (kcfOutputVolume & flags)      strs.add("OutputVolume");
        if (kcfPlayerAGC & flags)         strs.add("PlayerAGC");
#endif

        return STG(FMT("kcf{%s}") % strs.merge(","));
    };

    return S("");
}

std::string Verbose::seizeFail(KSeizeFail sf)
{
    switch (sf)
    {
        case ksfChannelLocked:   return S("ksfChannelLocked");
        case ksfChannelBusy:     return S("ksfChannelBusy");
        case ksfIncomingChannel: return S("ksfIncomingChannel");
        case ksfDoubleSeizure:   return S("ksfDoubleSeizure");
        case ksfCongestion:      return S("ksfCongestion");
        case ksfNoDialTone:      return S("ksfNoDialTone");

        default:
            return STG(FMT("[KSeizeFail='%d']") % (int)sf);
    }
}

#if K3L_AT_LEAST(1,5,0)
std::string Verbose::internal_sipFailures(KSIP_Failures code)
{
    switch (code)
    {
#if K3L_AT_LEAST(1,6,0)
        case kveResponse_200_OK_Success:                 return S("kveResponse_200_OK_Success");
#endif
        case kveRedirection_300_MultipleChoices:         return S("kveRedirection_300_MultipleChoices");
        case kveRedirection_301_MovedPermanently:        return S("kveRedirection_301_MovedPermanently");
        case kveRedirection_302_MovedTemporarily:        return S("kveRedirection_302_MovedTemporarily");
        case kveRedirection_305_UseProxy:                return S("kveRedirection_305_UseProxy");
        case kveRedirection_380_AlternativeService:      return S("kveRedirection_380_AlternativeService");
        case kveFailure_400_BadRequest:                  return S("kveFailure_400_BadRequest");
        case kveFailure_401_Unauthorized:                return S("kveFailure_401_Unauthorized");
        case kveFailure_402_PaymentRequired:             return S("kveFailure_402_PaymentRequired");
        case kveFailure_403_Forbidden:                   return S("kveFailure_403_Forbidden");
        case kveFailure_404_NotFound:                    return S("kveFailure_404_NotFound");
        case kveFailure_405_MethodNotAllowed:            return S("kveFailure_405_MethodNotAllowed");
        case kveFailure_406_NotAcceptable:               return S("kveFailure_406_NotAcceptable");
        case kveFailure_407_ProxyAuthenticationRequired: return S("kveFailure_407_ProxyAuthenticationRequired");
        case kveFailure_408_RequestTimeout:              return S("kveFailure_408_RequestTimeout");
        case kveFailure_410_Gone:                        return S("kveFailure_410_Gone");
        case kveFailure_413_RequestEntityTooLarge:       return S("kveFailure_413_RequestEntityTooLarge");
        case kveFailure_414_RequestURI_TooLong:          return S("kveFailure_414_RequestURI_TooLong");
        case kveFailure_415_UnsupportedMediaType:        return S("kveFailure_415_UnsupportedMediaType");
        case kveFailure_416_UnsupportedURI_Scheme:       return S("kveFailure_416_UnsupportedURI_Scheme");
        case kveFailure_420_BadExtension:                return S("kveFailure_420_BadExtension");
        case kveFailure_421_ExtensionRequired:           return S("kveFailure_421_ExtensionRequired");
        case kveFailure_423_IntervalTooBrief:            return S("kveFailure_423_IntervalTooBrief");
        case kveFailure_480_TemporarilyUnavailable:      return S("kveFailure_480_TemporarilyUnavailable");
        case kveFailure_481_CallDoesNotExist:            return S("kveFailure_481_CallDoesNotExist");
        case kveFailure_482_LoopDetected:                return S("kveFailure_482_LoopDetected");
        case kveFailure_483_TooManyHops:                 return S("kveFailure_483_TooManyHops");
        case kveFailure_484_AddressIncomplete:           return S("kveFailure_484_AddressIncomplete");
        case kveFailure_485_Ambiguous:                   return S("kveFailure_485_Ambiguous");
        case kveFailure_486_BusyHere:                    return S("kveFailure_486_BusyHere");
        case kveFailure_487_RequestTerminated:           return S("kveFailure_487_RequestTerminated");
        case kveFailure_488_NotAcceptableHere:           return S("kveFailure_488_NotAcceptableHere");
        case kveFailure_491_RequestPending:              return S("kveFailure_491_RequestPending");
        case kveFailure_493_Undecipherable:              return S("kveFailure_493_Undecipherable");
        case kveServer_500_InternalError:                return S("kveServer_500_InternalError");
        case kveServer_501_NotImplemented:               return S("kveServer_501_NotImplemented");
        case kveServer_502_BadGateway:                   return S("kveServer_502_BadGateway");
        case kveServer_503_ServiceUnavailable:           return S("kveServer_503_ServiceUnavailable");
        case kveServer_504_TimeOut:                      return S("kveServer_504_TimeOut");
        case kveServer_505_VersionNotSupported:          return S("kveServer_505_VersionNotSupported");
        case kveServer_513_MessageTooLarge:              return S("kveServer_513_MessageTooLarge");
        case kveGlobalFailure_600_BusyEverywhere:        return S("kveGlobalFailure_600_BusyEverywhere");
        case kveGlobalFailure_603_Decline:               return S("kveGlobalFailure_603_Decline");
        case kveGlobalFailure_604_DoesNotExistAnywhere:  return S("kveGlobalFailure_604_DoesNotExistAnywhere");
        case kveGlobalFailure_606_NotAcceptable:         return S("kveGlobalFailure_606_NotAcceptable");
    }

    throw internal_not_found();
}

std::string Verbose::sipFailures(KSIP_Failures code)
{
    try
    {
        return internal_sipFailures(code);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KSIP_Failures='%d']") % (int)code);
    }
}

#endif

#if K3L_AT_LEAST(1,5,1)
std::string Verbose::internal_isdnCause(KQ931Cause code)
{
    switch (code)
    {
        case kq931cNone:                           return S("kq931cNone");
        case kq931cUnallocatedNumber:              return S("kq931cUnallocatedNumber");
        case kq931cNoRouteToTransitNet:            return S("kq931cNoRouteToTransitNet");
        case kq931cNoRouteToDest:                  return S("kq931cNoRouteToDest");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cSendSpecialInfoTone:            return S("kq931cSendSpecialInfoTone");
        case kq931cMisdialedTrunkPrefix:           return S("kq931cMisdialedTrunkPrefix");
#endif
        case kq931cChannelUnacceptable:            return S("kq931cChannelUnacceptable");
        case kq931cCallAwarded:                    return S("kq931cCallAwarded");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cPreemption:                     return S("kq931cPreemption");
        case kq931cPreemptionCircuitReuse:         return S("kq931cPreemptionCircuitReuse");
        case kq931cQoR_PortedNumber:               return S("kq931cQoR_PortedNumber");
#endif
        case kq931cNormalCallClear:                return S("kq931cNormalCallClear");
        case kq931cUserBusy:                       return S("kq931cUserBusy");
        case kq931cNoUserResponding:               return S("kq931cNoUserResponding");
        case kq931cNoAnswerFromUser:               return S("kq931cNoAnswerFromUser");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cSubscriberAbsent:               return S("kq931cSubscriberAbsent");
#endif
        case kq931cCallRejected:                   return S("kq931cCallRejected");
        case kq931cNumberChanged:                  return S("kq931cNumberChanged");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cRedirectionToNewDest:           return S("kq931cRedirectionToNewDest");
        case kq931cCallRejectedFeatureDest:        return S("kq931cCallRejectedFeatureDest");
        case kq931cExchangeRoutingError:           return S("kq931cExchangeRoutingError");
#endif
        case kq931cNonSelectedUserClear:           return S("kq931cNonSelectedUserClear");
        case kq931cDestinationOutOfOrder:          return S("kq931cDestinationOutOfOrder");
        case kq931cInvalidNumberFormat:            return S("kq931cInvalidNumberFormat");
        case kq931cFacilityRejected:               return S("kq931cFacilityRejected");
        case kq931cRespStatusEnquiry:              return S("kq931cRespStatusEnquiry");
        case kq931cNormalUnspecified:              return S("kq931cNormalUnspecified");
        case kq931cNoCircuitChannelAvail:          return S("kq931cNoCircuitChannelAvail");
        case kq931cNetworkOutOfOrder:              return S("kq931cNetworkOutOfOrder");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cPermanentFrameConnOutOfService: return S("kq931cPermanentFrameConnOutOfService");
        case kq931cPermanentFrameConnOperational:  return S("kq931cPermanentFrameConnOperational");
#endif
        case kq931cTemporaryFailure:               return S("kq931cTemporaryFailure");
        case kq931cSwitchCongestion:               return S("kq931cSwitchCongestion");
        case kq931cAccessInfoDiscarded:            return S("kq931cAccessInfoDiscarded");
        case kq931cRequestedChannelUnav:           return S("kq931cRequestedChannelUnav");
        case kq931cPrecedenceCallBlocked:          return S("kq931cPrecedenceCallBlocked");
        case kq931cResourceUnavailable:            return S("kq931cResourceUnavailable");
        case kq931cQosUnavailable:                 return S("kq931cQosUnavailable");
        case kq931cReqFacilityNotSubsc:            return S("kq931cReqFacilityNotSubsc");
        case kq931cOutCallsBarredWithinCUG:        return S("kq931cOutCallsBarredWithinCUG");
        case kq931cInCallsBarredWithinCUG:         return S("kq931cInCallsBarredWithinCUG");
        case kq931cBearerCapabNotAuthor:           return S("kq931cBearerCapabNotAuthor");
        case kq931cBearerCapabNotAvail:            return S("kq931cBearerCapabNotAvail");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cInconsistency:                  return S("kq931cInconsistency");
#endif
        case kq931cServiceNotAvailable:            return S("kq931cServiceNotAvailable");
        case kq931cBcNotImplemented:               return S("kq931cBcNotImplemented");
        case kq931cChannelTypeNotImplem:           return S("kq931cChannelTypeNotImplem");
        case kq931cReqFacilityNotImplem:           return S("kq931cReqFacilityNotImplem");
        case kq931cOnlyRestrictedBcAvail:          return S("kq931cOnlyRestrictedBcAvail");
        case kq931cServiceNotImplemented:          return S("kq931cServiceNotImplemented");
        case kq931cInvalidCrv:                     return S("kq931cInvalidCrv");
        case kq931cChannelDoesNotExist:            return S("kq931cChannelDoesNotExist");
        case kq931cCallIdDoesNotExist:             return S("kq931cCallIdDoesNotExist");
        case kq931cCallIdInUse:                    return S("kq931cCallIdInUse");
        case kq931cNoCallSuspended:                return S("kq931cNoCallSuspended");
        case kq931cCallIdCleared:                  return S("kq931cCallIdCleared");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cUserNotMemberofCUG:             return S("kq931cUserNotMemberofCUG");
#endif
        case kq931cIncompatibleDestination:        return S("kq931cIncompatibleDestination");
        case kq931cInvalidTransitNetSel:           return S("kq931cInvalidTransitNetSel");
        case kq931cInvalidMessage:                 return S("kq931cInvalidMessage");
        case kq931cMissingMandatoryIe:             return S("kq931cMissingMandatoryIe");
        case kq931cMsgTypeNotImplemented:          return S("kq931cMsgTypeNotImplemented");
        case kq931cMsgIncompatWithState:           return S("kq931cMsgIncompatWithState");
        case kq931cIeNotImplemented:               return S("kq931cIeNotImplemented");
        case kq931cInvalidIe:                      return S("kq931cInvalidIe");
        case kq931cMsgIncompatWithState2:          return S("kq931cMsgIncompatWithState2");
        case kq931cRecoveryOnTimerExpiry:          return S("kq931cRecoveryOnTimerExpiry");
        case kq931cProtocolError:                  return S("kq931cProtocolError");
#if 1 /* this changed during K3L 1.6.0 development cycle... */
        case kq931cMessageWithUnrecognizedParam:   return S("kq931cMessageWithUnrecognizedParam");
        case kq931cProtocolErrorUnspecified:       return S("kq931cProtocolErrorUnspecified");
#endif
        case kq931cInterworking:                   return S("kq931cInterworking");
        case kq931cCallConnected:                  return S("kq931cCallConnected");
        case kq931cCallTimedOut:                   return S("kq931cCallTimedOut");
        case kq931cCallNotFound:                   return S("kq931cCallNotFound");
        case kq931cCantReleaseCall:                return S("kq931cCantReleaseCall");
        case kq931cNetworkFailure:                 return S("kq931cNetworkFailure");
        case kq931cNetworkRestart:                 return S("kq931cNetworkRestart");
    }

    throw internal_not_found();
}

std::string Verbose::isdnCause(KQ931Cause code)
{
    try
    {
        return internal_isdnCause(code);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KQ931Cause='%d']") % (int)code);
    }
}
#endif

#if K3L_AT_LEAST(1,5,2)
std::string Verbose::isdnDebug(int32 flags)
{
    if (0x00 != flags)
    {
        Strings::Merger strs;

        if (flags & kidfQ931)   strs.add("Q931");
        if (flags & kidfLAPD)   strs.add("LAPD");
        if (flags & kidfSystem) strs.add("System");
        
        return STG(FMT("kidf{%s}") % strs.merge(","));
    }
    
    return S("");
}
#endif

std::string Verbose::internal_signGroupB(KSignGroupB group)
{
    switch (group)
    {
        case kgbLineFreeCharged:     return S("kgbLineFreeCharged");
        case kgbLineFreeNotCharged:  return S("kgbLineFreeNotCharged");
        case kgbLineFreeChargedLPR:  return S("kgbLineFreeChargedLPR");
        case kgbBusy:                return S("kgbBusy");
        case kgbNumberChanged:       return S("kgbNumberChanged");
        case kgbCongestion:          return S("kgbCongestion");
        case kgbInvalidNumber:       return S("kgbInvalidNumber");
        case kgbLineOutOfOrder:      return S("kgbLineOutOfOrder");
        case kgbNone:                return S("kgbNone");
    }
    
    throw internal_not_found();
}

std::string Verbose::signGroupB(KSignGroupB group)
{
    try
    {
        return internal_signGroupB(group);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KSignGroupB='%d']") % (int)group);
    }
}

std::string Verbose::internal_signGroupII(KSignGroupII group)
{
    switch (group)
    {
        case kg2Ordinary:         return S("kg2Ordinary");
        case kg2Priority:         return S("kg2Priority");
        case kg2Maintenance:      return S("kg2Maintenance");
        case kg2LocalPayPhone:    return S("kg2LocalPayPhone");
        case kg2TrunkOperator:    return S("kg2TrunkOperator");
        case kg2DataTrans:        return S("kg2DataTrans");
        case kg2NonLocalPayPhone: return S("kg2NonLocalPayPhone");
        case kg2CollectCall:      return S("kg2CollectCall");
        case kg2OrdinaryInter:    return S("kg2OrdinaryInter");
        case kg2Transfered:       return S("kg2Transfered");
    }

    throw internal_not_found();
}

std::string Verbose::signGroupII(KSignGroupII group)
{
    try
    {
        return internal_signGroupII(group);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KSignGroupII='%d']") % (int)group);
    }
}

std::string Verbose::callFail(KSignaling sig, int32 info)
{
    try
    {
        switch (sig)
        {
            case ksigInactive:
                throw internal_not_found();

            case ksigAnalog:
                if (('a' <= ((char)info) && 'z' >= ((char)info)) || ('A' <= ((char)info) && 'Z' >= ((char)info)))
                    return STG(FMT("%c") % (char)info);
                else
                    throw internal_not_found();

#if K3L_AT_LEAST(1,5,4)
            case ksigLineSide:
#endif
#if K3L_AT_LEAST(1,6,0)
            case ksigCAS_EL7:
            case ksigE1LC:
                return S("NOT IMPLEMENTED");
                
            case ksigAnalogTerminal:
#endif
            case ksigContinuousEM:
            case ksigPulsedEM:

            case ksigOpenCAS:
            case ksigOpenR2:
            case ksigR2Digital:
            case ksigUserR2Digital:
                return internal_signGroupB((KSignGroupB)info);

#if K3L_AT_LEAST(1,5,0)
            case ksigSIP:
                return internal_sipFailures((KSIP_Failures)info);
#endif

#if K3L_AT_LEAST(1,5,1)
            case ksigOpenCCS:
            case ksigPRI_EndPoint:
            case ksigPRI_Network:
            case ksigPRI_Passive:
                return internal_isdnCause((KQ931Cause)info);
#endif

#if K3L_AT_LEAST(1,6,0)
            case ksigGSM:
                return internal_gsmCallCause((KGsmCallCause)info);
#endif
        }
    }
    catch (internal_not_found e)
    {
        /* this exception is used for breaking the control flow */
    }

    return STG(FMT("[%s, callFail='%d']") % signaling(sig) % (int)info);
}

std::string Verbose::channelFail(KSignaling sig, int32 code)
{
    try
    {
        switch (sig)
        {
            case ksigInactive:
            case ksigAnalog:
            case ksigSIP:
                throw internal_not_found();

#if K3L_AT_LEAST(1,6,0)
            case ksigGSM:
                return internal_gsmMobileCause((KGsmMobileCause)code);

            case ksigAnalogTerminal:
            case ksigCAS_EL7:
            case ksigE1LC:
#endif

            case ksigContinuousEM:
            case ksigPulsedEM:
            
            case ksigLineSide:
            
            case ksigOpenCAS:
            case ksigOpenR2:
            case ksigR2Digital:
            case ksigUserR2Digital:
                switch ((KChannelFail)code)
                {
                    case kfcRemoteFail:         return S("kfcRemoteFail");
                    case kfcLocalFail:          return S("kfcLocalFail");
                    case kfcRemoteLock:         return S("kfcRemoteLock");
                    case kfcLineSignalFail:     return S("kfcLineSignalFail");
                    case kfcAcousticSignalFail: return S("kfcAcousticSignalFail");
                }

                throw internal_not_found();

#if K3L_AT_LEAST(1,5,1)
            case ksigOpenCCS:
            case ksigPRI_EndPoint:
            case ksigPRI_Network:
            case ksigPRI_Passive:
                return internal_isdnCause((KQ931Cause)code);
#endif
        }
    }
    catch (internal_not_found e)
    {
        /* this exception is used for breaking the control flow */
    }

    return STG(FMT("[%s, channelFail='%d']") % signaling(sig) % (int)code);
}

std::string Verbose::internalFail(KInternalFail inf)
{
    switch (inf)
    {
        case kifInterruptCtrl:     return S("kifInterruptCtrl");
        case kifCommunicationFail: return S("kifCommunicationFail");
        case kifProtocolFail:      return S("kifProtocolFail");
        case kifInternalBuffer:    return S("kifInternalBuffer");
        case kifMonitorBuffer:     return S("kifMonitorBuffer");
        case kifInitialization:    return S("kifInitialization");
        case kifInterfaceFail:     return S("kifInterfaceFail");
        case kifClientCommFail:    return S("kifClientCommFail");
    }

    return STG(FMT("[KInternalFail='%d']") % (int)inf);
}

std::string Verbose::linkErrorCounter(KLinkErrorCounter ec)
{
    switch (ec)
    {
        case klecChangesToLock:     return S("klecChangesToLock");
        case klecLostOfSignal:      return S("klecLostOfSignal");
        case klecAlarmNotification: return S("klecAlarmNotification");
        case klecLostOfFrame:       return S("klecLostOfFrame");
        case klecLostOfMultiframe:  return S("klecLostOfMultiframe");
        case klecRemoteAlarm:       return S("klecRemoteAlarm");
        case klecUnknowAlarm:       return S("klecUnknowAlarm");
        case klecPRBS:              return S("klecPRBS");
           case klecWrogrBits:         return S("klecWrongBits");
        case klecJitterVariation:   return S("klecJitterVariation");
           case klecFramesWithoutSync: return S("klecFramesWithoutSync");
        case klecMultiframeSignal:  return S("klecMultiframeSignal");
           case klecFrameError:        return S("klecFrameError");
        case klecBipolarViolation:  return S("klecBipolarViolation");
           case klecCRC4:              return S("klecCRC4");
        case klecCount:             return S("klecCount");
    }

    return STG(FMT("[KLinkErrorCounter='%d']") % (int)ec);
}

std::string Verbose::callStatus(KCallStatus code)
{
    switch (code)
    {
        case kcsFree:       return S("kcsFree");
        case kcsIncoming:   return S("kcsIncoming");
        case kcsOutgoing:   return S("kcsOutgoing");
        case kcsFail:       return S("kcsFail");
    }

    return STG(FMT("[KCallStatus='%d']") % (int)code);
}

std::string Verbose::linkStatus(KSignaling sig, int32 code)
{
    switch (sig)
    {
        case ksigInactive:
            return S("[ksigInactive]");

        case ksigAnalog:
            return S("[ksigAnalog]");

#if K3L_AT_LEAST(1,4,1)
        case ksigSIP:
            return S("[ksigSIP]");
#endif

        case ksigOpenCAS:
        case ksigOpenR2:
        case ksigR2Digital:
        case ksigUserR2Digital:

#if K3L_AT_LEAST(1,5,1)
        case ksigOpenCCS:
        case ksigPRI_EndPoint:
        case ksigPRI_Network:
        case ksigPRI_Passive:
#endif
#if K3L_AT_LEAST(1,5,3)
        case ksigLineSide:
#endif
#if K3L_AT_LEAST(1,6,0)
        case ksigCAS_EL7:
        case ksigAnalogTerminal:
#endif
            if (kesOk == code)
            {
                return S("kesOk");
            }
            else
            {
                Strings::Merger strs;
                
                if (kesSignalLost & code)         strs.add("SignalLost");
                if (kesNetworkAlarm & code)       strs.add("NetworkAlarm");
                if (kesFrameSyncLost & code)      strs.add("FrameSyncLost");
                if (kesMultiframeSyncLost & code) strs.add("MultiframeSyncLost");
                if (kesRemoteAlarm & code)        strs.add("RemoteAlarm");
                if (kesHighErrorRate & code)      strs.add("HighErrorRate");
                if (kesUnknownAlarm & code)       strs.add("UnknownAlarm");
                if (kesE1Error & code)            strs.add("E1Error");

                return STG(FMT("kes{%s}") % strs.merge(","));
            }
        default:
            return STG(FMT("[%s, linkStatus='%d']") % signaling(sig) % (int)code);
    }
}

std::string Verbose::channelStatus(KSignaling sig, int32 flags)
{
    try
    {
        switch (sig)
        {
            case ksigInactive:
                return S("[ksigInactive]");

            case ksigAnalog:
#if K3L_AT_LEAST(1,6,0)
                switch ((KFXOChannelStatus)flags)
#else
                switch ((KFXChannelStatus)flags)
#endif
                {
                    case kfcsDisabled:   return S("kfcsDisabled");
                    case kfcsEnabled:    return S("kfcsEnabled");
                }

                throw internal_not_found();

#if K3L_AT_LEAST(1,6,0)
            case ksigAnalogTerminal:
                switch ((KFXSChannelStatus)flags)
                {
                    case kfxsOnHook:   return S("kfxsOnHook");
                    case kfxsOffHook:  return S("kfxsOffHook");
                    case kfxsRinging:  return S("kfxsRinging");
                    case kfxsFail:     return S("kfxsFail");
                }
                
                throw internal_not_found();

            case ksigGSM:
                switch ((KGsmChannelStatus)flags)
                {
                    case kgsmIdle:            return S("kgsmIdle");
                    case kgsmCallInProgress:  return S("kgsmCallInProgress");
                    case kgsmSMSInProgress:   return S("kgsmSMSInProgress");
                    case kgsmModemError:      return S("kgsmModemError");
                    case kgsmSIMCardError:    return S("kgsmSIMCardError");
                    case kgsmNetworkError:    return S("kgsmNetworkError");
                    case kgsmNotReady:        return S("kgsmNotReady");
                }

                throw internal_not_found();
#endif

#if K3L_AT_LEAST(1,4,1)
            case ksigSIP:
                return S("[ksigSIP]");
#endif
            /* deprecated, but still.. */
            case ksigPulsedEM:
            case ksigContinuousEM:

            case ksigOpenCAS:
            case ksigOpenR2:
            case ksigR2Digital:
            case ksigUserR2Digital:

#if K3L_AT_LEAST(1,5,1)
            case ksigOpenCCS:
            case ksigPRI_EndPoint:
            case ksigPRI_Network:
            case ksigPRI_Passive:
#endif
#if K3L_AT_LEAST(1,5,3)
            case ksigLineSide:
#endif
#if K3L_AT_LEAST(1,6,0)
            case ksigCAS_EL7:
            case ksigE1LC:
#endif
            {
                if (flags == kecsFree)
                {
                    return S("kecsFree");
                }
                else
                {
                    Strings::Merger strs;
                    
                    if (flags & kecsBusy)
                        strs.add("Busy");

                    switch (flags & 0x06)
                    {
                        case kecsOutgoing:
                            strs.add("Outgoing");
                             break;
                        case kecsIncoming:
                            strs.add("Incoming");
                            break;
                        case kecsLocked:
                            strs.add("Locked");
                        default:
                            break;
                    }

                    int32 value = (flags & 0xf0);

                    if (kecsOutgoingLock & value)
                        strs.add("OutgoingLock");

                    if (kecsLocalFail & value)
                        strs.add("LocalFail");

                    if (kecsIncomingLock & value)
                        strs.add("IncomingLock");

                    if (kecsRemoteLock & value)
                        strs.add("RemoteLock");

                    return STG(FMT("kecs{%s}") % strs.merge(","));
                }
                
                throw internal_not_found();
            }
        }
    }
    catch (internal_not_found e)
    {
        /* we use this exception to break the control flow */
    }
    
    return STG(FMT("[%s, channelStatus='%d']") % signaling(sig) % flags);
}

std::string Verbose::status(KLibraryStatus code)
{
    switch (code)
    {
        case ksSuccess:        return S("ksSuccess");
        case ksFail:           return S("ksFail");
        case ksTimeOut:        return S("ksTimeOut");
        case ksBusy:           return S("ksBusy");
        case ksLocked:         return S("ksLocked");
        case ksInvalidParams:  return S("ksInvalidParams");
        case ksEndOfFile:      return S("ksEndOfFile");
        case ksInvalidState:   return S("ksInvalidState");
        case ksServerCommFail: return S("ksServerCommFail");
        case ksOverflow:       return S("ksOverflow");
        case ksUnderrun:       return S("ksUnderrun");

#if K3L_AT_LEAST(1,4,0)
        case ksNotFound:       return S("ksNotFound");
        case ksNotAvailable:   return S("ksNotAvaiable");
#endif
    }

    return STG(FMT("[KLibraryStatus='%d']") % (int)code);
}

std::string Verbose::h100configIndex(KH100ConfigIndex code)
{
    switch (code)
    {
        case khciDeviceMode:         return S("khciDeviceMode");
        case khciMasterGenClock:     return S("khciMasterGenClock");
        case khciCTNetRefEnable:     return S("khciCTNetRefEnable");
        case khciSCbusEnable:        return S("khciSCbusEnable");
        case khciHMVipEnable:        return S("khciHMVipEnable");
        case khciMVip90Enable:       return S("khciMVip90Enable");
        case khciCTbusDataEnable:    return S("khciCTbusDataEnable");
        case khciCTbusFreq03_00:     return S("khciCTbusFreq03_00");
        case khciCTbusFreq07_04:     return S("khciCTbusFreq07_04");
        case khciCTbusFreq11_08:     return S("khciCTbusFreq11_08");
        case khciCTbusFreq15_12:     return S("khciCTbusFreq15_12");
        case khciMax:                return S("khciMax");
        case khciMasterDevId:        return S("khciMasterDevId");
        case khciSecMasterDevId:     return S("khciSecMasterDevId");
        case khciCtNetrefDevId:      return S("khciCtNetrefDevId");
#if K3L_AT_LEAST(1,6,0)
        case khciMaxH100ConfigIndex: return S("khciMaxH100ConfigIndex");
#endif
    }

    return STG(FMT("[KH100ConfigIndex='%d']") % (int)code);
}

#if K3L_AT_LEAST(1,6,0)
std::string Verbose::callStartInfo(KCallStartInfo code)
{
    switch (code)
    {
        case kcsiHumanAnswer:         return S("kcsiHumanAnswer");
        case kcsiAnsweringMachine:    return S("kcsiAnsweringMachine");
        case kcsiCellPhoneMessageBox: return S("kcsiCellPhoneMessageBox");
        case kcsiUnknown:             return S("kcsiUnknown");
        case kcsiCarrierMessage:      return S("kcsiCarrierMessage");
    }
    
    return STG(FMT("[KCallStartInfo='%d']") % (int)code);
}

std::string Verbose::gsmCallCause(KGsmCallCause code)
{
    try
    {
        return internal_gsmCallCause(code);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KGsmCallCause='%d']") % (int)code);
    }
}

std::string Verbose::internal_gsmCallCause(KGsmCallCause code)
{
    switch (code)
    {
        case kgccNone:                               return S("kgccNone");
        case kgccUnallocatedNumber:                  return S("kgccUnallocatedNumber");
        case kgccNoRouteToDest:                      return S("kgccNoRouteToDest");
        case kgccChannelUnacceptable:                return S("kgccChannelUnacceptable");
        case kgccOperatorDeterminedBarring:          return S("kgccOperatorDeterminedBarring");
        case kgccNormalCallClear:                    return S("kgccNormalCallClear");
        case kgccUserBusy:                           return S("kgccUserBusy");
        case kgccNoUserResponding:                   return S("kgccNoUserResponding");
        case kgccNoAnswerFromUser:                   return S("kgccNoAnswerFromUser");
        case kgccCallRejected:                       return S("kgccCallRejected");
        case kgccNumberChanged:                      return S("kgccNumberChanged");
        case kgccNonSelectedUserClear:               return S("kgccNonSelectedUserClear");
        case kgccDestinationOutOfOrder:              return S("kgccDestinationOutOfOrder");
        case kgccInvalidNumberFormat:                return S("kgccInvalidNumberFormat");
        case kgccFacilityRejected:                   return S("kgccFacilityRejected");
        case kgccRespStatusEnquiry:                  return S("kgccRespStatusEnquiry");
        case kgccNormalUnspecified:                  return S("kgccNormalUnspecified");
        case kgccNoCircuitChannelAvail:              return S("kgccNoCircuitChannelAvail");
        case kgccNetworkOutOfOrder:                  return S("kgccNetworkOutOfOrder");
        case kgccTemporaryFailure:                   return S("kgccTemporaryFailure");
        case kgccSwitchCongestion:                   return S("kgccSwitchCongestion");
        case kgccAccessInfoDiscarded:                return S("kgccAccessInfoDiscarded");
        case kgccRequestedChannelUnav:               return S("kgccRequestedChannelUnav");
        case kgccResourceUnavailable:                return S("kgccResourceUnavailable");
        case kgccQosUnavailable:                     return S("kgccQosUnavailable");
        case kgccReqFacilityNotSubsc:                return S("kgccReqFacilityNotSubsc");
        case kgccCallBarredWitchCUG:                 return S("kgccCallBarredWitchCUG");
        case kgccBearerCapabNotAuthor:               return S("kgccBearerCapabNotAuthor");
        case kgccBearerCapabNotAvail:                return S("kgccBearerCapabNotAvail");
        case kgccServiceNotAvailable:                return S("kgccServiceNotAvailable");
        case kgccBcNotImplemented:                   return S("kgccBcNotImplemented");
        case kgccReqFacilityNotImplem:               return S("kgccReqFacilityNotImplem");
        case kgccOnlyRestrictedBcAvail:              return S("kgccOnlyRestrictedBcAvail");
        case kgccServiceNotImplemented:              return S("kgccServiceNotImplemented");
        case kgccInvalidCrv:                         return S("kgccInvalidCrv");
        case kgccUserNotMemberOfCUG:                 return S("kgccUserNotMemberOfCUG");
        case kgccIncompatibleDestination:            return S("kgccIncompatibleDestination");
        case kgccInvalidTransitNetSel:               return S("kgccInvalidTransitNetSel");
        case kgccInvalidMessage:                     return S("kgccInvalidMessage");
        case kgccMissingMandatoryIe:                 return S("kgccMissingMandatoryIe");
        case kgccMsgTypeNotImplemented:              return S("kgccMsgTypeNotImplemented");
        case kgccMsgIncompatWithState:               return S("kgccMsgIncompatWithState");
        case kgccIeNotImplemented:                   return S("kgccIeNotImplemented");
        case kgccInvalidIe:                          return S("kgccInvalidIe");
        case kgccMsgIncompatWithState2:              return S("kgccMsgIncompatWithState2");
        case kgccRecoveryOnTimerExpiry:              return S("kgccRecoveryOnTimerExpiry");
        case kgccProtocolError:                      return S("kgccProtocolError");
        case kgccInterworking:                       return S("kgccInterworking");
#if 0 /* this changed during K3L 1.6.0 development cycle... */
        case kgccPhoneFailure:                       return S("kgccPhoneFailure");
        case kgccSmsServiceReserved:                 return S("kgccSmsServiceReserved");
        case kgccOperationNotAllowed:                return S("kgccOperationNotAllowed");
        case kgccOperationNotSupported:              return S("kgccOperationNotSupported");
        case kgccInvalidPDUModeParameter:            return S("kgccInvalidPDUModeParameter");
        case kgccInvalidTextModeParameter:           return S("kgccInvalidTextModeParameter");
        case kgccSIMNotInserted:                     return S("kgccSIMNotInserted");
        case kgccSIMPINNecessary:                    return S("kgccSIMPINNecessary");
        case kgccPH_SIMPINNecessary:                 return S("kgccPH_SIMPINNecessary");
        case kgccSIMFailure:                         return S("kgccSIMFailure");
        case kgccSIMBusy:                            return S("kgccSIMBusy");
        case kgccSIMWrong:                           return S("kgccSIMWrong");
        case kgccMemoryFailure:                      return S("kgccMemoryFailure");
        case kgccInvalidMemoryIndex:                 return S("kgccInvalidMemoryIndex");
        case kgccMemoryFull:                         return S("kgccMemoryFull");
        case kgccSMSCAddressUnknown:                 return S("kgccSMSCAddressUnknown");
        case kgccNoNetworkService:                   return S("kgccNoNetworkService");
        case kgccNetworkTimeout:                     return S("kgccNetworkTimeout");
        case kgccUnknownError:                       return S("kgccUnknownError");
#endif
    }

    throw internal_not_found();
}

std::string Verbose::gsmMobileCause(KGsmMobileCause code)
{
    try
    {
        return internal_gsmMobileCause(code);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KGsmMobileCause='%d']") % (int)code);
    }
}

std::string Verbose::internal_gsmMobileCause(KGsmMobileCause code)
{
    switch (code)
    {
        case kgmcPhoneFailure:                    return S("kgmcPhoneFailure");
        case kgmcNoConnectionToPhone:             return S("kgmcNoConnectionToPhone");
        case kgmcPhoneAdaptorLinkReserved:        return S("kgmcPhoneAdaptorLinkReserved");
#if 0 /* this changed during K3L 1.6.0 development cycle... */
        case kgmcCoperationNotAllowed:            return S("kgmcCoperationNotAllowed");
        case kgmcCoperationNotSupported:          return S("kgmcCoperationNotSupported");
#else
        case kgmcOperationNotAllowed:             return S("kgmcOperationNotAllowed");
        case kgmcOperationNotSupported:           return S("kgmcOperationNotSupported");
#endif
        case kgmcPH_SIMPINRequired:               return S("kgmcPH_SIMPINRequired");
        case kgmcPH_FSIMPINRequired:              return S("kgmcPH_FSIMPINRequired");
        case kgmcPH_FSIMPUKRequired:              return S("kgmcPH_FSIMPUKRequired");
        case kgmcSIMNotInserted:                  return S("kgmcSIMNotInserted");
        case kgmcSIMPINRequired:                  return S("kgmcSIMPINRequired");
        case kgmcSIMPUKRequired:                  return S("kgmcSIMPUKRequired");
        case kgmcSIMFailure:                      return S("kgmcSIMFailure");
        case kgmcSIMBusy:                         return S("kgmcSIMBusy");
        case kgmcSIMWrong:                        return S("kgmcSIMWrong");
        case kgmcIncorrectPassword:               return S("kgmcIncorrectPassword");
        case kgmcSIMPIN2Required:                 return S("kgmcSIMPIN2Required");
        case kgmcSIMPUK2Required:                 return S("kgmcSIMPUK2Required");
        case kgmcMemoryFull:                      return S("kgmcMemoryFull");
        case kgmcInvalidIndex:                    return S("kgmcInvalidIndex");
        case kgmcNotFound:                        return S("kgmcNotFound");
        case kgmcMemoryFailure:                   return S("kgmcMemoryFailure");
        case kgmcTextStringTooLong:               return S("kgmcTextStringTooLong");
        case kgmcInvalidCharInTextString:         return S("kgmcInvalidCharInTextString");
        case kgmcDialStringTooLong:               return S("kgmcDialStringTooLong");
        case kgmcInvalidCharInDialString:         return S("kgmcInvalidCharInDialString");
        case kgmcNoNetworkService:                return S("kgmcNoNetworkService");
        case kgmcNetworkTimeout:                  return S("kgmcNetworkTimeout");
        case kgmcNetworkNotAllowed:               return S("kgmcNetworkNotAllowed");
        case kgmcCommandAborted:                  return S("kgmcCommandAborted");
        case kgmcNumParamInsteadTextParam:        return S("kgmcNumParamInsteadTextParam");
        case kgmcTextParamInsteadNumParam:        return S("kgmcTextParamInsteadNumParam");
        case kgmcNumericParamOutOfBounds:         return S("kgmcNumericParamOutOfBounds");
        case kgmcTextStringTooShort:              return S("kgmcTextStringTooShort");
        case kgmcNetworkPINRequired:              return S("kgmcNetworkPINRequired");
        case kgmcNetworkPUKRequired:              return S("kgmcNetworkPUKRequired");
        case kgmcNetworkSubsetPINRequired:        return S("kgmcNetworkSubsetPINRequired");
        case kgmcNetworkSubnetPUKRequired:        return S("kgmcNetworkSubnetPUKRequired");
        case kgmcServiceProviderPINRequired:      return S("kgmcServiceProviderPINRequired");
        case kgmcServiceProviderPUKRequired:      return S("kgmcServiceProviderPUKRequired");
        case kgmcCorporatePINRequired:            return S("kgmcCorporatePINRequired");
        case kgmcCorporatePUKRequired:            return S("kgmcCorporatePUKRequired");
        case kgmcSIMServiceOptNotSupported:       return S("kgmcSIMServiceOptNotSupported");
        case kgmcUnknown:                         return S("kgmcUnknown");
        case kgmcIllegalMS_N3:                    return S("kgmcIllegalMS_N3");
        case kgmcIllegalME_N6:                    return S("kgmcIllegalME_N6");
        case kgmcGPRSServicesNotAllowed_N7:       return S("kgmcGPRSServicesNotAllowed_N7");
        case kgmcPLMNNotAllowed_No11:             return S("kgmcPLMNNotAllowed_No11");
        case kgmcLocationAreaNotAllowed_N12:      return S("kgmcLocationAreaNotAllowed_N12");
        case kgmcRoamingNotAllowed_N13:           return S("kgmcRoamingNotAllowed_N13");
        case kgmcServiceOptNotSupported_N32:      return S("kgmcServiceOptNotSupported_N32");
        case kgmcReqServOptNotSubscribed_N33:     return S("kgmcReqServOptNotSubscribed_N33");
        case kgmcServOptTempOutOfOrder_N34:       return S("kgmcServOptTempOutOfOrder_N34");
        case kgmcLongContextActivation:           return S("kgmcLongContextActivation");
        case kgmcUnspecifiedGPRSError:            return S("kgmcUnspecifiedGPRSError");
        case kgmcPDPAuthenticationFailure:        return S("kgmcPDPAuthenticationFailure");
        case kgmcInvalidMobileClass:              return S("kgmcInvalidMobileClass");
        case kgmcGPRSDisconnectionTmrActive:      return S("kgmcGPRSDisconnectionTmrActive");
        case kgmcTooManyActiveCalls:              return S("kgmcTooManyActiveCalls");
        case kgmcCallRejected:                    return S("kgmcCallRejected");
        case kgmcUnansweredCallPending:           return S("kgmcUnansweredCallPending");
        case kgmcUnknownCallingError:             return S("kgmcUnknownCallingError");
        case kgmcNoPhoneNumRecognized:            return S("kgmcNoPhoneNumRecognized");
        case kgmcCallStateNotIdle:                return S("kgmcCallStateNotIdle");
        case kgmcCallInProgress:                  return S("kgmcCallInProgress");
        case kgmcDialStateError:                  return S("kgmcDialStateError");
        case kgmcUnlockCodeRequired:              return S("kgmcUnlockCodeRequired");
        case kgmcNetworkBusy:                     return S("kgmcNetworkBusy");
        case kgmcInvalidPhoneNumber:              return S("kgmcInvalidPhoneNumber");
        case kgmcNumberEntryAlreadyStarted:       return S("kgmcNumberEntryAlreadyStarted");
        case kgmcCancelledByUser:                 return S("kgmcCancelledByUser");
        case kgmcNumEntryCouldNotBeStarted:       return S("kgmcNumEntryCouldNotBeStarted");
        case kgmcDataLost:                        return S("kgmcDataLost");
        case kgmcInvalidBessageBodyLength:        return S("kgmcInvalidBessageBodyLength");
        case kgmcInactiveSocket:                  return S("kgmcInactiveSocket");
        case kgmcSocketAlreadyOpen:               return S("kgmcSocketAlreadyOpen");
    }
    
    throw internal_not_found();
}

std::string Verbose::gsmSmsCause(KGsmSmsCause code)
{
    try
    {
        return internal_gsmSmsCause(code);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KGsmSmsCause='%d']") % (int)code);
    }
}

std::string Verbose::internal_gsmSmsCause(KGsmSmsCause code)
{
    switch (code)
    {
        case kgscUnassigned:                     return S("kgscUnassigned");
        case kgscOperatorDeterminedBarring:      return S("kgscOperatorDeterminedBarring");
        case kgscCallBarred:                     return S("kgscCallBarred");
        case kgscSMSTransferRejected:            return S("kgscSMSTransferRejected");
        case kgscDestinationOutOfService:        return S("kgscDestinationOutOfService");
        case kgscUnidentifiedSubscriber:         return S("kgscUnidentifiedSubscriber");
        case kgscFacilityRejected:               return S("kgscFacilityRejected");
        case kgscUnknownSubscriber:              return S("kgscUnknownSubscriber");
        case kgscNetworkOutOfOrder:              return S("kgscNetworkOutOfOrder");
        case kgscTemporaryFailure:               return S("kgscTemporaryFailure");
        case kgscCongestion:                     return S("kgscCongestion");
        case kgscResourcesUnavailable:           return S("kgscResourcesUnavailable");
        case kgscFacilityNotSubscribed:          return S("kgscFacilityNotSubscribed");
        case kgscFacilityNotImplemented:         return S("kgscFacilityNotImplemented");
        case kgscInvalidSMSTransferRefValue:     return S("kgscInvalidSMSTransferRefValue");
        case kgscInvalidMessage:                 return S("kgscInvalidMessage");
        case kgscInvalidMandatoryInformation:    return S("kgscInvalidMandatoryInformation");
        case kgscMessageTypeNonExistent:         return S("kgscMessageTypeNonExistent");
        case kgscMsgNotCompatWithSMProtState:    return S("kgscMsgNotCompatWithSMProtState");
        case kgscInformationElementNonExiste:    return S("kgscInformationElementNonExiste");
        case kgscProtocolError:                  return S("kgscProtocolError");
        case kgscInterworking:                   return S("kgscInterworking");
        case kgscTelematicInterworkingNotSup:    return S("kgscTelematicInterworkingNotSup");
        case kgscSMSTypeZeroNotSupported:        return S("kgscSMSTypeZeroNotSupported");
        case kgscCannotReplaceSMS:               return S("kgscCannotReplaceSMS");
        case kgscUnspecifiedTPPIDError:          return S("kgscUnspecifiedTPPIDError");
        case kgscAlphabetNotSupported:           return S("kgscAlphabetNotSupported");
        case kgscMessageClassNotSupported:       return S("kgscMessageClassNotSupported");
        case kgscUnspecifiedTPDCSError:          return S("kgscUnspecifiedTPDCSError");
        case kgscCommandCannotBeActioned:        return S("kgscCommandCannotBeActioned");
        case kgscCommandUnsupported:             return S("kgscCommandUnsupported");
        case kgscUnspecifiedTPCommandError:      return S("kgscUnspecifiedTPCommandError");
        case kgscTPDUNotSupported:               return S("kgscTPDUNotSupported");
        case kgscSCBusy:                         return S("kgscSCBusy");
        case kgscNoSCSubscription:               return S("kgscNoSCSubscription");
        case kgscSCSystemFailure:                return S("kgscSCSystemFailure");
        case kgscInvalidSMEAddress:              return S("kgscInvalidSMEAddress");
        case kgscDestinationSMEBarred:           return S("kgscDestinationSMEBarred");
        case kgscSMRejectedDuplicateSM:          return S("kgscSMRejectedDuplicateSM");
        case kgscTPVPFNotSupported:              return S("kgscTPVPFNotSupported");
        case kgscTPVPNotSupported:               return S("kgscTPVPNotSupported");
        case kgscSIMSMSStorageFull:              return S("kgscSIMSMSStorageFull");
        case kgscNoSMSStorageCapabilityInSIM:    return S("kgscNoSMSStorageCapabilityInSIM");
        case kgscErrorInMS:                      return S("kgscErrorInMS");
        case kgscMemoryCapacityExceeded:         return S("kgscMemoryCapacityExceeded");
        case kgscSIMDataDownloadError:           return S("kgscSIMDataDownloadError");
        case kgscUnspecifiedError:               return S("kgscUnspecifiedError");
        case kgscPhoneFailure:                   return S("kgscPhoneFailure");
        case kgscSmsServiceReserved:             return S("kgscSmsServiceReserved");
        case kgscOperationNotAllowed:            return S("kgscOperationNotAllowed");
        case kgscOperationNotSupported:          return S("kgscOperationNotSupported");
        case kgscInvalidPDUModeParameter:        return S("kgscInvalidPDUModeParameter");
        case kgscInvalidTextModeParameter:       return S("kgscInvalidTextModeParameter");
        case kgscSIMNotInserted:                 return S("kgscSIMNotInserted");
        case kgscSIMPINNecessary:                return S("kgscSIMPINNecessary");
        case kgscPH_SIMPINNecessary:             return S("kgscPH_SIMPINNecessary");
        case kgscSIMFailure:                     return S("kgscSIMFailure");
        case kgscSIMBusy:                        return S("kgscSIMBusy");
        case kgscSIMWrong:                       return S("kgscSIMWrong");
        case kgscMemoryFailure:                  return S("kgscMemoryFailure");
        case kgscInvalidMemoryIndex:             return S("kgscInvalidMemoryIndex");
        case kgscMemoryFull:                     return S("kgscMemoryFull");
        case kgscSMSCAddressUnknown:             return S("kgscSMSCAddressUnknown");
        case kgscNoNetworkService:               return S("kgscNoNetworkService");
        case kgscNetworkTimeout:                 return S("kgscNetworkTimeout");
        case kgscUnknownError:                   return S("kgscUnknownError");
        case kgscNetworkBusy:                    return S("kgscNetworkBusy");
        case kgscInvalidDestinationAddress:      return S("kgscInvalidDestinationAddress");
        case kgscInvalidMessageBodyLength:       return S("kgscInvalidMessageBodyLength");
        case kgscPhoneIsNotInService:            return S("kgscPhoneIsNotInService");
        case kgscInvalidPreferredMemStorage:     return S("kgscInvalidPreferredMemStorage");
        case kgscUserTerminated:                 return S("kgscUserTerminated");
    }
    
    throw internal_not_found();
}

std::string Verbose::q931ProgressIndication(KQ931ProgressIndication code)
{
    try
    {
        return internal_q931ProgressIndication(code);
    }
    catch (internal_not_found e)
    {
        return STG(FMT("[KQ931ProgressIndication='%d']") % (int)code);
    }
}

std::string Verbose::internal_q931ProgressIndication(KQ931ProgressIndication code)
{
    switch (code)
    {
        case kq931pTonesMaybeAvailable:     return S("kq931pTonesMaybeAvailable");
        case kq931pDestinationIsNonIsdn:    return S("kq931pDestinationIsNonIsdn");
        case kq931pOriginationIsNonIsdn:    return S("kq931pOriginationIsNonIsdn");
        case kq931pCallReturnedToIsdn:      return S("kq931pCallReturnedToIsdn");
        case kq931pTonesAvailable:          return S("kq931pTonesAvailable");
    }

    throw internal_not_found();
}

#endif /* K3L_AT_LEAST(1,6,0) */

/********/

std::string Verbose::commandName(int32 code)
{
    switch ((kcommand)code)
    {
        case K_CM_SEIZE:                    return S("CM_SEIZE");
        case K_CM_SYNC_SEIZE:               return S("CM_SYNC_SEIZE");
        case K_CM_DIAL_DTMF:                return S("CM_DIAL_DTMF");
#if K3L_AT_LEAST(1,6,0)
        case K_CM_SIP_REGISTER:             return S("CM_SIP_REGISTER");
#endif
        case K_CM_DISCONNECT:               return S("CM_DISCONNECT");
        case K_CM_CONNECT:                  return S("CM_CONNECT");
        case K_CM_PRE_CONNECT:              return S("CM_PRE_CONNECT");
        case K_CM_CAS_CHANGE_LINE_STT:      return S("CM_CAS_CHANGE_LINE_STT");
        case K_CM_CAS_SEND_MFC:             return S("CM_CAS_SEND_MFC");
        case K_CM_SET_FORWARD_CHANNEL:      return S("CM_SET_FORWARD_CHANNEL");
        case K_CM_CAS_SET_MFC_DETECT_MODE:  return S("CM_CAS_SET_MFC_DETECT_MODE");
        case K_CM_DROP_COLLECT_CALL:        return S("CM_DROP_COLLECT_CALL");

#if K3L_AT_LEAST(1,5,0)
        case K_CM_MAKE_CALL:                return S("CM_MAKE_CALL");
#endif

#if K3L_AT_LEAST(1,4,0)
        case K_CM_RINGBACK:                 return S("CM_RINGBACK");
#endif

#if K3L_AT_LEAST(1,5,1)
        case K_CM_USER_INFORMATION:         return S("CM_USER_INFORMATION");
#endif

#if K3L_AT_LEAST(1,4,0)
        case K_CM_VOIP_SEIZE:               return S("CM_VOIP_SEIZE");

#if !K3L_AT_LEAST(2,0,0) 
        /* internal commands */
        case K_CM_VOIP_START_DEBUG:         return S("CM_VOIP_START_DEBUG");
        case K_CM_VOIP_STOP_DEBUG:          return S("CM_VOIP_STOP_DEBUG");
        case K_CM_VOIP_DUMP_STAT:           return S("CM_VOIP_DUMP_STAT");
#endif
#endif

#if K3L_AT_LEAST(1,5,2) && !K3L_AT_LEAST(2,0,0) 
        /* internal command */
        case K_CM_ISDN_DEBUG:               return S("CM_ISDN_DEBUG");
#endif

        case K_CM_LOCK_INCOMING:            return S("CM_LOCK_INCOMING");
        case K_CM_UNLOCK_INCOMING:          return S("CM_UNLOCK_INCOMING");
        case K_CM_LOCK_OUTGOING:            return S("CM_LOCK_OUTGOING");
        case K_CM_UNLOCK_OUTGOING:          return S("CM_UNLOCK_OUTGOING");

        case K_CM_START_SEND_FAIL:          return S("CM_START_SEND_FAIL");
        case K_CM_STOP_SEND_FAIL:           return S("CM_STOP_SEND_FAIL");

#if K3L_AT_LEAST(1,5,3)
        case K_CM_END_OF_NUMBER:            return S("CM_END_OF_NUMBER");
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_CM_SS_TRANSFER:              return S("CM_SS_TRANSFER");
        case K_CM_GET_SMS:                  return S("CM_GET_SMS");
        case K_CM_PREPARE_SMS:              return S("CM_PREPARE_SMS");
        case K_CM_SEND_SMS:                 return S("CM_SEND_SMS");
#endif
#if K3L_HAS_MPTY_SUPPORT
        case K_CM_HOLD_SWITCH:              return S("CM_HOLD_SWITCH");
        case K_CM_MPTY_CONF:                return S("CM_MPTY_CONF");
        case K_CM_MPTY_SPLIT:               return S("CM_MPTY_SPLIT");
#endif

        case K_CM_ENABLE_DTMF_SUPPRESSION:  return S("CM_ENABLE_DTMF_SUPPRESSION");
        case K_CM_DISABLE_DTMF_SUPPRESSION: return S("CM_DISABLE_DTMF_SUPPRESSION");
        case K_CM_ENABLE_AUDIO_EVENTS:      return S("CM_ENABLE_AUDIO_EVENTS");
        case K_CM_DISABLE_AUDIO_EVENTS:     return S("CM_DISABLE_AUDIO_EVENTS");
        case K_CM_ENABLE_CALL_PROGRESS:     return S("CM_ENABLE_CALL_PROGRESS");
        case K_CM_DISABLE_CALL_PROGRESS:    return S("CM_DISABLE_CALL_PROGRESS");
        case K_CM_FLASH:                    return S("CM_FLASH");
        case K_CM_ENABLE_PULSE_DETECTION:   return S("CM_ENABLE_PULSE_DETECTION");
        case K_CM_DISABLE_PULSE_DETECTION:  return S("CM_DISABLE_PULSE_DETECTION");
        case K_CM_ENABLE_ECHO_CANCELLER:    return S("CM_ENABLE_ECHO_CANCELLER");
        case K_CM_DISABLE_ECHO_CANCELLER:   return S("CM_DISABLE_ECHO_CANCELLER");
        case K_CM_ENABLE_AGC:               return S("CM_ENABLE_AGC");
        case K_CM_DISABLE_AGC:              return S("CM_DISABLE_AGC");
        case K_CM_ENABLE_HIGH_IMP_EVENTS:   return S("CM_ENABLE_HIGH_IMP_EVENTS");
        case K_CM_DISABLE_HIGH_IMP_EVENTS:  return S("CM_DISABLE_HIGH_IMP_EVENTS");

#if K3L_AT_LEAST(1,6,0)
        case K_CM_ENABLE_CALL_ANSWER_INFO:  return S("CM_ENABLE_CALL_ANSWER_INFO");
        case K_CM_DISABLE_CALL_ANSWER_INFO: return S("CM_DISABLE_CALL_ANSWER_INFO"); 
#endif

        case K_CM_RESET_LINK:               return S("CM_RESET_LINK");

#if K3L_AT_LEAST(1,6,0)
        case K_CM_CLEAR_LINK_ERROR_COUNTER: return S("CM_CLEAR_LINK_ERROR_COUNTER");
#endif

        case K_CM_SEND_DTMF:                return S("CM_SEND_DTMF");
        case K_CM_STOP_AUDIO:               return S("CM_STOP_AUDIO");
        case K_CM_HARD_RESET:               return S("CM_HARD_RESET");

        case K_CM_SEND_TO_CTBUS:            return S("CM_SEND_TO_CTBUS");
        case K_CM_RECV_FROM_CTBUS:          return S("CM_RECV_FROM_CTBUS");
        case K_CM_SETUP_H100:               return S("CM_SETUP_H100");

        case K_CM_MIXER:                    return S("CM_MIXER");
        case K_CM_CLEAR_MIXER:              return S("CM_CLEAR_MIXER");
        case K_CM_PLAY_FROM_FILE:           return S("CM_PLAY_FROM_FILE");
        case K_CM_RECORD_TO_FILE:           return S("CM_RECORD_TO_FILE");
        case K_CM_PLAY_FROM_STREAM:         return S("CM_PLAY_FROM_STREAM");
        case K_CM_STOP_PLAY:                return S("CM_STOP_PLAY");
        case K_CM_STOP_RECORD:              return S("CM_STOP_RECORD");
        case K_CM_PAUSE_PLAY:               return S("CM_PAUSE_PLAY");
        case K_CM_PAUSE_RECORD:             return S("CM_PAUSE_RECORD");
        case K_CM_INCREASE_VOLUME:          return S("CM_INCREASE_VOLUME");
        case K_CM_DECREASE_VOLUME:          return S("CM_DECREASE_VOLUME");
        case K_CM_LISTEN:                   return S("CM_LISTEN");
        case K_CM_STOP_LISTEN:              return S("CM_STOP_LISTEN");
        case K_CM_PREPARE_FOR_LISTEN:       return S("CM_PREPARE_FOR_LISTEN");

        case K_CM_PLAY_SOUND_CARD:          return S("CM_PLAY_SOUND_CARD");
        case K_CM_STOP_SOUND_CARD:          return S("CM_STOP_SOUND_CARD");

        case K_CM_MIXER_CTBUS:              return S("CM_MIXER_CTBUS");
        case K_CM_PLAY_FROM_STREAM_EX:      return S("CM_PLAY_FROM_STREAM_EX");
        case K_CM_ENABLE_PLAYER_AGC:        return S("CM_ENABLE_PLAYER_AGC");
        case K_CM_DISABLE_PLAYER_AGC:       return S("CM_DISABLE_PLAYER_AGC");
        case K_CM_START_STREAM_BUFFER:      return S("CM_START_STREAM_BUFFER");
        case K_CM_ADD_STREAM_BUFFER:        return S("CM_ADD_STREAM_BUFFER");
        case K_CM_STOP_STREAM_BUFFER:       return S("CM_STOP_STREAM_BUFFER");
        case K_CM_SEND_BEEP:                return S("CM_SEND_BEEP");
        case K_CM_SEND_BEEP_CONF:           return S("CM_SEND_BEEP_CONF");
        case K_CM_ADD_TO_CONF:              return S("CM_ADD_TO_CONF");
        case K_CM_REMOVE_FROM_CONF:         return S("CM_REMOVE_FROM_CONF");
        case K_CM_RECORD_TO_FILE_EX:        return S("CM_RECORD_TO_FILE_EX");

#if K3L_AT_LEAST(1,5,4)
        case K_CM_SET_VOLUME:               return S("CM_SET_VOLUME");
#endif
        case K_CM_SET_LINE_CONDITION:       return S("CM_SET_LINE_CONDITION");
        case K_CM_SEND_LINE_CONDITION:      return S("CM_SEND_LINE_CONDITION");
        case K_CM_SET_CALLER_CATEGORY:      return S("CM_SET_CALLER_CATEGORY");
        case K_CM_DIAL_MFC:                 return S("CM_DIAL_MFC");

        case K_CM_INTERNAL_PLAY:            return S("CM_INTERNAL_PLAY");
        case K_CM_RESUME_PLAY:              return S("CM_RESUME_PLAY");
        case K_CM_RESUME_RECORD:            return S("CM_RESUME_RECORD");
        case K_CM_INTERNAL_PLAY_EX:         return S("CM_INTERNAL_PLAY_EX");
#if !K3L_AT_LEAST(2,0,0)
        case K_CM_PING:                     return S("CM_PING");
#if K3L_AT_LEAST(1,6,0)
        case K_CM_LOG_REQUEST:              return S("CM_LOG_REQUEST");
        case K_CM_LOG_CREATE_DISPATCHER:    return S("CM_LOG_CREATE_DISPATCHER");
        case K_CM_LOG_DESTROY_DISPATCHER:   return S("CM_LOG_DESTROY_DISPATCHER");
#endif
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_CM_START_CADENCE:            return S("CM_START_CADENCE");
        case K_CM_STOP_CADENCE:             return S("CM_STOP_CADENCE");
        case K_CM_CHECK_NEW_SMS:            return S("CM_CHECK_NEW_SMS");
#endif
    }

    return STG(FMT("[command='%d']") % code);
}

std::string Verbose::eventName(int32 code)
{
    switch ((kevent)code)
    {
        case K_EV_CHANNEL_FREE:         return S("EV_CHANNEL_FREE");
        case K_EV_CONNECT:              return S("EV_CONNECT");
        case K_EV_DISCONNECT:           return S("EV_DISCONNECT");
        case K_EV_CALL_SUCCESS:         return S("EV_CALL_SUCCESS");
        case K_EV_CALL_FAIL:            return S("EV_CALL_FAIL");
        case K_EV_NO_ANSWER:            return S("EV_NO_ANSWER");
        case K_EV_BILLING_PULSE:        return S("EV_BILLING_PULSE");
        case K_EV_SEIZE_SUCCESS:        return S("EV_SEIZE_SUCCESS");
        case K_EV_SEIZE_FAIL:           return S("EV_SEIZE_FAIL");
        case K_EV_SEIZURE_START:        return S("EV_SEIZURE_START");
        case K_EV_CAS_LINE_STT_CHANGED: return S("EV_CAS_LINE_STT_CHANGED");
        case K_EV_CAS_MFC_RECV:         return S("EV_CAS_MFC_RECV");

#if K3L_AT_LEAST(1,5,0)
        case K_EV_NEW_CALL:             return S("EV_NEW_CALL");
#endif

#if K3L_AT_LEAST(1,5,1)
        case K_EV_USER_INFORMATION:     return S("EV_USER_INFORMATION");
#endif

#if K3L_AT_LEAST(1,5,3)
        case K_EV_DIALED_DIGIT:         return S("EV_DIALED_DIGIT");
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_EV_SIP_REGISTER_INFO:    return S("EV_SIP_REGISTER_INFO");
#endif

#if K3L_AT_LEAST(1,4,0)
        case K_EV_CALL_HOLD_START:      return S("EV_CALL_HOLD_START");
        case K_EV_CALL_HOLD_STOP:       return S("EV_CALL_HOLD_STOP");
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_EV_SS_TRANSFER_FAIL:     return S("EV_SS_TRANSFER_FAIL");
        case K_EV_FLASH:                return S("EV_FLASH");
#endif

        case K_EV_DTMF_DETECTED:        return S("EV_DTMF_DETECTED");
        case K_EV_DTMF_SEND_FINISH:     return S("EV_DTMF_SEND_FINISH");
        case K_EV_AUDIO_STATUS:         return S("EV_AUDIO_STATUS");
        case K_EV_CADENCE_RECOGNIZED:   return S("EV_CADENCE_RECOGNIZED");

        case K_EV_END_OF_STREAM:        return S("EV_END_OF_STREAM");
        case K_EV_PULSE_DETECTED:       return S("EV_PULSE_DETECTED");

#if K3L_AT_LEAST(1,5,1)
        case K_EV_POLARITY_REVERSAL:    return S("EV_POLARITY_REVERSAL");
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_EV_ISDN_PROGRESS_INDICATOR: return S("EV_ISDN_PROGRESS_INDICATOR");
        case K_EV_CALL_ANSWER_INFO:        return S("EV_CALL_ANSWER_INFO");
        case K_EV_COLLECT_CALL:            return S("EV_COLLECT_CALL");
        case K_EV_SIP_DTMF_DETECTED:       return S("EV_SIP_DTMF_DETECTED");

        case K_EV_RECV_FROM_MODEM:      return S("EV_RECV_FROM_MODEM");
        case K_EV_NEW_SMS:              return S("EV_NEW_SMS");
        case K_EV_SMS_INFO:             return S("EV_SMS_INFO");
        case K_EV_SMS_DATA:             return S("EV_SMS_DATA");
        case K_EV_SMS_SEND_RESULT:      return S("EV_SMS_SEND_RESULT");
        case K_EV_RING_DETECTED:        return S("EV_RING_DETECTED");
        case K_EV_PHYSICAL_LINK_DOWN:   return S("EV_PHYSICAL_LINK_DOWN");
        case K_EV_PHYSICAL_LINK_UP:     return S("EV_PHYSICAL_LINK_UP");
#endif
#if K3L_HAS_MPTY_SUPPORT
        case K_EV_CALL_MPTY_START:      return S("EV_CALL_MPTY_START");
        case K_EV_CALL_MPTY_STOP:       return S("EV_CALL_MPTY_STOP");
#endif
        case K_EV_PONG:                 return S("EV_PONG");
        case K_EV_CHANNEL_FAIL:         return S("EV_CHANNEL_FAIL");
        case K_EV_REFERENCE_FAIL:       return S("EV_REFERENCE_FAIL");
        case K_EV_INTERNAL_FAIL:        return S("EV_INTERNAL_FAIL");
        case K_EV_HARDWARE_FAIL:        return S("EV_HARDWARE_FAIL");
        case K_EV_LINK_STATUS:          return S("EV_LINK_STATUS");

#if K3L_AT_LEAST(1,4,0)
        case K_EV_CLIENT_RECONNECT:     return S("EV_CLIENT_RECONNECT");
        case K_EV_VOIP_SEIZURE:         return S("EV_VOIP_SEIZURE");
#endif
        case K_EV_SEIZURE:              return S("EV_SEIZURE");
    }
    
    return STG(FMT("[event='%d']") % code);
}


std::string Verbose::command(int32 dev, K3L_COMMAND *k3lcmd)
{
    return command(k3lcmd->Cmd, dev, k3lcmd->Object, (const char *) k3lcmd->Params);
}

std::string Verbose::command(int32 cmd_code, int32 dev_idx, int32 obj_idx, const char * params)
{
    ushort dev = (ushort) dev_idx;
    ushort obj = (ushort) obj_idx;

    kcommand code = (kcommand) cmd_code;

    std::string buf, extra;
    
    switch (code)
    {
        case K_CM_SEIZE:
        case K_CM_SYNC_SEIZE:
        case K_CM_VOIP_SEIZE:
        case K_CM_DIAL_MFC:
        case K_CM_DIAL_DTMF:

        case K_CM_CONNECT:
        case K_CM_PRE_CONNECT:
        case K_CM_DISCONNECT:
        case K_CM_DROP_COLLECT_CALL:

        case K_CM_START_SEND_FAIL:
        case K_CM_STOP_SEND_FAIL:

        case K_CM_ENABLE_DTMF_SUPPRESSION:
        case K_CM_DISABLE_DTMF_SUPPRESSION:
        case K_CM_ENABLE_AUDIO_EVENTS:
        case K_CM_DISABLE_AUDIO_EVENTS:
        case K_CM_ENABLE_CALL_PROGRESS:
        case K_CM_DISABLE_CALL_PROGRESS:
        case K_CM_ENABLE_PULSE_DETECTION:
        case K_CM_DISABLE_PULSE_DETECTION:
        case K_CM_ENABLE_ECHO_CANCELLER:
        case K_CM_DISABLE_ECHO_CANCELLER:
        case K_CM_ENABLE_AGC:
        case K_CM_DISABLE_AGC:
        case K_CM_ENABLE_HIGH_IMP_EVENTS:
        case K_CM_DISABLE_HIGH_IMP_EVENTS:

        case K_CM_FLASH:
        case K_CM_RESET_LINK:
        case K_CM_CLEAR_MIXER:

        case K_CM_LOCK_INCOMING:
        case K_CM_UNLOCK_INCOMING:
        case K_CM_LOCK_OUTGOING:
        case K_CM_UNLOCK_OUTGOING:

        case K_CM_INCREASE_VOLUME:
        case K_CM_DECREASE_VOLUME:

        case K_CM_STOP_RECORD:
        case K_CM_PAUSE_RECORD:
        case K_CM_RESUME_RECORD:

        case K_CM_STOP_LISTEN:

        case K_CM_PLAY_SOUND_CARD:
        case K_CM_STOP_SOUND_CARD:
        case K_CM_RINGBACK:
#if K3L_AT_LEAST(1,4,0) && !K3L_AT_LEAST(2,0,0)
        case K_CM_VOIP_START_DEBUG:
        case K_CM_VOIP_STOP_DEBUG:
        case K_CM_VOIP_DUMP_STAT:
#endif

#if K3L_AT_LEAST(1,5,3)
        case K_CM_END_OF_NUMBER:
#endif

#if K3L_AT_LEAST(1,5,4)
        case K_CM_SET_VOLUME:
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_CM_ENABLE_CALL_ANSWER_INFO:
        case K_CM_DISABLE_CALL_ANSWER_INFO:

        case K_CM_SS_TRANSFER:

        case K_CM_CHECK_NEW_SMS:
        case K_CM_GET_SMS:
        case K_CM_PREPARE_SMS:
        case K_CM_SEND_SMS:

        case K_CM_START_CADENCE:
        case K_CM_STOP_CADENCE:
#endif
#if K3L_HAS_MPTY_SUPPORT
        case K_CM_HOLD_SWITCH:
        case K_CM_MPTY_CONF:
        case K_CM_MPTY_SPLIT:
#endif
            if (params != NULL)
            {
                extra += "param='";
                extra += (params ? params : "<empty>");
                extra += "'";

                return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
            }
            else
            {
                return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)));
            }

        case K_CM_SEND_DTMF: /* ?? */
            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)));

        /****/

        case K_CM_STOP_AUDIO:
            extra  = "stop='";
            switch ((params ? (int)(*params) : -1))
            {
                case 1:   extra += "tx";
                case 2:   extra += "rx";
                case 3:   extra += "tx+rx";
                default:  extra += "<unknown>";
            }
            extra  = "'";
            
            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        /****/
 
#if K3L_AT_LEAST(1,5,2) && !K3L_AT_LEAST(2,0,0)
        case K_CM_ISDN_DEBUG:
            extra  = "flags='";
            extra += isdnDebug((unsigned long)params);
            extra += "'";
            
            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
#endif

        /****/

#if K3L_AT_LEAST(1,5,1)
        case K_CM_USER_INFORMATION:
#endif
            if (params != NULL)
            {
                KUserInformation * userinfo = (KUserInformation *)params;

                std::string tmp((const char*) userinfo->UserInfo, userinfo->UserInfoLength);

                extra = STG(FMT("proto='%d',length='%d',data='%s'")
                        % userinfo->ProtocolDescriptor % userinfo->UserInfoLength % tmp);

                return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
            }
            else
            {
                return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)));
            }

        /****/


        
        case K_CM_CAS_CHANGE_LINE_STT:
        {
            const char status = (params ? *params : 0x00);
            
            extra += "status='";
            extra += (status & 0x01 ? "1" : "0");
            extra += (status & 0x02 ? "1" : "0");
            extra += (status & 0x04 ? "1" : "0");
            extra += (status & 0x08 ? "1" : "0");
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
        }
        
        case K_CM_CAS_SEND_MFC:
        {
            char mfc = (params ? *params : 0xff);
            
            extra = STG(FMT("mfc='%d'") % (int) mfc);

            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
        }

        case K_CM_CAS_SET_MFC_DETECT_MODE:
        {
            int mode = (params ? *((int *)params) : -1);

            extra = STG(FMT("mode='%d'") % mode);

            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
        }

        case K_CM_SET_FORWARD_CHANNEL:
        {
            int channel = (params ? *((int*) params) : -1);

            extra = STG(FMT("forward='%03d'") % channel);

            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
        }

#if K3L_AT_LEAST(1,5,0)
        case K_CM_MAKE_CALL:
            extra  = "options='";
            extra += (params ? params : "<empty>");
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
#endif

        case K_CM_MIXER:
        case K_CM_MIXER_CTBUS:
        {
            if (params)
            {
                std::string src("<unknown>");
                std::string idx("<unknown>");

                KMixerCommand *m = (KMixerCommand*)params;

                switch (m->Source)
                {
                    case kmsChannel:
                        src = "kmsChannel";
                        idx = STG(FMT("%02d") % (int)m->SourceIndex);
                        break;
                    case kmsPlay:
                        src = "kmsPlay";
                        idx = STG(FMT("%02d") % (int)m->SourceIndex);
                        break;
                    case kmsGenerator:
                        src = "kmsGenerator";
                        idx = mixerTone((KMixerTone)m->SourceIndex);
                        break;
                    case kmsCTbus:
                        src = "kmsCTbus";
                        idx = STG(FMT("%02d") % (int)m->SourceIndex);
                        break;
#if (K3L_AT_LEAST(1,4,0) && !K3L_AT_LEAST(1,6,0))
                    case kmsVoIP:
                        src = "kmsVoIP";
                        idx = STG(FMT("%02d") % (int)m->SourceIndex);
#endif
#if K3L_AT_LEAST(1,6,0)
                    case kmsNoDelayChannel:
                        src = "kmsNoDelayChannel";
                        idx = STG(FMT("%02d") % (int)m->SourceIndex);
#endif
                };

                extra = STG(FMT("track='%d',src='%s',index='%s'") % (int)m->Track % src % idx);
            }
            else
            {
                extra = "<unknown>";
            }
                        
            return show(buf, commandName(code), kobject::item(deviceId(dev), mixerId(obj)), extra);
        };

        case K_CM_PLAY_FROM_FILE:
            extra  = "file='";
            extra += (params ? params : "<empty>");
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)), extra);

        case K_CM_RECORD_TO_FILE:
            extra  = "file='";
            extra += (params ? params : "<empty>");
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)), extra);

        case K_CM_RECORD_TO_FILE_EX:
            extra  = "params='";
            extra += (params ? params : "<empty>");
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)), extra);

        case K_CM_PLAY_FROM_STREAM:
        case K_CM_ADD_STREAM_BUFFER:
        {
            struct buffer_param
            {
                const void * ptr;
                const int   size;
            }
            *p = (buffer_param *) params;

            std::stringstream stream;

            extra = STG(FMT("buffer='%p',size='%d'")
                % (const void *) p->ptr % (const int) p->size);

            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)), extra);
        }

        case K_CM_PLAY_FROM_STREAM_EX:
        {
            struct buffer_param
            {
                const void  *  ptr;
                const int     size;
                const char   codec;
            }
            *p = (buffer_param *) params;

            std::string codec;

            switch (p->codec)
            {
                case 0:  codec = "A-Law";
                case 1:  codec = "PCM-08khz";
                case 2:  codec = "PCM-11khz";
                default: codec = "<unknown>";
            }
            
            std::stringstream stream;

            extra = STG(FMT("buffer='%p',size='%d',codec='%s'")
                % (const void *) p->ptr % (const int) p->size % codec);

            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)), extra);
        }
        
        case K_CM_STOP_PLAY:
        case K_CM_PAUSE_PLAY:
        case K_CM_RESUME_PLAY:

        case K_CM_START_STREAM_BUFFER:
        case K_CM_STOP_STREAM_BUFFER:
        
        case K_CM_ENABLE_PLAYER_AGC:
        case K_CM_DISABLE_PLAYER_AGC:

        case K_CM_SEND_BEEP:
        case K_CM_SEND_BEEP_CONF:

        case K_CM_INTERNAL_PLAY:
        case K_CM_INTERNAL_PLAY_EX:
            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)));

        case K_CM_ADD_TO_CONF:
            extra += "conference='";
            extra += (params ? (int) (*params) : -1);
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), mixerId(obj)), extra);
        
        case CM_REMOVE_FROM_CONF:
            return show(buf, commandName(code), kobject::item(deviceId(dev), mixerId(obj)));

        case K_CM_LISTEN:
        case K_CM_PREPARE_FOR_LISTEN:
        {
            int msecs = (params ? *((int*)params) : -1);
            
            extra = STG(FMT("msecs='%d'") % msecs);

            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)), extra);
        }

        case K_CM_SEND_TO_CTBUS:
        case K_CM_RECV_FROM_CTBUS:
        {
            KCtbusCommand *p = (KCtbusCommand*)(params);

            extra = STG(FMT("stream='%02d',timeslot='%02d',enable='%d'")
                % (int32)p->Stream % (int32)p->TimeSlot % (int32)p->Enable);

            return show(buf, commandName(code), kobject::item(deviceId(dev), playerId(obj)), extra);
        }

        case K_CM_SET_LINE_CONDITION:
        case K_CM_SEND_LINE_CONDITION:
            extra  = "condition='";
            extra += signGroupB((KSignGroupB) *((int *) params));
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_CM_SET_CALLER_CATEGORY:
            extra  = "category='";
            extra += signGroupII((KSignGroupII) *((int *) params));
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

#if K3L_AT_LEAST(1,6,0)
        case K_CM_CLEAR_LINK_ERROR_COUNTER:
            return show(buf, commandName(code), kobject::item(deviceId(dev), linkId(obj)));

        case K_CM_SIP_REGISTER:
            if (params != NULL)
            {
                extra += "param='";
                extra += (params ? params : "<empty>");
                extra += "'";

                return show(buf, commandName(code), kobject::item(deviceId(dev)), extra);
            }
            else
            {
                return show(buf, commandName(code), kobject::item(deviceId(dev)));
            }
#endif

        case K_CM_SETUP_H100:
            extra += "option='";
            extra += h100configIndex((KH100ConfigIndex)obj_idx);
            extra += "'value='";
            extra += (params ? STG(FMT("%02d") % (int)(*params)) : "<empty>");
            extra += "'";

            return show(buf, commandName(code), kobject::item(deviceId(dev)), extra);
        
        case K_CM_HARD_RESET:
            return show(buf, commandName(code), kobject::item(deviceId(dev)));

#if !K3L_AT_LEAST(2,0,0) 
        /* como funciona? */
        case K_CM_LOG_REQUEST:
        case K_CM_LOG_CREATE_DISPATCHER:
        case K_CM_LOG_DESTROY_DISPATCHER:

        case K_CM_PING:
#endif
            return show(buf, commandName(code), kobject::item(deviceId(255)));
    }

    return show(buf, commandName(code), kobject::item(deviceId(dev), channelId(dev)));
}

std::string Verbose::event(KSignaling sig, int32 obj_idx, K3L_EVENT *ev)
{
    ushort dev = (ushort) ev->DeviceId;
    ushort obj = (ushort) obj_idx;

    kevent code = (kevent) ev->Code;

    std::string buf, extra;

    switch (code)
    {
        case K_EV_CHANNEL_FREE:
        case K_EV_SEIZE_SUCCESS:
        case K_EV_CALL_SUCCESS:
        case K_EV_NO_ANSWER:
        case K_EV_CONNECT:
        case K_EV_DTMF_SEND_FINISH:
        case K_EV_SEIZURE_START:
        case K_EV_BILLING_PULSE:
        case K_EV_REFERENCE_FAIL:

#if K3L_AT_LEAST(1,4,0)
        case K_EV_CALL_HOLD_START:
        case K_EV_CALL_HOLD_STOP:
#endif

#if K3L_AT_LEAST(1,5,0)
        case K_EV_NEW_CALL:
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_EV_FLASH:
        case K_EV_POLARITY_REVERSAL:
        case K_EV_COLLECT_CALL:
        case K_EV_SS_TRANSFER_FAIL:
        case K_EV_RING_DETECTED:
#endif
#if K3L_HAS_MPTY_SUPPORT
        case K_EV_CALL_MPTY_START:
        case K_EV_CALL_MPTY_STOP:
#endif
            break;

#if K3L_AT_LEAST(1,6,0)
        case K_EV_RECV_FROM_MODEM:
        case K_EV_SMS_INFO:
        case K_EV_SMS_DATA:
            extra  = "data='";
            extra += (ev->Params ? (const char *)(ev->Params) : "<empty>");
            extra += "'";

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_EV_SMS_SEND_RESULT:
            extra  = "result='";
#if K3L_AT_LEAST(2,0,0)
            extra += gsmSmsCause((KGsmSmsCause)ev->AddInfo);
#else
            extra += gsmCallCause((KGsmCallCause)ev->AddInfo);
#endif
            extra += "'";
            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_CALL_ANSWER_INFO:
            extra  = "info='";
            extra += callStartInfo((KCallStartInfo)ev->AddInfo);
            extra += "'";
            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_NEW_SMS:
            if (ev->AddInfo != 0)
            {
                extra  = "messages='";
                extra += STG(FMT("%d") % ev->AddInfo);
                extra += "'";
                return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
            }
            else
            {
                return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)));
            }
        
        case K_EV_ISDN_PROGRESS_INDICATOR:
            if (ev->AddInfo != 0)
            {
                extra  = "indication='";
                extra += q931ProgressIndication((KQ931ProgressIndication)ev->AddInfo);
                extra += "'";
                return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
            }
            else
            {
                return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)));
            }
        
#endif

        case K_EV_CAS_LINE_STT_CHANGED:
            extra = STG(FMT("[a=%d,b=%d,c=%d,d=%d]")
                % ((ev->AddInfo & 0x8) >> 3) % ((ev->AddInfo & 0x4) >> 2)
                % ((ev->AddInfo & 0x2) >> 1) %  (ev->AddInfo & 0x1));

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_CAS_MFC_RECV:
            extra = STG(FMT("digit='%d'") % ev->AddInfo);

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_CALL_FAIL:
            extra  = "cause='";
            extra += callFail(sig, ev->AddInfo);
            extra += "'";

            if (ev->Params != NULL && ev->ParamSize != 0)
            {
                if (!extra.empty())
                    extra += ",";

                extra += "params='";
                extra += (const char *) ev->Params;
                extra += "'";
            }

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_DISCONNECT:
            switch (sig)
            {
#if K3L_AT_LEAST(1,5,1)
                case ksigOpenCCS:
                case ksigPRI_EndPoint:
                case ksigPRI_Network:
                case ksigPRI_Passive:
                    extra  = "cause='";
                    extra += isdnCause((KQ931Cause) ev->AddInfo);
                    extra += "'";
#endif
                default:
                    break;
            }

            if (ev->Params != NULL && ev->ParamSize != 0)
            {
                if (!extra.empty())
                    extra += ",";

                extra += "params='";
                extra += (const char *) ev->Params;
                extra += "'";
            }
            
            if (!extra.empty())
                return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
            else
                return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)));

            break;

#if K3L_AT_LEAST(1,6,0)
        case K_EV_SIP_DTMF_DETECTED:
#endif
        case K_EV_DTMF_DETECTED:
        case K_EV_PULSE_DETECTED:
        case K_EV_DIALED_DIGIT:
            extra = STG(FMT("digit='%c'") % (char)ev->AddInfo);

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_SEIZURE:
        {
            KIncomingSeizeParams *n = (KIncomingSeizeParams *)
                ( ((char*)ev) + sizeof(K3L_EVENT) );

            extra += "orig_addr='";
            extra += n->NumberA;
            extra += "',dest_addr='";
            extra += n->NumberB;
            extra += "'";

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
        }

#if K3L_AT_LEAST(1,4,0)
        case K_EV_VOIP_SEIZURE:
        {
            char *numB = ((char*)ev) + sizeof(K3L_EVENT);
            char *numA = numB + 61;

            extra  = "numberA='";
            extra += numA;
            extra += "',numberB='";
            extra += numB;
            extra += "'";

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
        }
#endif


        case K_EV_END_OF_STREAM:
            return show(buf, eventName(code), kobject::item(deviceId(dev), playerId(obj)));

        case K_EV_AUDIO_STATUS:
            extra  = "tone='";
            extra += mixerTone((KMixerTone)ev->AddInfo);
            extra += "'";
             
            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_CADENCE_RECOGNIZED:
            extra = STG(FMT("cadence='%c'") % (char)(ev->AddInfo));

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_CHANNEL_FAIL:
            extra  = "reason='";
            extra += channelFail(sig, ev->AddInfo);
            extra += "'";

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_SEIZE_FAIL:
            extra  = "reason='";
            extra += seizeFail((KSeizeFail) ev->AddInfo);
            extra += "'";

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_INTERNAL_FAIL:
            extra  = "reason='";
            extra += internalFail((KInternalFail) ev->AddInfo);
            extra += "'";

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);

        case K_EV_HARDWARE_FAIL:
            extra  = "component='";
            extra += systemObject((KSystemObject) ev->AddInfo);
            extra += "'";

            switch (ev->AddInfo)
            {
                case ksoChannel:
                    return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
                case ksoLink:
                    return show(buf, eventName(code), kobject::item(deviceId(dev), linkId(obj)), extra);
                case ksoLinkMon:
                case ksoH100:
                case ksoFirmware:
                case ksoDevice:
                    return show(buf, eventName(code), kobject::item(deviceId(dev)), extra);
                case ksoAPI:
                    return show(buf, eventName(code), kobject::item(deviceId(255)), extra);
            }


        case K_EV_LINK_STATUS:
            // EV_LINK_STATUS has always zero in ObjectInfo (and AddInfo!)
            
            /*
            extra  = "status='";
            extra += linkStatus(sig, ev->ObjectInfo);
            extra += "'";
            return show(buf, eventName(code), kobject::item(deviceId(dev), linkId(obj)), extra);
            */
            
            /* fall throught... */
            
#if K3L_AT_LEAST(1,6,0)
        case K_EV_PHYSICAL_LINK_UP:
        case K_EV_PHYSICAL_LINK_DOWN:
            return show(buf, eventName(code), kobject::item(deviceId(dev), linkId(obj)));
#endif

#if K3L_AT_LEAST(1,5,1)
        case K_EV_USER_INFORMATION:
        {
            KUserInformation *info = (KUserInformation *)(ev->Params);

            std::string data((const char *)info->UserInfo,
                std::min<size_t>(info->UserInfoLength, KMAX_USER_USER_LEN));
            
            extra = STG(FMT("proto='%d',length='%d',data='%s'")
                % info->ProtocolDescriptor % info->UserInfoLength % data);

            return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
        }
#endif

#if K3L_AT_LEAST(1,6,0)
        case K_EV_SIP_REGISTER_INFO:
            extra  = "params='";
            extra += (ev->Params ? (const char *) (ev->Params) : "<unknown>");
            extra += "',status='";
            extra += sipFailures((KSIP_Failures)(ev->AddInfo));
            extra += "'";

            return show(buf, eventName(code), kobject::item(deviceId(dev)), extra);
#endif

        case K_EV_PONG:

#if K3L_AT_LEAST(1,4,0)
        case K_EV_CLIENT_RECONNECT:
#endif
            return show(buf, eventName(code), kobject::item(deviceId(255)));

    }

    // default handler...
    if (ev->Params != NULL && ev->ParamSize != 0)
    {
        extra += "params='";
        extra.append((const char *) ev->Params, (unsigned int) std::max<int>(ev->ParamSize - 1, 0));
        extra += "'";

        return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)), extra);
    }
    else
        return show(buf, eventName(code), kobject::item(deviceId(dev), channelId(obj)));
}

/********************************************/

std::string Verbose::show(std::string & buf, std::string name, kobject::item obj, std::string & extra)
{
    std::string tmp(",");
    tmp += extra;

    generate(buf, name, obj, tmp);
    return buf;
}

std::string Verbose::show(std::string & buf, std::string name, kobject::item obj)
{
    std::string tmp("");

    generate(buf, name, obj, tmp);
    return buf;
}

void Verbose::generate(std::string &buf, std::string &name, kobject::item obj, std::string &extra)
{
    switch (obj.type())
    {
        case kobject::item::DEVICE:
            buf += STG(FMT("<%s> (d=%01hhd%s)") % name
                % (unsigned int)obj.device() % extra);
            break;

        default:
        {
            const char *kind = "object";

            switch (obj.type())
            {
                case kobject::item::CHANNEL:
                    kind = "c";
                    break;
                case kobject::item::PLAYER:
                    kind = "p";
                    break;
                case kobject::item::MIXER:
                    kind = "m";
                    break;
                case kobject::item::LINK:
                    kind = "l";
                default:
                    break;
            }

            buf += STG(FMT("<%s> (d=%01hhd,%s=%03hd%s)")
                % name % (unsigned int) obj.device() % kind
                % (unsigned int) obj.object() % extra);
            break;
        }
    }
}
