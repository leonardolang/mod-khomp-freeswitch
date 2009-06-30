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

#include "opt.h"
#include "globals.h"

bool        Opt::_debug;
//std::string Opt::_ip;
//unsigned int Opt::_port;
std::string Opt::_dialplan;
std::string Opt::_codec_string;
//char * Opt::_codec_order[SWITCH_MAX_CODECS];
//int Opt::_codec_order_last;
std::string Opt::_codec_rates_string;
//char * Opt::_codec_rates[SWITCH_MAX_CODECS];
//int Opt::_codec_rates_last;

//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_dialplan, Opt::_dialplan);
//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_string, Opt::_codec_string);
//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_rates_string, Opt::_codec_rates_string);
//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_ip, Opt::_ip);

void Opt::initialize(void) 
{ 
    Globals::options.add(ConfigOption("debug", _debug, false));
//    Globals::options.add(ConfigOption("port", _port, 4569u, 255u, 65535u));
//    Globals::options.add(ConfigOption("ip", _ip, "localhost"));
    Globals::options.add(ConfigOption("dialplan", _dialplan, "default"));
    Globals::options.add(ConfigOption("codec-prefs", _codec_string, "PCMA"));
    Globals::options.add(ConfigOption("codec-rates", _codec_rates_string, "8"));
}

void Opt::obtain(void)
{
    /* reset loaded options */
    Globals::options.reset();

    load_configuration("khomp.conf", NULL);

    /* commit, loading defaults where needed */
    ConfigOptions::messages_type msgs = Globals::options.commit();

    /* config already full loaded at this point, so we can use our own log system... */
    for (ConfigOptions::messages_type::iterator i = msgs.begin(); i != msgs.end(); i++)
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, (*i).c_str());
    }
}

void Opt::load_configuration(const char *file_name, const char **section, bool show_errors)
{
    switch_xml_t cfg, xml, settings, param;

    switch_mutex_init(&Globals::mutex, SWITCH_MUTEX_NESTED, Globals::module_pool);
    if (!(xml = switch_xml_open_cfg(file_name, &cfg, NULL))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", file_name);
        return;
    }

    if ((settings = switch_xml_child(cfg, "settings")))
    {
        for (param = switch_xml_child(settings, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            try
            {
                Globals::options.process(var, val);
            }
            catch (ConfigProcessFailure e)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "config processing error: %s.\n", e.msg.c_str());

            }
            /*
            if (!strcmp(var, "_debug")) {
                _debug = atoi(val);
            } else if (!strcmp(var, "port")) {
                _port = atoi(val);
            } else if (!strcmp(var, "ip")) {
                set_global_ip(val);
            //} else if (!strcmp(var, "codec-master")) {
                //if (!strcasecmp(val, "us")) {
                //    switch_set_flag(&globals, GFLAG_MY_CODEC_PREFS);
                //}
            } else if (!strcmp(var, "dialplan")) {
                set_global_dialplan(val);
            } else if (!strcmp(var, "codec-prefs")) {
                    set_global_codec_string(val);
                    _codec_order_last = switch_separate_string(_codec_string, ',', _codec_order, SWITCH_MAX_CODECS);
                    
            } else if (!strcmp(var, "codec-rates")) {
                set_global_codec_rates_string(val);
                _codec_rates_last = switch_separate_string(_codec_rates_string, ',', _codec_rates, SWITCH_MAX_CODECS);
            }
            */

        }
    }

    switch_xml_free(xml);

}

void Opt::clean_configuration(void)
{
}

