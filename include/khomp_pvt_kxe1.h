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

#ifndef _KHOMP_PVT_E1_H_
#define _KHOMP_PVT_E1_H_

#include "khomp_pvt.h"

/******************************************************************************/
/********************************** E1 Board **********************************/
/******************************************************************************/
struct BoardE1: public Board
{
/******************************************************************************/
/********************************* E1 Channel *********************************/
struct KhompPvtE1: public KhompPvt 
{
/*********************************** E1 Call **********************************/
    struct CallE1 : public Call
    {
        CallE1() {}

        bool process(std::string name, std::string value = "")
        {
            if (name == "answer_info")
            {
                _call_info_report = true;
            }
            else if (name == "drop_on")
            {
                _call_info_report = true;

                Strings::vector_type drop_item;
                Strings::tokenize (value, drop_item, ".+");

                for (Strings::vector_type::iterator i = drop_item.begin(); i != drop_item.end(); i++)
                {

                         if ((*i) == "message_box")        _call_info_drop |= CI_MESSAGE_BOX;
                    else if ((*i) == "human_answer")       _call_info_drop |= CI_HUMAN_ANSWER;
                    else if ((*i) == "answering_machine")  _call_info_drop |= CI_ANSWERING_MACHINE;
                    else if ((*i) == "carrier_message")    _call_info_drop |= CI_CARRIER_MESSAGE;
                    else if ((*i) == "unknown")            _call_info_drop |= CI_UNKNOWN;
                    else
                    {
                        K::Logger::Logg(C_ERROR, FMT("unknown paramenter to 'calldrop' Dial option: '%s'.") % (*i));
                        continue;
                    }

                    DBG(FUNC, FMT("droping call on '%s'.") % (*i));
                }
            }
            else
            {            
                return Call::process(name, value);
            }

            return true;
        }
        
        bool clear()
        {
            _call_info_report = false;
            _call_info_drop = 0;
 
            return Call::clear();
        }

        /* report what we got? */
        bool _call_info_report;

        /* what call info flags should make us drop the call? */
        long int _call_info_drop;
    };
/******************************************************************************/
    KhompPvtE1(K3LAPI::target & target) : KhompPvt(target) {}

    ~KhompPvtE1() {}

    CallE1 * callE1()
    {
        return (CallE1 *)call();
    }

    bool isOK(void); 

    bool isPhysicalFree() 
    {
        K3L_CHANNEL_STATUS status;

        if (k3lGetDeviceStatus (_target.device, _target.object + ksoChannel, &status, sizeof (status)) != ksSuccess)
            return false; 

        bool physically_free = (status.AddInfo == kecsFree);

        if(status.CallStatus != kcsFree || !physically_free)
        {
            return false;
        }

        return true;
    }

    void onChannelRelease(K3L_EVENT *e);
    bool onCallSuccess(K3L_EVENT *e);


    virtual int eventHandler(K3L_EVENT *e)
    {
        DBG(FUNC, D("(E1) c"));

        int ret = ksSuccess;

        switch(e->Code)
        {
            case EV_CHANNEL_FREE:
            case EV_CHANNEL_FAIL:
                onChannelRelease(e);
                break;
            default:
                ret = KhompPvt::eventHandler(e);
                break;
        }        

        DBG(FUNC, D("(E1) r"));
        return ret;
    }
    
    int makeCall(std::string params = "");
    int doChannelAnswer(CommandRequest &);

    bool indicateBusyUnlocked(int cause, bool sent_signaling = false);
};
/******************************************************************************/
/********************************** ISDN Channel ******************************/
struct KhompPvtISDN: public KhompPvtE1 
{
/********************************** ISDN Call *********************************/
    struct CallISDN : public CallE1
    {
    
        CallISDN() {}

