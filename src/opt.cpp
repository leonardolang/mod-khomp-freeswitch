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

#include "opt.h"
#include "globals.h"
#include "defs.h"
#include "logger.h"

bool                            Opt::_debug;
std::string                     Opt::_dialplan;
std::string                     Opt::_context;
std::string                     Opt::_user_xfer;
std::map < std::string, CSpan > Opt::_spans;
Opt::GroupToDestMapType         Opt::_groups;
Opt::CadencesMapType            Opt::_cadences;

bool Opt::_echo_canceller;
bool Opt::_auto_gain_control;
bool Opt::_out_of_band_dtmfs;
bool Opt::_suppression_delay;
bool Opt::_pulse_forwarding;
bool Opt::_native_bridge;
bool Opt::_recording;
bool Opt::_has_ctbus;
bool Opt::_fxs_bina;
bool Opt::_fxo_send_pre_audio;
bool Opt::_drop_collect_call;
bool Opt::_ignore_letter_dtmfs;
bool Opt::_optimize_audio_path;

bool         Opt::_auto_fax_adjustment;
unsigned int Opt::_fax_adjustment_timeout;

bool         Opt::_r2_strict_behaviour;
unsigned int Opt::_r2_preconnect_wait;

unsigned int Opt::_fxs_digit_timeout;

unsigned int Opt::_transferdigittimeout;

std::string Opt::_atxfer;
std::string Opt::_blindxfer;

unsigned int Opt::_ringback_co_delay;
unsigned int Opt::_ringback_pbx_delay;

unsigned int Opt::_disconnect_delay;

int Opt::_input_volume;
int Opt::_output_volume;

Opt::DestVectorType     Opt::_fxs_co_dialtone;
Opt::OrigToDestMapType  Opt::_fxs_hotline;
std::string             Opt::_fxs_global_orig_base;

Opt::BoardToOrigMapType Opt::_fxs_orig_base;
Opt::BranchToOptMapType Opt::_branch_options;

std::string Opt::_global_mohclass;
std::string Opt::_global_language;

std::string Opt::_record_prefix;

std::string Opt::_context_gsm_call;
std::string Opt::_context2_gsm_call;
std::string Opt::_context_gsm_sms;
std::string Opt::_context_fxo;
std::string Opt::_context2_fxo;
std::string Opt::_context_fxs;
std::string Opt::_context2_fxs;
std::string Opt::_context_digital;
std::string Opt::_context_pr;

int Opt::_amaflags;
unsigned long long Opt::_callgroup;
unsigned long long Opt::_pickupgroup;

std::string Opt::_accountcode;

unsigned int Opt::_kommuter_timeout;
std::string  Opt::_kommuter_activation;

unsigned int Opt::_audio_packet_size;

