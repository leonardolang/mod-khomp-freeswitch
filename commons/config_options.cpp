#include <config_options.hpp>

ConfigOption::ConfigOption(std::string name, const ConfigOption::StringType & value, const ConfigOption::StringType defvalue, string_allowed_type allowed, bool list_me)
: _my_name(name), _value_data(new StringData(const_cast<StringType &>(value), defvalue, allowed)),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

ConfigOption::ConfigOption(std::string name, const ConfigOption::StringType & value, const ConfigOption::StringType defvalue, bool list_me)
: _my_name(name), _value_data(new StringData(const_cast<StringType &>(value), defvalue, string_allowed_type())),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

ConfigOption::ConfigOption(std::string name, const ConfigOption::SignedIntType & value, const ConfigOption::SignedIntType defvalue,
                             ConfigOption::SignedIntType min, ConfigOption::SignedIntType max, ConfigOption::SignedIntType step, bool list_me)
: _my_name(name), _value_data(new SignedIntData(const_cast<SignedIntType &>(value), defvalue, range<SignedIntType>(min, max, step))),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

ConfigOption::ConfigOption(std::string name, const ConfigOption::UnsignedIntType & value, const ConfigOption::UnsignedIntType defvalue,
                             ConfigOption::UnsignedIntType min, ConfigOption::UnsignedIntType max, ConfigOption::UnsignedIntType step, bool list_me)
: _my_name(name), _value_data(new UnsignedIntData(const_cast<UnsignedIntType &>(value), defvalue, range<UnsignedIntType>(min, max, step))),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

ConfigOption::ConfigOption(std::string name, const ConfigOption::BooleanType & value, const ConfigOption::BooleanType defvalue, bool list_me)
: _my_name(name), _value_data(new BooleanData(const_cast<BooleanType &>(value), defvalue)),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

ConfigOption::ConfigOption(std::string name, ConfigOption::fun_type fun, std::string defvalue, string_allowed_type allowed, bool list_me)
: _my_name(name), _value_data(new fun_data_type(fun, defvalue, allowed)),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

ConfigOption::ConfigOption(std::string name, ConfigOption::fun_type fun, std::string defvalue, bool list_me)
: _my_name(name), _value_data(new fun_data_type(fun, defvalue, string_allowed_type())),
  _list_me(list_me), _values(NULL), _loaded(false)
{};

ConfigOption::~ConfigOption(void)
{
    if (_values)
    {
        for (unsigned int i = 0; _values[i] != NULL; i++)
            delete _values[i];

        delete[] _values;
    }
};

