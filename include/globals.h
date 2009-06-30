#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "k3lapi.hpp"

#include <config_options.hpp>

#include <vector>

extern "C"
{
    #include <switch.h>
}

/* As this is a static-variable-only struct, member variable *
 * names need not to get "_" in front of the name            */

struct Globals
{
	static const unsigned int    packet_duration  = 16;                   // in ms
	static const unsigned int        packet_size  = packet_duration *  8; // in bytes
	static const unsigned int stream_buffer_size  = packet_size     *  4; // in bytes
	static const unsigned int record_buffer_size  = packet_size     * 32; // in bytes

    static K3LAPI        k3lapi;

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