void Opt::initialize(void) 
{ 
    Globals::options.add(ConfigOption("debug",    _debug,    false));
    Globals::options.add(ConfigOption("dialplan", _dialplan, "XML"));
    Globals::options.add(ConfigOption("context",  _context,  "default"));

	Globals::options.add(ConfigOption("echo-canceller",       _echo_canceller,       true));
	Globals::options.add(ConfigOption("auto-gain-control",    _auto_gain_control,    true));
	Globals::options.add(ConfigOption("out-of-band-dtmfs",    _out_of_band_dtmfs,    true));
	Globals::options.add(ConfigOption("suppression-delay",    _suppression_delay,    true));
	Globals::options.add(ConfigOption("pulse-forwarding",     _pulse_forwarding,     false));
	Globals::options.add(ConfigOption("native-bridge",        _native_bridge,        true));
	Globals::options.add(ConfigOption("recording",            _recording,            true));
	Globals::options.add(ConfigOption("has-ctbus",            _has_ctbus,            false));
	Globals::options.add(ConfigOption("fxs-bina",             _fxs_bina,             true));
	Globals::options.add(ConfigOption("fxo-send-pre-audio",   _fxo_send_pre_audio,   true));
	Globals::options.add(ConfigOption("drop-collect-call",    _drop_collect_call,    false));
	Globals::options.add(ConfigOption("ignore-letter-dtmfs", _ignore_letter_dtmfs,  false));
	Globals::options.add(ConfigOption("optimize-audio-path",  _optimize_audio_path,  false));

    Globals::options.add(ConfigOption("auto-fax-adjustment",    _auto_fax_adjustment,    true));
    Globals::options.add(ConfigOption("fax-adjustment-timeout", _fax_adjustment_timeout, 30u, 3u, 9999u));

    Globals::options.add(ConfigOption("r2-strict-behaviour", _r2_strict_behaviour, false));
    Globals::options.add(ConfigOption("r2-preconnect-wait",  _r2_preconnect_wait,  250u, 25u, 500u));

    Globals::options.add(ConfigOption("fxs-digit-timeout",   _fxs_digit_timeout,   7u,  1u, 30u));

    Globals::options.add(ConfigOption("transferdigittimeout", _transferdigittimeout, 3u, 0u, 90u));

    Globals::options.add(ConfigOption("atxfer", _atxfer, ""));
    Globals::options.add(ConfigOption("blindxfer", _blindxfer, ""));

    Globals::options.add(ConfigOption("delay-ringback-co",  _ringback_co_delay,  1500u, 0u, 999000u)); 
    Globals::options.add(ConfigOption("delay-ringback-pbx", _ringback_pbx_delay, 2500u, 0u, 999000u));
    
    Globals::options.add(ConfigOption("disconnect-delay", _disconnect_delay, 0u, 0u, 100000u)); 

    Globals::options.add(ConfigOption("input-volume",  _input_volume, 0, -10, 10));
    Globals::options.add(ConfigOption("output-volume", _output_volume, 0, -10, 10));


    Globals::options.add(ConfigOption("fxs-co-dialtone", ProcessFXSCODialtone(), ""));
    Globals::options.add(ConfigOption("fxs-global-orig", _fxs_global_orig_base,  "0"));

    Globals::options.add(ConfigOption("language", _global_language, ""));
    Globals::options.add(ConfigOption("mohclass", _global_mohclass, ""));

    Globals::options.add(ConfigOption("record-prefix", ProcessRecordPrefix(), "/var/spool/asterisk/monitor/"));

    Globals::options.add(ConfigOption("context-fxo",          _context_fxo,       "khomp-DD-CC"));
    Globals::options.add(ConfigOption("context-fxo-alt",      _context2_fxo,      "khomp-DD"));
    Globals::options.add(ConfigOption("context-fxs",          _context_fxs,       "khomp-DD-CC"));
    Globals::options.add(ConfigOption("context-fxs-alt",      _context2_fxs,      "khomp-DD"));
    Globals::options.add(ConfigOption("context-gsm-call",     _context_gsm_call,  "khomp-DD-CC"));
    Globals::options.add(ConfigOption("context-gsm-call-alt", _context2_gsm_call, "khomp-DD"));
    Globals::options.add(ConfigOption("context-gsm-sms",      _context_gsm_sms,   "khomp-sms-DD-CC"));
    Globals::options.add(ConfigOption("context-digital",      _context_digital,   "khomp-DD-LL"));
    Globals::options.add(ConfigOption("context-pr",           _context_pr,        "khomp-DD-CC"));

    Globals::options.add(ConfigOption("amaflags",    ProcessAMAFlags(),    "default"));
    Globals::options.add(ConfigOption("callgroup",   ProcessCallGroup(),   "0"));
    Globals::options.add(ConfigOption("pickupgroup", ProcessPickupGroup(), "0"));

    Globals::options.add(ConfigOption("accountcode", _accountcode, ""));

    ConfigOption::string_allowed_type kommuter_allowed;
    kommuter_allowed.insert("auto");
    kommuter_allowed.insert("manual");

    Globals::options.add(ConfigOption("kommuter-activation", _kommuter_activation , "auto", kommuter_allowed));
    Globals::options.add(ConfigOption("kommuter-timeout",    _kommuter_timeout ,(unsigned int) 10 , (unsigned int) 0 , (unsigned int) 255));

    Globals::options.add(ConfigOption("audio-packet-length", _audio_packet_size,
         (unsigned int)KHOMP_READ_PACKET_SIZE, (unsigned int)KHOMP_MIN_READ_PACKET_SIZE, (unsigned int)KHOMP_MAX_READ_PACKET_SIZE, 8u));

    Globals::options.add(ConfigOption("log-to-disk",    ProcessLogOptions(O_GENERIC), "standard", false));
    Globals::options.add(ConfigOption("log-to-console", ProcessLogOptions(O_CONSOLE), "standard", false));

    Globals::options.add(ConfigOption("trace",          ProcessTraceOptions(), "", false));

    /* aliases */
    Globals::options.synonym("context-gsm", "context-gsm-call");
    Globals::options.synonym("context-gsm-alt", "context-gsm-call-alt");
    Globals::options.synonym("echocanceller", "echo-canceller");
    Globals::options.synonym("dtmfsuppression", "out-of-band-dtmfs");
    Globals::options.synonym("dtmfsupression", "out-of-band-dtmfs");
    Globals::options.synonym("pulsedetection", "pulse-forwarding");
    Globals::options.synonym("r2preconnectwait", "r2-preconnect-wait");
    Globals::options.synonym("bridge", "native-bridge");
    Globals::options.synonym("suppressiondelay", "suppression-delay");
    Globals::options.synonym("log", "log-to-disk");
    Globals::options.synonym("volume", "output-volume");
    Globals::options.synonym("disconnectdelay", "disconnect-delay");

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
        DBG(FUNC,FMT("%s") % (*i).c_str());
    }
}

