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

#ifndef _GLOBALS_H_
#define _GLOBALS_H_
#include "k3lapi.hpp"

#include <config_options.hpp>
#include <k3lutil.hpp>
#include <verbose.hpp>
#include <regex.hpp>

#include <vector>
#include <string>
#include <fstream>

extern "C"
{
    #include <switch.h>
}

/* As this is a static-variable-only struct, member variable *
 * names need not to get "_" in front of the name            */

struct Globals
{
    static const unsigned int switch_packet_duration  =                          30; // in ms
    static const unsigned int boards_packet_duration  =                          16; // in ms

    static const unsigned int switch_packet_size      = switch_packet_duration *  8; // in bytes
    static const unsigned int boards_packet_size      = boards_packet_duration *  8; // in bytes

    static const unsigned int    cng_buffer_size      =          boards_packet_size; // in bytes

    static K3LAPI        k3lapi;
    static K3LUtil       k3lutil;
    static Verbose       verbose;

    static std::string     base_path;
    static std::ofstream   generic_file;

    /* Config options class */
    static ConfigOptions options;

    static switch_endpoint_interface_t *khomp_endpoint_interface;
    static switch_api_interface_t      *api_interface;
    static switch_memory_pool_t        *module_pool;

    static int             running;
    static int             calls;

    static unsigned int    flags;
    static switch_mutex_t *mutex;
};

#endif /* _GLOBALS_H_ */
