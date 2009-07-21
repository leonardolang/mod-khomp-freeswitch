#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "k3lapi.hpp"

#include <config_options.hpp>
#include <verbose.hpp>

#include <vector>

#define OBJ_FMT(dev,obj,msg) \
	FMT("(dev=%02d,channel=%03d)" msg) % dev % obj

#define OBJ_MSG(dev,obj,msg) \
	"(dev=%02d,channel=%03d)" msg, dev, obj

#define PVT_FMT(tgt,msg) \
	FMT("(dev=%02hu,channel=%03hu)" msg) % tgt.device % tgt.object

#define PVT_MSG(tgt,msg) \
	"(dev=%02hu,channel=%03hu)" msg, tgt.device, tgt.object

#define STR(fmt) \
	STG(fmt).c_str()

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
	static Verbose       verbose;

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