void Opt::load_configuration(const char *file_name, const char **section, bool show_errors)
{
    switch_xml_t cfg, xml, settings;

    //TODO: Implement NestedLock !?
    switch_mutex_init(&Globals::mutex, SWITCH_MUTEX_NESTED, Globals::module_pool);

    if (!(xml = switch_xml_open_cfg(file_name, &cfg, NULL))) {
        K::Logger::Logg(C_ERROR,FMT("Open of %s failed") % file_name);
        return;
    }

    /* Load all the global settings pertinent to all boards */
    settings = ProcessSimpleXML(cfg,"settings");
    
    /* Process channel settings */
    ProcessSimpleXML(settings,"channels");

    /* Process groups settings */
    ProcessGroupXML(settings);

    /* Process cadence settings */
    ProcessCadenceXML(settings);

    /* Process fxs branches settings */
    ProcessFXSBranchesXML(settings);

    /* Process hotlines settings */
    ProcessFXSHotlines(settings);

    /* Process fxs options settings */
    ProcessFXSOptions(settings);

    /* Process conf for each span */
    ProcessSpans(cfg);
    
    switch_xml_free(xml);
}

void Opt::printConfiguration(switch_stream_handle_t* stream)
{
    for( std::map<std::string, CSpan>::iterator ii=_spans.begin(); ii!=_spans.end(); ++ii )
    {
        stream->write_function(stream,
                               "Span: %s.\nDialplan: %s.\nContext: %s.\nDialstring: %s.\n\n",
                               (*ii).first.c_str(),
                               (*ii).second._dialplan.c_str(),
                               (*ii).second._context.c_str(),
                               (*ii).second._dialstring.c_str());
    }
}

void Opt::clean_configuration(void)
{
    _fxs_orig_base.clear();
    _fxs_hotline.clear();
    _fxs_co_dialtone.clear();
    _branch_options.clear();

    _groups.clear();
    _cadences.clear();

    _cadences.insert(CadencesPairType("fast-busy", CadenceType(100,100)));
    _cadences.insert(CadencesPairType("ringback", CadenceType(1000,4000)));
    _cadences.insert(CadencesPairType("pbx-dialtone", CadenceType(1000,100)));
    _cadences.insert(CadencesPairType("co-dialtone", CadenceType(0,0)));
    _cadences.insert(CadencesPairType("vm-dialtone", CadenceType(1000,100,100,100)));
}

void Opt::ProcessFXSCODialtone::operator()(std::string options) // const
{
    Strings::vector_type tokens;
    Strings::tokenize(options, tokens, ",");

    for (Strings::vector_type::iterator i = tokens.begin(); i != tokens.end(); i++)
        _fxs_co_dialtone.push_back(*i);
}

void Opt::ProcessRecordPrefix::operator()(std::string path) // const
{
    if (mkdir(path.c_str(), 493 /* 0755 */) < 0 && errno != EEXIST)
    {
        throw ConfigProcessFailure("the default recording directory could not be created.");
    }
    else
    {
        _record_prefix = path;
    }
}