        bool process(std::string name, std::string value = "")
        {
            if (name == "uui")
            {
                Strings::vector_type values;
                Strings::tokenize(value, values, "#", 2);

                try
                {
                    std::string uui_proto_s = values[0];
                    std::string uui_data_s = values[1];

                    _uui_descriptor = Strings::toulong(uui_proto_s);
                    _uui_information.append(uui_data_s);

                    DBG(FUNC, FMT("uui adjusted (%s, '%s')!") % uui_proto_s.c_str() % uui_data_s.c_str());
                }
                catch (...)
                {
                    K::Logger::Logg(C_ERROR, FMT("invalid uui protocol descriptor: '%s' is not a number.") % value.c_str());
                }
            }
            else
            {            
                return CallE1::process(name, value);
            }

            return true;
        }
    
        bool clear()
        {
            _uui_descriptor = -1;
            _uui_information = "";
            _isdn_cause = -1;
    
            return CallE1::clear();
        }
        
        /* used for isdn EV_USER_INFORMATION */
        long int     _uui_descriptor;
        std::string  _uui_information;
        long int     _isdn_cause;
    };
/******************************************************************************/
    KhompPvtISDN(K3LAPI::target & target) : KhompPvtE1(target) {}

    ~KhompPvtISDN() {}

    CallISDN * callISDN()
    {
        return (CallISDN *)call();
    }
    
    RingbackDefs::RingbackStType sendRingBackStatus(int rb_value = RingbackDefs::RB_SEND_DEFAULT); 
    
    bool sendPreAudio(int rb_value = RingbackDefs::RB_SEND_NOTHING);

    bool onIsdnProgressIndicator(K3L_EVENT *e);
    
    bool onNewCall(K3L_EVENT *e);

    bool onCallSuccess(K3L_EVENT *e);
    
    bool onCallFail(K3L_EVENT *e);

    virtual int eventHandler(K3L_EVENT *e)
    {
        DBG(FUNC, D("(ISDN) c"));

        int ret = ksSuccess;
        
        switch(e->Code)
        {
            case EV_ISDN_PROGRESS_INDICATOR:
                onIsdnProgressIndicator(e);
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
            default:
                ret = KhompPvtE1::eventHandler(e);
                break;
        }        

        DBG(FUNC, D("(ISDN) r"));
        return ret;
    }

    int makeCall(std::string params = "");

    int causeFromCallFail(int fail);

    int callFailFromCause(int cause);

    void reportFailToReceive(int fail_code);

    int doChannelAnswer(CommandRequest &); 

};
/******************************************************************************/
/********************************* R2 Channel *********************************/
struct KhompPvtR2: public KhompPvtE1
{
/********************************* R2 Call ************************************/
    struct CallR2 : public CallE1
    {
    
        CallR2() {}


        bool process(std::string name, std::string value = "")
        {
            if (name == "category")
            {
                try
                {
                    unsigned long int category = Strings::toulong (value);
                    DBG(FUNC, FMT("r2 category adjusted (%s)!") % value.c_str());
                    _r2_category = category;
                }
                catch (...)
                {
                   K::Logger::Logg(C_ERROR, FMT("invalid r2 category: '%s' is not a number.") % value.c_str());
                }

            }
            else
            {
                return CallE1::process(name, value);
            }
            return true;
        }
        
        bool clear()
        {
            _r2_category  = -1; 
            _r2_condition = -1;

            return CallE1::clear();
        }

