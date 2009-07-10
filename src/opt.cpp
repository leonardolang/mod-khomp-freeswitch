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
std::string Opt::_dialplan;
std::string Opt::_context;


//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_dialplan, Opt::_dialplan);
//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_string, Opt::_codec_string);
//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_codec_rates_string, Opt::_codec_rates_string);
//SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_ip, Opt::_ip);

void Opt::initialize(void) 
{ 
    Globals::options.add(ConfigOption("debug", _debug, false));
    Globals::options.add(ConfigOption("dialplan", _dialplan, "XML"));
    Globals::options.add(ConfigOption("context", _context, "default"));
    
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
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "%s", (*i).c_str());
    }
}

void Opt::load_configuration(const char *file_name, const char **section, bool show_errors)
{
    switch_xml_t cfg, xml, settings, param, span;

    switch_mutex_init(&Globals::mutex, SWITCH_MUTEX_NESTED, Globals::module_pool);
    if (!(xml = switch_xml_open_cfg(file_name, &cfg, NULL))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", file_name);
        return;
    }

    /* Load all the global settings pertinent to all boards */
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
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_WARNING,
                                  "config processing error: %s. [%s=%s]\n",
                                  e.msg.c_str(),
                                  var,
                                  val);

            }
        }
    }

    /* Load each span */
    for (span = switch_xml_child(cfg, "span"); span; span = span->next)
    {
        char *span_id = (char *) switch_xml_attr_soft(span, "id");

        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "New span detected: %s.\n", span_id);
        
        for (param = switch_xml_child(span, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            try
            {
                Globals::options.process(var, val);
            }
            catch (ConfigProcessFailure e)
            {
                switch_log_printf(SWITCH_CHANNEL_LOG,
                                  SWITCH_LOG_WARNING,
                                  "config processing error: %s. [%s=%s]\n",
                                  e.msg.c_str(),
                                  var,
                                  val);
            }
        }
    }
    
    switch_xml_free(xml);

}

void Opt::clean_configuration(void)
{
}