void Opt::ProcessAMAFlags::operator()(std::string options)
{
/*
    //TODO: Do we need this ?
    amaflags = ast_cdr_amaflags2int(options.c_str());

    if (amaflags < 0)
        throw ConfigProcessFailure(STG(FMT("invalid AMA flags: %s") % options));
*/
}

void Opt::ProcessCallGroup::operator()(std::string options)
{
//    _callgroup = options.c_str();
}

void Opt::ProcessPickupGroup::operator()(std::string options)
{
//    _pickupgroup = options.c_str();
}

void Opt::ProcessLogOptions::operator()(std::string options)
{
    switch (_output)
    {   
        case O_GENERIC:
            ProcessLogDisk(-1, options, false, true);
            break;

        case O_CONSOLE:
            ProcessLogConsole(-1, options, false, true);
            break;

        default:
            throw ConfigProcessFailure("attempt to process unknown log file configuration");
            break;
    }   
}

void Opt::ProcessTraceOptions::operator()(std::string options)
{
    Strings::vector_type tokens;
    Strings::tokenize(options, tokens, ",");

    bool         enable_k3l_tracing  = false;
    bool         enable_r2_tracing   = false;
    bool         enable_rdsi_tracing = false;

    for (Strings::vector_type::iterator i = tokens.begin(); i != tokens.end(); i++)
    {
        std::string tok = Strings::trim(*i);

        if (tok == "k3l")
        {
            enable_k3l_tracing = true;
        }
        else if (tok == "r2")
        {
            enable_r2_tracing = true;
        }
        else if (tok == "rdsi")
        {
            enable_rdsi_tracing = true;
        }
    }

    //TODO: Need port of logconfig too
    /*
    Logfile logfile;

    // k3l tracing 
    if (enable_k3l_tracing ^ K::globals::flag_trace_k3l)
    {
        K::logconfig::set(logfile, "K3L", "Value", enable_k3l_tracing);
        K::logconfig::set(logfile, "K3L", "CallProgress", enable_k3l_tracing);
        K::logconfig::set(logfile, "K3L", "CallAnalyzer", enable_k3l_tracing);
        K::logconfig::set(logfile, "K3L", "CadenceRecog", enable_k3l_tracing);
        K::logconfig::set(logfile, "K3L", "CallControl", enable_k3l_tracing);
        K::logconfig::set(logfile, "K3L", "Fax", enable_k3l_tracing);
    }

    // r2 tracing 
    K::logconfig::set(logfile, "R2", "Value", enable_r2_tracing);
    K::logconfig::set(logfile, "R2", "Signaling", enable_r2_tracing);
    K::logconfig::set(logfile, "R2", "States", enable_r2_tracing);

    // ISDN tracing 
    if (K::globals::flag_trace_rdsi ^ enable_rdsi_tracing)
    {
        K::logconfig::set(logfile, "ISDN", "Value", enable_rdsi_tracing);
        K::logconfig::set(logfile, "ISDN", "Lapd", enable_rdsi_tracing);
        K::logconfig::set(logfile, "ISDN", "Q931", enable_rdsi_tracing);
    }

    if (K::logconfig::commit(logfile))
    {
        K::util::sendCmd(-1, -1, CM_LOG_UPDATE);
    }
    */
}

