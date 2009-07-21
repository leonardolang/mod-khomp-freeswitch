/*  
    KHOMP endpoint driver for FreeSWITCH library.
    Copyright (C) 2007-2009 Khomp Ind. & Com.  
  
  The contents of this file are subject to the Mozilla Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.
  
*/

#include "globals.h"

K3LAPI  Globals::k3lapi;
K3LUtil Globals::k3lutil(Globals::k3lapi);
Verbose Globals::verbose(Globals::k3lapi);

ConfigOptions Globals::options;

switch_endpoint_interface_t * Globals::khomp_endpoint_interface = NULL;
switch_api_interface_t      * Globals::api_interface = NULL;
switch_memory_pool_t        * Globals::module_pool = NULL;
int                           Globals::running = 1;

unsigned int     Globals::flags = 0;
int              Globals::calls = 0;
switch_mutex_t * Globals::mutex = NULL;

