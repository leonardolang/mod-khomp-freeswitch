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
#ifndef _KHOMP_PVT_E1_H_
#define _KHOMP_PVT_E1_H_

#include "khomp_pvt.h"


struct CKhompPvtE1: public CBaseKhompPvt {

    CKhompPvtE1(K3LAPI::target & target);
    ~CKhompPvtE1();
    /* Implement whatever you want */
    /*
    void on_ev_new_call(K3L_EVENT *);
    void on_ev_seizure_start(K3L_EVENT *);
    void on_ev_connect(K3L_EVENT *);
    void on_ev_disconnect(K3L_EVENT *);
    void on_ev_call_success(K3L_EVENT *);
    void on_ev_channel_free(K3L_EVENT *);
    void on_ev_no_answer(K3L_EVENT *);
    void on_ev_call_hold_start(K3L_EVENT *);
    void on_ev_call_hold_stop(K3L_EVENT *);
    void on_ev_call_answer_info(K3L_EVENT *);
    void on_ev_ss_transfer_fail(K3L_EVENT *);
    void on_ev_ring_detected(K3L_EVENT *);
    void on_ev_prolarity_reversal(K3L_EVENT *);
    void on_ev_collect_call(K3L_EVENT *);
    void on_ev_cas_mfc_recv(K3L_EVENT *);
    void on_ev_cas_line_stt_changed(K3L_EVENT *);
    void on_ev_dtmf_detected(K3L_EVENT *);
    void on_ev_pulse_detected(K3L_EVENT *);
    void on_ev_flash(K3L_EVENT *);
    void on_ev_billing_pulse(K3L_EVENT *);
    void on_ev_audio_status(K3L_EVENT *);
    void on_ev_cadence_recognized(K3L_EVENT *);
    void on_ev_dtmf_send_finnish(K3L_EVENT *);
    void on_ev_end_of_stream(K3L_EVENT *);
    void on_ev_user_information(K3L_EVENT *);
    void on_ev_isdn_progess_indicator(K3L_EVENT *);
    void on_ev_isdn_subaddress(K3L_EVENT *);
    void on_ev_dialed_digit(K3L_EVENT *);
    void on_ev_recv_from_modem(K3L_EVENT *);
    void on_ev_new_sms(K3L_EVENT *);
    void on_ev_sms_info(K3L_EVENT *);
    void on_ev_sms_data(K3L_EVENT *);
    void on_ev_sms_send_result(K3L_EVENT *);
    void on_ev_call_fail(K3L_EVENT *);
    void on_ev_reference_fail(K3L_EVENT *);
    void on_ev_channel_fail(K3L_EVENT *);
    void on_ev_internal_fail(K3L_EVENT *);
    void on_ev_client_reconnect(K3L_EVENT *);
    void on_ev_link_status(K3L_EVENT *);
    void on_ev_physical_link_down(K3L_EVENT *);
    void on_ev_physical_link_up(K3L_EVENT *);
    void on_ev_untreated(K3L_EVENT *);
     */
};

#endif /* _KHOMP_PVT_H_*/