void Opt::ProcessLogDisk(int fd, std::string & options, bool invert, bool unique)
{
    class_type classe = ( fd == -1 ? C_MESSAGE : C_CLI );

	Strings::vector_type tokens;
	Strings::tokenize(options, tokens, ",");

	bool flag_errors    = false;
	bool flag_warnings  = false;
	bool flag_messages  = false;
	bool flag_events    = false;
	bool flag_commands  = false;
	bool flag_audio     = false;
	bool flag_modem     = false;
	bool flag_link      = false;
	bool flag_cas       = false;
	bool flag_functions = false;
	bool flag_threads   = false;
	bool flag_locks     = false;
	bool flag_streams   = false;

	Strings::Merger strs;

	for (Strings::vector_type::iterator i = tokens.begin(); i != tokens.end(); i++)
	{
		std::string tok = Strings::trim(*i);

		     if ((tok) == "errors")     flag_errors    = true;
		else if ((tok) == "warnings")   flag_warnings  = true;
		else if ((tok) == "messages")   flag_messages  = true;
		else if ((tok) == "events")     flag_events    = true;
		else if ((tok) == "commands")   flag_commands  = true;
		else if ((tok) == "audio")      flag_audio     = true;
		else if ((tok) == "modem")      flag_modem     = true;
		else if ((tok) == "link")       flag_link      = true;
		else if ((tok) == "cas")        flag_cas       = true;
		else if ((tok) == "functions")  flag_functions = true;
		else if ((tok) == "threads")    flag_threads   = true;
		else if ((tok) == "locks")      flag_locks     = true;
		else if ((tok) == "streams")    flag_streams   = true;
		else if ((tok) == "standard")
		{
			flag_errors   = true;
			flag_warnings = true;
			flag_messages = true;
			flag_link     = true;
		}
		else if ((tok) == "debugging")
		{
			flag_errors    = true;
			flag_warnings  = true;
			flag_messages  = true;
			flag_commands  = true;
			flag_events    = true;
			flag_audio     = true;
			flag_modem     = true;
			flag_link      = true;
			flag_cas       = true;
			flag_functions = true;
		}
		else if ((tok) == "all")
		{
			flag_errors    = true;
			flag_warnings  = true;
			flag_messages  = true;
			flag_events    = true;
			flag_commands  = true;
			flag_audio     = true;
			flag_modem     = true;
			flag_link      = true;
			flag_cas       = true;
			flag_functions = true;
			flag_threads   = true;
			flag_locks     = true;
			flag_streams   = true;
		}
		else
		{
			/* do not show! */
			continue;
		}

		strs.add(tok);
	}

    //TODO: Need logger class to enlight 
    /*
	if ((unique && !flag_errors) || flag_errors)
		K::logger::logg.classe(C_ERROR).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_errors));

	if ((unique && !flag_warnings) || flag_warnings)
		K::logger::logg.classe(C_WARNING).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_warnings));

	if ((unique && !flag_messages) || flag_messages)
		K::logger::logg.classe(C_MESSAGE).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_messages));

	if ((unique && !flag_events) || flag_events)
		K::logger::logg.classe(C_EVENT).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_events));

	if ((unique && !flag_commands) || flag_commands)
		K::logger::logg.classe(C_COMMAND).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_commands));

	if ((unique && !flag_audio) || flag_audio)
		K::logger::logg.classe(C_AUDIO_EV).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_audio));

	if ((unique && !flag_modem) || flag_modem)
		K::logger::logg.classe(C_MODEM_EV).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_modem));

	if ((unique && !flag_link) || flag_link)
		K::logger::logg.classe(C_LINK_STT).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_link));

	if ((unique && !flag_cas) || flag_cas)
		K::logger::logg.classe(C_CAS_MSGS).set(O_GENERIC, log_manager::option::ENABLED, (!invert && flag_cas));

	if ((unique && !flag_functions) || flag_functions)
		K::logger::logg.classe(C_DBG_FUNC).enabled(!invert && flag_functions);

	if ((unique && !flag_threads) || flag_threads)
		K::logger::logg.classe(C_DBG_THRD).enabled(!invert && flag_threads);

	if ((unique && !flag_locks) || flag_locks)
		K::logger::logg.classe(C_DBG_LOCK).enabled(!invert && flag_locks);

	if ((unique && !flag_streams) || flag_streams)
		K::logger::logg.classe(C_DBG_STRM).enabled(!invert && flag_streams);

	if (!strs.empty())
	{
		K::logger::logg2(classe, fd, FMT("NOTICE: %s %sthe logging of the following messages: %s.")
			% (invert ? "Disabling" : "Enabling")
			% (unique ? "just " : "") % strs.merge(", "));

		if ((flag_streams || flag_locks) && !invert)
		{
			K::logger::logg2(classe, fd, "WARNING: You have enabled *INTENSIVE* debug messages for the Khomp channel!");
			K::logger::logg2(classe, fd, "WARNING: Don't *EVER* use these options on production systems, unless you *REALLY* know what you are doing!");
		}
		else if ((flag_functions || flag_threads) && !invert)
		{
			K::logger::logg2(classe, fd, "WARNING: You have enabled some debug messages for the Khomp channel.");
			K::logger::logg2(classe, fd, "WARNING: Do not use these options on production systems unless you really know what you are doing!");
		}
	}
	else
	{
		K::logger::logg2(classe, fd, "WARNING: No valid log messages have been specified, doing nothing.");
	}
*/
}