        long int _r2_category;
        long int _r2_condition;
    };
/******************************************************************************/
    KhompPvtR2(K3LAPI::target & target) : KhompPvtE1(target) 
    {
        K3L_E1600A_FW_CONFIG dspAcfg;

        if (k3lGetDeviceConfig(_target.device, ksoFirmware + kfiE1600A, &dspAcfg, sizeof(dspAcfg)) != ksSuccess)
        {
            DBG(FUNC, PVT_FMT(target, "unable to get signaling locality for board: assuming brazilian signaling"));
 
            _r2_country = Verbose::R2_COUNTRY_BRA;
           
            return;
        }
        Regex::Expression e(".+\\((Arg|Bra|Chi|Mex|Ury|Ven)\\).+", Regex::E_EXTENDED);
        std::string fwname(dspAcfg.FwVersion);

        Regex::Match what(fwname, e);

        if (!what.matched() || !what.matched(1))
        {
            DBG(FUNC, PVT_FMT(target, "invalid firmware string, unable to find country code: assuming brazilian signaling.\n"));
            
            _r2_country = Verbose::R2_COUNTRY_BRA;
            return;
        }

        std::string country = what.submatch(1);
    
        /**/ if (country == "Arg")
            _r2_country = Verbose::R2_COUNTRY_ARG;
        else if (country == "Bra")
            _r2_country = Verbose::R2_COUNTRY_BRA;
        else if (country == "Chi")
            _r2_country = Verbose::R2_COUNTRY_CHI;
        else if (country == "Mex")
            _r2_country = Verbose::R2_COUNTRY_MEX;
        else if (country == "Ury")
            _r2_country = Verbose::R2_COUNTRY_URY;
        else if (country == "Ven")
            _r2_country = Verbose::R2_COUNTRY_VEN;
        else
        {
            DBG(FUNC, PVT_FMT(target, "invalid firmware string (%s), assuming brazilian signaling.") % country.c_str());

            _r2_country = Verbose::R2_COUNTRY_BRA;
            return;
        }
    
        DBG(FUNC, PVT_FMT(target, "adjusting country signaling to code '%s'...")
            % country.c_str());

    }

    ~KhompPvtR2() {}
    
    CallR2 * callR2()
    {
        return (CallR2 *)call();
    }

    RingbackDefs::RingbackStType sendRingBackStatus(int rb_value = RingbackDefs::RB_SEND_DEFAULT); 

    bool sendPreAudio(int rb_value = RingbackDefs::RB_SEND_NOTHING);

    
    bool onNewCall(K3L_EVENT *e);
    
    bool onCallSuccess(K3L_EVENT *e);

    bool onCallFail(K3L_EVENT *e);
    
    virtual int eventHandler(K3L_EVENT *e)
    {
        DBG(FUNC, D("(R2) c"));

        int ret = ksSuccess;        

        switch(e->Code)
        {
            case EV_NEW_CALL:
                onNewCall(e);
                break;
            case EV_CALL_SUCCESS:
                onCallSuccess(e);
                break;
            case EV_CALL_FAIL:
                onCallFail(e);
                break;
            default:
                ret = KhompPvtE1::eventHandler(e);
                break;
        }        

        DBG(FUNC, D("(R2) r"));
        return ret;

    }
    
    int makeCall(std::string params = "");

    int causeFromCallFail(int fail);

    int callFailFromCause(int cause);

    void reportFailToReceive(int fail_code);

    int doChannelAnswer(CommandRequest &); 

public:
    Verbose::R2CountryType _r2_country;

};
/******************************************************************************/
/******************************************************************************/
    BoardE1(int id) : Board(id) {}

    void onLinkStatus(K3L_EVENT *e);


    virtual int eventHandler(const int obj, K3L_EVENT *e)
    {
        DBG(FUNC, D("(E1 Board) c"));

        int ret = ksSuccess;

        switch(e->Code)
        {
        case EV_LINK_STATUS:
            onLinkStatus(e);
            break;
        case EV_PHYSICAL_LINK_DOWN:
            K::Logger::Logg(C_ERROR,FMT("Link %02hu on board %02hu is DOWN.") 
            % e->AddInfo           
            % e->DeviceId);
            break;
        case EV_PHYSICAL_LINK_UP:
            K::Logger::Logg(C_ERROR,FMT("Link %02hu on board %02hu is UP.") 
            % e->AddInfo           
            % e->DeviceId);
            break;
        default:
            ret = Board::eventHandler(obj, e);
            break;
        }

        DBG(FUNC, D("(E1 Board) r"));
        return ret;
    }
   
};
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
#endif /* _KHOMP_PVT_H_*/

