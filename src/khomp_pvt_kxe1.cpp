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
 *
 */

#include "khomp_pvt_kxe1.h"

CKhompPvtE1::CKhompPvtE1(K3LAPI::target & target) : CBaseKhompPvt(target) {};

void CKhompPvtE1::on_ev_new_call(K3L_EVENT * e)
{
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO,
                      "New call on %u to %s from %s. [EV_NEW_CALL]\n",
                      _target.object,
                      Globals::k3lapi.get_param(e, "dest_addr").c_str(),
                      Globals::k3lapi.get_param(e, "orig_addr").c_str());
    
    if (khomp_channel_from_event(_target.device, _target.object, e) != ksSuccess )
    {
        switch_log_printf(SWITCH_CHANNEL_LOG,
                          SWITCH_LOG_CRIT,
                          "Something bad happened while getting channel session. Device:%u/Channel:%u. [EV_NEW_CALL]\n",
                          _target.device, _target.object);
    }
    try 
    {
        Globals::k3lapi.command(_target.device, _target.object, CM_RINGBACK, NULL);
        Globals::k3lapi.command(_target.device, _target.object, CM_CONNECT, NULL); 
    }
    catch (K3LAPI::failed_command & err)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not set board channel status! [EV_NEW_CALL]\n");
    }
}