void Opt::ProcessLogConsole(int fd, std::string & options, bool invert, bool unique)
{
	class_type classe = ( fd == -1 ? C_MESSAGE : C_CLI );

	Strings::vector_type tokens;
	Strings::tokenize(options, tokens, ",");

	bool flag_errors   = false;
	bool flag_warnings = false;
	bool flag_messages = false;
	bool flag_events   = false;
	bool flag_commands = false;
	bool flag_audio    = false;
	bool flag_modem    = false;
	bool flag_link     = false;
	bool flag_cas      = false;

	Strings::Merger strs;

	for (Strings::vector_type::iterator i = tokens.begin(); i != tokens.end(); i++)
	{
		std::string tok = Strings::trim(*i);

		     if ((tok) == "errors")     flag_errors   = true;
		else if ((tok) == "warnings")   flag_warnings = true;
		else if ((tok) == "messages")   flag_messages = true;
		else if ((tok) == "events")     flag_events   = true;
		else if ((tok) == "commands")   flag_commands = true;
		else if ((tok) == "audio")      flag_audio    = true;
		else if ((tok) == "modem")      flag_modem    = true;
		else if ((tok) == "link")       flag_link     = true;
		else if ((tok) == "cas")        flag_cas      = true;
		else if ((tok) == "standard")
		{
			flag_errors   = true;
			flag_warnings = true;
			flag_messages = true;
			flag_link     = true;
		}
		else if ((tok) == "all")
		{
			flag_errors   = true;
			flag_warnings = true;
			flag_messages = true;
			flag_events   = true;
			flag_commands = true;
			flag_audio    = true;
			flag_modem    = true;
			flag_link     = true;
			flag_cas      = true;
		}
		else
		{
            //TODO: Need logger class to enlight 
           // K::logger::logg2(classe, fd, FMT("WARNING: The following console message option is not valid and will be ignored: %s.") % (tok) );
            continue;
		}

		strs.add(tok);
	}

    //TODO: Need logger class to enlight 
    /*

	if ((unique && !flag_errors) || flag_errors)
		K::logger::logg.classe(C_ERROR).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_errors));

	if ((unique && !flag_warnings) || flag_warnings)
		K::logger::logg.classe(C_WARNING).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_warnings));

	if ((unique && !flag_messages) || flag_messages)
		K::logger::logg.classe(C_MESSAGE).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_messages));

	if ((unique && !flag_events) || flag_events)
		K::logger::logg.classe(C_EVENT).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_events));

	if ((unique && !flag_commands) || flag_commands)
		K::logger::logg.classe(C_COMMAND).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_commands));

	if ((unique && !flag_audio) || flag_audio)
		K::logger::logg.classe(C_AUDIO_EV).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_audio));

	if ((unique && !flag_modem) || flag_modem)
		K::logger::logg.classe(C_MODEM_EV).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_modem));

	if ((unique && !flag_link) || flag_link)
		K::logger::logg.classe(C_LINK_STT).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_link));

	if ((unique && !flag_cas) || flag_cas)
		K::logger::logg.classe(C_CAS_MSGS).set(O_CONSOLE, log_manager::option::ENABLED, (!invert && flag_cas));

	if (!strs.empty())
	{
		K::logger::logg2(classe, fd, FMT("NOTICE: %s %sthe following console messages: %s.")
			% (invert ? "Disabling" : "Enabling")
			% (unique ? "just " : "") % strs.merge(", "));
	}
	else
	{
		K::logger::logg2(classe, fd, "WARNING: No valid console messages have been specified, doing nothing.");
	}
    */
}