void ConfigOption::set(ConfigOption::StringType value)
{
    switch (_value_data.which())
    {
        case ID_STRING:
        {
            try 
            {
                StringData & tmp = _value_data.get<StringData>();
            

                if (tmp.string_allowed.empty())
                {
                    tmp.string_val = value;
                    _loaded = true;
                }
                else
                {
                    if (tmp.string_allowed.find(value) != tmp.string_allowed.end())
                    {
                        tmp.string_val = value;
                        _loaded = true;
                        return;
                    }

                    std::string allowed_string;

                    for (string_allowed_type::iterator i = tmp.string_allowed.begin(); i != tmp.string_allowed.end(); i++)
                    {
                        allowed_string += " '";
                        allowed_string += (*i);
                        allowed_string += "'";
                    }
    
                    throw config_process_failure(STG(FMT("value '%s' not allowed for option '%s' (allowed values:%s)")
                        % value % _my_name % allowed_string));
                }
                break;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
        }

        case ID_FUN:
        {
            try 
            {
                fun_data_type & tmp = _value_data.get<fun_data_type>();
                tmp.fun_val(value);
                _loaded = true;
                break;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }

        }

        default:
        {
            throw config_process_failure(STG(FMT("option '%s' is not of type string, nor function defined") % _my_name));
        }
    }
}

void ConfigOption::set(ConfigOption::SignedIntType value)
{
    try
    {
        SignedIntData & tmp = _value_data.get<SignedIntData>();

        if (value < tmp.sint_range.minimum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too low)") % value % _my_name));

        if (value > tmp.sint_range.maximum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too high)") % value % _my_name));

        if (((value - tmp.sint_range.minimum) % tmp.sint_range.step) != 0)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (outside allowed step)") % value % _my_name));

        tmp.sint_val = value;
        _loaded = true;
    }
    catch(ValueType::InvalidType & e)
    {
        throw;
    }
}

void ConfigOption::set(ConfigOption::UnsignedIntType value)
{
    try
    {
        UnsignedIntData & tmp = _value_data.get<UnsignedIntData>();

        if (value < tmp.uint_range.minimum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too low)") % value % _my_name));

        if (value > tmp.uint_range.maximum)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (too high)") % value % _my_name));

        if (((value - tmp.uint_range.minimum) % tmp.uint_range.step) != 0)
            throw config_process_failure(STG(FMT("value '%d' out-of-range for option '%s' (outside allowed step)") % value % _my_name));

        tmp.uint_val = value;
        _loaded = true;
    }
    catch(ValueType::InvalidType & e)
    {
        throw;
    }
}

void ConfigOption::set(ConfigOption::BooleanType value)
{
    try
    {
        BooleanData & tmp = _value_data.get<BooleanData>();
        tmp.bool_val = value;
        _loaded = true;
    }
    catch(ValueType::InvalidType & e)
    {
        throw;
    }
}

std::string & ConfigOption::name(void) { return _my_name; };

ConfigOption::value_id_type ConfigOption::type(void) 
{ 
    return (value_id_type) _value_data.which(); 
};

const char ** ConfigOption::values(void)
{
    if (_values != NULL)
        return _values;

    switch ((value_id_type) _value_data.which())
    {
        case ConfigOption::ID_BOOL:
        {
            _values = new const char*[3];

            _values[0] = strdup("yes");
            _values[1] = strdup("no");
            _values[2] = NULL;

            return _values;
        }

        case ConfigOption::ID_SINT:
        {
            try
            {
                SignedIntData & tmp = _value_data.get<SignedIntData>();


                unsigned int count = ((tmp.sint_range.maximum - tmp.sint_range.minimum) / tmp.sint_range.step) + 1;
                unsigned int index = 0;

                _values = new const char*[count + 1];

                for (SignedIntType i = tmp.sint_range.minimum; i <= tmp.sint_range.maximum; i += tmp.sint_range.step, index++)
                    _values[index] = strdup(STG(FMT("%d") % i).c_str());

                _values[index] = NULL;

                return _values;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
        }

        case ConfigOption::ID_UINT:
        {
            try
            {
                UnsignedIntData & tmp = _value_data.get<UnsignedIntData>();

                unsigned int count = ((tmp.uint_range.maximum - tmp.uint_range.minimum) / tmp.uint_range.step) + 1;
                unsigned int index = 0;

                _values = new const char*[count + 1];

                for (UnsignedIntType i = tmp.uint_range.minimum; i <= tmp.uint_range.maximum; i += tmp.uint_range.step, index++)
                    _values[index] = strdup(STG(FMT("%d") % i).c_str());

                _values[index] = NULL;

                return _values;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
        }

        case ConfigOption::ID_STRING:
        {
            try
            {
                StringData & tmp = _value_data.get<StringData>();
            
                _values = new const char*[ tmp.string_allowed.size() + 1 ];

                unsigned int index = 0;

                for (string_allowed_type::iterator i = tmp.string_allowed.begin(); i != tmp.string_allowed.end(); i++, index++)
                    _values[index] = strdup((*i).c_str());

                _values[index] = NULL;

                return _values;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
        }

        case ConfigOption::ID_FUN:
        {
            try
            {
                fun_data_type & tmp = _value_data.get<fun_data_type>();
            
                _values = new const char*[ tmp.fun_allowed.size() + 1 ];

                unsigned int index = 0;
    
                for (string_allowed_type::iterator i = tmp.fun_allowed.begin(); i != tmp.fun_allowed.end(); i++, index++)
                    _values[index] = strdup((*i).c_str());

                _values[index] = NULL;
                return _values;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }

        }

        default:
            throw config_process_failure(STG(FMT("unknown type identifier '%d'") % _value_data.which()));
    }
};

void ConfigOption::reset(void)
{
    _loaded = false;
};

void ConfigOption::commit(void)
{
    if (_loaded)
        return;

    switch ((value_id_type) _value_data.which())
    {
        case ConfigOption::ID_BOOL:
        {
            try
            {
                BooleanData & tmp = _value_data.get<BooleanData>();
                tmp.bool_val = tmp.bool_default;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case ConfigOption::ID_SINT:
        {
            try
            {
                SignedIntData & tmp = _value_data.get<SignedIntData>();
                tmp.sint_val = tmp.sint_default;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
            break;
        }

        case ConfigOption::ID_UINT:
        {
            try
            {
                UnsignedIntData & tmp = _value_data.get<UnsignedIntData>();
                tmp.uint_val = tmp.uint_default;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
            break;
        }

        case ConfigOption::ID_STRING:
        {
            try
            {
                StringData & tmp = _value_data.get<StringData>();
                tmp.string_val = tmp.string_default;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
            break;
        }

        case ConfigOption::ID_FUN:
        {
            try
            {
                fun_data_type & tmp = _value_data.get<fun_data_type>();
                tmp.fun_val(tmp.fun_default);
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }
            break;
        }

        default:
            throw config_process_failure(STG(FMT("unknown type identifier '%d'") % _value_data.which()));
    }

    _loaded = true;
};

void ConfigOption::copy_from(ConfigOption & src)
{
    if (src._value_data.which() != _value_data.which())
        throw config_process_failure(STG(FMT("unable to copy options, source type differs from destination.")));

    if (!src._loaded)
        return;

    switch ((value_id_type) _value_data.which())
    {
        case ConfigOption::ID_BOOL:
        {
            try
            {
                BooleanData & stmp = src._value_data.get<BooleanData>();
                BooleanData & dtmp = _value_data.get<BooleanData>();
                /* do not copy references, but values.. */
                bool tmpval = stmp.bool_val;
                dtmp.bool_val = tmpval;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case ConfigOption::ID_SINT:
        {
            try
            {
                SignedIntData & stmp = src._value_data.get<SignedIntData>();
                SignedIntData & dtmp = _value_data.get<SignedIntData>();
                /* do not copy references, but values.. */
                int tmpval = stmp.sint_val;
                dtmp.sint_val = tmpval;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case ConfigOption::ID_UINT:
        {
            try
            {
                UnsignedIntData & stmp = src._value_data.get<UnsignedIntData>();
                UnsignedIntData & dtmp = _value_data.get<UnsignedIntData>();
                /* do not copy references, but values.. */
                unsigned int tmpval = stmp.uint_val;
                dtmp.uint_val = tmpval;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }


            break;
        }

        case ConfigOption::ID_STRING:
        {
            try
            {
                StringData & stmp = src._value_data.get<StringData>();
                StringData & dtmp = _value_data.get<StringData>();
                /* do not copy references, but values.. */
                std::string tmpval = stmp.string_val;
                dtmp.string_val = tmpval;
            }
            catch(ValueType::InvalidType & e)
            {
                throw;
            }

            break;
        }

        case ConfigOption::ID_FUN:
        {
            /* TO IMPLEMENT (NEEDS ANOTHER METHOD ON FUNCTION FOR GETTING VALUE) */

//            fun_data_type & tmp = boost::get<fun_data_type>(_value_data);
//
//            if (!tmp.loaded)
//            {
//                tmp.fun_val(tmp.fun_default);
//                tmp.loaded = true;
//            }
            break;
        }

        default:
            throw config_process_failure(STG(FMT("unknown type identifier '%d'") % _value_data.which()));
    }

    _loaded = true;
};

/*********************************/

bool ConfigOptions::add(ConfigOption option)
{
    //option_map_type::iterator iter2 = _map.begin();

    //boost::tie(iter2, ok2) 
    std::pair<option_map_type::iterator, bool> ret = _map.insert(option_pair_type(option.name(), option));

    return ret.second;
}

bool ConfigOptions::synonym(std::string equiv_opt, std::string main_opt)
{
    //syn_option_map_type::iterator iter = _syn_map.begin();

    //boost::tie(iter, ok) 
    std::pair<syn_option_map_type::iterator, bool> ret = _syn_map.insert(syn_option_pair_type(equiv_opt, main_opt));

    return ret.second;
}

ConfigOptions::string_set ConfigOptions::options(void)
{
    string_set res;

    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
        res.insert((*i).first);

    return res;
}

void ConfigOptions::process(const char * name, const char * value)
{
    option_map_type::iterator iter = find_option(name);

    if (iter == _map.end())
        throw config_process_failure(STG(FMT("unknown option '%s'") % name));

    try
    {
        switch ((*iter).second.type())
        {
            case ConfigOption::ID_SINT:
                set<ConfigOption::SignedIntType>((*iter).first, Strings::toulong(value));
                return;
            case ConfigOption::ID_UINT:
                set<ConfigOption::UnsignedIntType>((*iter).first, Strings::tolong(value));
                return;
            case ConfigOption::ID_BOOL:
                set<ConfigOption::BooleanType>((*iter).first, Strings::toboolean(value));
                return;
            case ConfigOption::ID_STRING:
            case ConfigOption::ID_FUN:
                set<ConfigOption::StringType>((*iter).first, std::string(value));
                return;
            default:
                throw config_process_failure(STG(FMT("unknown type identifier '%d'") % (*iter).second.type()));
        }
    }
    catch (Strings::invalid_value e)
    {
        throw config_process_failure(STG(FMT("invalid value '%s' for option '%s'") % value % name));
    }
}

const char ** ConfigOptions::values(const char * name)
{
    option_map_type::iterator iter = find_option(name);

    if (iter == _map.end())
        throw config_process_failure(STG(FMT("unknown option '%s'") % name));

    return (*iter).second.values();
}

const char ** ConfigOptions::values(void)
{
    if (_values != NULL)
        return _values;

    unsigned int count = 0;

    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
        if ((*i).second.list_me())
            ++count;

    _values = new const char*[ count + 1 ];

    unsigned int index = 0;

    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
    {
        if ((*i).second.list_me())
        {
            _values[index] = strdup((*i).first.c_str());
            ++index;
        }
    }

    _values[index] = NULL;

    return _values;
}

void ConfigOptions::reset(void)
{
    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
        (*i).second.reset();
}

ConfigOptions::messages_type ConfigOptions::commit(void)
{
    messages_type msgs;
    
    for (option_map_type::iterator i = _map.begin(); i != _map.end(); i++)
    {
        try
        {
            (*i).second.commit();
        }
        catch (config_process_failure e)
        {
            msgs.push_back(e.msg);
        }
    }
    
    return msgs;
}

bool ConfigOptions::loaded(std::string name)
{
    option_map_type::iterator iter = find_option(name);

    if (iter == _map.end())
        return false;

    return iter->second.loaded();
}

void ConfigOptions::copy_from(ConfigOptions & source, std::string name)
{
    option_map_type::iterator iter_src = source.find_option(name);
    option_map_type::iterator iter_dst = find_option(name);

    if (iter_src == source._map.end())
        throw config_process_failure(STG(FMT("unknown option '%s' on source") % name));

    if (iter_dst == _map.end())
        throw config_process_failure(STG(FMT("unknown option '%s' on destination") % name));

    iter_dst->second.copy_from(iter_src->second);
}

ConfigOptions::option_map_type::iterator ConfigOptions::find_option(std::string name)
{
    syn_option_map_type::iterator syn_iter = _syn_map.find(name);

    if (syn_iter != _syn_map.end())
        name = syn_iter->second;

    option_map_type::iterator iter = _map.find(name);

    return iter;
}