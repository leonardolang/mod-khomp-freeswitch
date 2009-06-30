#include "globals.h"


K3LAPI Globals::k3lapi;

ConfigOptions Globals::options;

switch_endpoint_interface_t * Globals::khomp_endpoint_interface = NULL;
switch_api_interface_t      * Globals::api_interface = NULL;
switch_memory_pool_t        * Globals::module_pool = NULL;
int                           Globals::running = 1;

unsigned int     Globals::flags = 0;
int              Globals::calls = 0;
switch_mutex_t * Globals::mutex = NULL;