switch_xml_t Opt::ProcessSimpleXML(switch_xml_t &xml, const std::string& child_name)
{
    switch_xml_t param, child;

    if ((child = switch_xml_child(xml, child_name.c_str())))
    {
        for (param = switch_xml_child(child, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            try
            {
                Globals::options.process(var, val);
            }
            catch (ConfigProcessFailure e)
            {
                K::Logger::Logg(C_ERROR,FMT("config processing error: %s. [%s=%s]")  
                    % e.msg.c_str()
                    % var
                    % val);

            }
        }
    }

    return child;
}

void Opt::ProcessGroupXML(switch_xml_t &xml)
{
    switch_xml_t param, child;

    if ((child = switch_xml_child(xml,"groups")))
    {
        for (param = switch_xml_child(child, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            try
            {
                DBG(FUNC,FMT("loading group '%s' options: '%s'...")
                    % var
                    % val);

                _groups.insert(GroupToDestPairType(var,val));

            }
            catch (ConfigProcessFailure e)
            {
                K::Logger::Logg(C_ERROR,FMT("config processing error: %s. [%s=%s]")  
                    % e.msg.c_str()
                    % var
                    % val);
            }
        }
    }
}

void Opt::ProcessCadenceXML(switch_xml_t &xml)
{
    switch_xml_t param, child;

    if ((child = switch_xml_child(xml,"cadences")))
    {
        for (param = switch_xml_child(child, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            try
            {
                DBG(FUNC,FMT("loading cadences '%s' options: '%s'...")
                    % var
                    % val);

                Strings::vector_type values;
                Strings::tokenize(val, values, " :,.");

                if (values.size() != 2 && values.size() != 4)
                {
                    DBG(FUNC,FMT("file '%s': wrong number of arguments at cadence '%s'!")
                        % "khomp.conf.xml"
                        % var);
                }
                else
                {
                    CadenceType cadence;

                    cadence.ring       = Strings::toulong(Strings::trim(values[0]));
                    cadence.ring_s     = Strings::toulong(Strings::trim(values[1]));

                    if (values.size() == 4)
                    {   
                        cadence.ring_ext   = Strings::toulong(Strings::trim(values[2]));
                        cadence.ring_ext_s = Strings::toulong(Strings::trim(values[3]));
                    }   

                    _cadences.erase(var); /* erases previous (possibly predefined) cadence */
                    _cadences.insert(CadencesPairType(var, cadence));
                } 
            }
            catch (ConfigProcessFailure e)
            {
                K::Logger::Logg(C_ERROR,FMT("config processing error: %s. [%s=%s]")  
                    % e.msg.c_str()
                    % var
                    % val);

            }                  
            catch (Strings::invalid_value e)
            {   
               K::Logger::Logg(C_ERROR,FMT("file '%s': number expected at cadence '%s', got '%s'.") 
                    % "khomp.conf.xml"
                    % var
                    % e.value().c_str());
            }   
        }
    }
}

void Opt::ProcessFXSBranchesXML(switch_xml_t &xml)
{
    switch_xml_t param, child;

    if ((child = switch_xml_child(xml,"fxs-branches")))
    {
        for (param = switch_xml_child(child, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");


            DBG(FUNC,FMT("loading fxs-branches '%s' options: '%s'...")
                    % var
                    % val);

            try
            {
                unsigned long orig_number = Strings::toulong(var);

                /* gcc! stop complaining! */
                (void)orig_number;

                Strings::vector_type boards;
                Strings::tokenize(val, boards, " :,");

                if (boards.size() < 1)
                {
                    DBG(FUNC,FMT("file '%s': orig number '%s' without any board!")
                        % "khomp.conf.xml"
                        % var);
                    continue;
                }

                for (Strings::vector_type::iterator iter = boards.begin(); iter != boards.end(); iter++)
                {
                    /* quebrar em strings por vírgula/espaço */
                    unsigned long serial_number = Strings::toulong(Strings::trim(*iter));

                    bool found = false;

                    for (unsigned int device = 0; device < Globals::k3lapi.device_count(); device++)
                    {
                        K3L_DEVICE_CONFIG & conf = Globals::k3lapi.device_config(device);

                        std::string str_serial(conf.SerialNumber);

                        if (Strings::toulong(str_serial) == serial_number)
                        {
                            found = true;

                            _fxs_orig_base.insert(BoardToOrigPairType(device, var));
                            break;
                        }
                    }

                    if (!found)
                    {
                        DBG(FUNC,FMT("file '%s': board with serial number '%04lu' not found!\n") 
                        % "khomp.conf.xml" % serial_number);

                        break;
                    }
                }
            }
            catch (ConfigProcessFailure e)
            {
                K::Logger::Logg(C_ERROR,FMT("config processing error: %s. [%s=%s]")  
                    % e.msg.c_str()
                    % var
                    % val);
            }
        }
    }
}

void Opt::ProcessFXSHotlines(switch_xml_t &xml)
{
    switch_xml_t param, child;

    if ((child = switch_xml_child(xml,"fxs-hotlines")))
    {
        for (param = switch_xml_child(child, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            try
            {
                DBG(FUNC,FMT("loading fxs-hotlines '%s' options: '%s'...")
                    % var
                    % val);

                unsigned long orig_number = Strings::toulong(var);

                (void)orig_number; /* stop complaining! */

                _fxs_hotline.insert(OrigToDestPairType(var, val));

            }
            catch (ConfigProcessFailure e)
            {
                K::Logger::Logg(C_ERROR,FMT("config processing error: %s. [%s=%s]")  
                    % e.msg.c_str()
                    % var
                    % val);        
            }
            catch (Strings::invalid_value e)
            {
                DBG(FUNC,FMT("file '%s': number expected, got '%s'!") 
                    % "khomp.conf.xml" % e.value().c_str());
            }  
        }
    }
}

void Opt::ProcessFXSOptions(switch_xml_t &xml)
{
    switch_xml_t param, child;

    if ((child = switch_xml_child(xml,"fxs-options")))
    {
        for (param = switch_xml_child(child, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            try
            {
                DBG(FUNC,FMT("loading fxs-options '%s' options: '%s'...")
                    % var
                    % val);

                Strings::vector_type branches;
                Strings::tokenize(var, branches, " ,");

                if (branches.size() < 1)
                {   
                    //TODO: Get linenumber
                    DBG(FUNC,FMT("file '%s': no branches specified in line %d!") 
                    % "khomp.conf.xml" % 0);
                }           
                else
                {   
                    for (Strings::vector_type::iterator iter = branches.begin();
                            iter != branches.end(); iter++)
                    {                   
                        std::string tmp_branch = Strings::trim(*iter);

                        unsigned long branch_number = Strings::toulong(tmp_branch);
                        (void) branch_number;

                        _branch_options.insert(BranchToOptPairType(tmp_branch, val));
                    }
                }
            }
            catch (ConfigProcessFailure e)
            {
                K::Logger::Logg(C_ERROR,FMT("config processing error: %s. [%s=%s]")  
                    % e.msg.c_str()
                    % var
                    % val);      
            }
            catch (Strings::invalid_value e)
            {
                DBG(FUNC,FMT("file '%s': number expected, got '%s'!") 
                    % "khomp.conf.xml" % e.value().c_str());
            }
        }
    }
}

void Opt::ProcessSpans(switch_xml_t &xml)
{
    switch_xml_t span, param; 

    /* Load each span */
    for (span = switch_xml_child(xml, "span"); span; span = span->next)
    {
        char *span_id = (char *) switch_xml_attr_soft(span, "id");
        DBG(FUNC,FMT("New span detected: %s")  % span_id);


        CSpan * tmp_span = new CSpan();
        
        for (param = switch_xml_child(span, "param"); param; param = param->next)
        {
            char *var = (char *) switch_xml_attr_soft(param, "name");
            char *val = (char *) switch_xml_attr_soft(param, "value");

            if (!strcmp("dialplan", var))
            {
                tmp_span->_dialplan = val;
            }
            else if (!strcmp("context", var))
            {
                tmp_span->_context = val;
            }
            else if (!strcmp("dialstring", var))
            {
                tmp_span->_dialstring = val;
            }
            else
            {
                K::Logger::Logg(C_ERROR,FMT("Option %s in span %s not valid.")
                % var % span_id);
            }
                    
            /* HELP: We can't seem to be able to use this here
                because ConfigOptions does not seem to support
                tree configuration model.
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
             */            
        }
        _spans.insert(span_pair_type(span_id, *tmp_span));
    }
}
