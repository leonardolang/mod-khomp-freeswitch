#include <set>
#include <map>
#include <vector>

#include <strings.hpp>

#include <variant.hpp>
#include <format.hpp>

#ifndef _CONFIG_OPTIONS_HPP_
#define _CONFIG_OPTIONS_HPP_

struct ConfigProcessFailure
{
    ConfigProcessFailure(std::string _msg): msg(_msg) {};
    std::string msg;
};

struct ConfigOption
{
    typedef int          SignedIntType;
    typedef unsigned int UnsignedIntType;
    typedef bool         BooleanType;
    typedef std::string  StringType;

    //typedef boost::function1<void, std::string>     
    typedef void (*FunctionType)(std::string);

    typedef std::set < StringType >     string_allowed_type;
    
    /* this should reflect 'variant.which()'! */
    typedef enum
    {
        ID_SINT   = 0,
        ID_UINT   = 1,
        ID_BOOL   = 2,
        ID_STRING = 3,
        ID_FUN    = 4,
    }
    value_id_type;

    template < typename number_type >
    struct Range
    {
        Range(number_type _minimum, number_type _maximum, number_type _step)
        : minimum(_minimum), maximum(_maximum), step(_step) {};

        number_type minimum, maximum, step;
    };

    struct SignedIntData : public VariantBaseType
    {
        SignedIntData(SignedIntType & _sint_val, SignedIntType _sint_default, Range< SignedIntType > _sint_Range)
        : sint_val(_sint_val), sint_default(_sint_default), sint_Range(_sint_Range) {};

        int which()
        {
            return ID_SINT;
        }

        SignedIntType        & sint_val;
        SignedIntType          sint_default;
        Range<SignedIntType>   sint_Range;
    };

    struct UnsignedIntData : public VariantBaseType
    {
        UnsignedIntData(UnsignedIntType & _uint_val, UnsignedIntType _uint_default, Range< UnsignedIntType > _uint_Range)
        : uint_val(_uint_val), uint_default(_uint_default), uint_Range(_uint_Range) {};

        int which()
        {
            return ID_UINT;
        }

        UnsignedIntType        & uint_val;
        UnsignedIntType          uint_default;
        Range<UnsignedIntType>   uint_Range;
    };

    struct BooleanData : public VariantBaseType
    {
        BooleanData(BooleanType & _bool_val, BooleanType _bool_default)
        : bool_val(_bool_val), bool_default(_bool_default) {};
        
        int which()
        {
            return ID_BOOL;
        }

        BooleanType      & bool_val;
        BooleanType        bool_default;
    };

    struct StringData : public VariantBaseType
    {
        StringData(std::string & _string_val, std::string _string_default, string_allowed_type _string_allowed)
        : string_val(_string_val), string_default(_string_default), string_allowed(_string_allowed) {};
        
        int which()
        {
            return ID_STRING;
        }

        std::string         & string_val;
        std::string           string_default;
        string_allowed_type   string_allowed;
    };

    struct FunctionData : public VariantBaseType
    {
        FunctionData(FunctionType _fun_val, std::string _fun_default, string_allowed_type _fun_allowed)
        : fun_val(_fun_val), fun_default(_fun_default), fun_allowed(_fun_allowed) {};
        
        int which()
        {
            return ID_FUN;
        }

        FunctionType             fun_val;
        std::string          fun_default;
        string_allowed_type  fun_allowed;
    };


    typedef Variant < VariantBaseType >  ValueType;

    ConfigOption(std::string, const StringType &, const StringType, string_allowed_type allowed, bool list_me = true);
    ConfigOption(std::string, const StringType &, const StringType = "", bool list_me = true);
    ConfigOption(std::string, const SignedIntType &, const SignedIntType = 0, SignedIntType min = -INT_MAX, SignedIntType max = INT_MAX, SignedIntType step = 1, bool list_me = true);
    ConfigOption(std::string, const UnsignedIntType &, const UnsignedIntType = 0, UnsignedIntType min = 0, UnsignedIntType max = UINT_MAX, UnsignedIntType step = 1, bool list_me = true);
    ConfigOption(std::string, const BooleanType &, const BooleanType = false, bool list_me = true);
    ConfigOption(std::string, FunctionType, std::string defvalue, string_allowed_type allowed, bool list_me = true);
    ConfigOption(std::string, FunctionType, std::string defvalue = "", bool list_me = true);

    ~ConfigOption(void);

    void set(StringType value);

    void set(SignedIntType value);
    void set(UnsignedIntType value);
    void set(BooleanType value);

    std::string    & name(void);
    value_id_type    type(void);

    const char **  values(void);

    void            reset(void);
    void           commit(void);

    bool          list_me(void) { return _list_me; };
    bool           loaded(void) { return _loaded;  };

    void        copy_from(ConfigOption &);

 protected:
    std::string       _my_name;
    ValueType     _value_data;

    bool              _list_me;
    const char **      _values;
    bool               _loaded;
};

struct ConfigOptions
{
    typedef std::vector < std::string >    messages_type;

    ConfigOptions(void): _values(NULL) {};

    typedef std::set < std::string >  string_set;

    typedef std::map  < std::string, ConfigOption >   option_map_type;
    typedef std::pair < std::string, ConfigOption >  option_pair_type;

    typedef std::map  < std::string, std::string >   syn_option_map_type;
    typedef std::pair < std::string, std::string >  syn_option_pair_type;

    bool add(ConfigOption option);

    /* only valid in "process" (for backwards compatibility config files) */
    bool synonym(std::string, std::string);

    template <typename ValueType>
    void set(std::string name, ValueType value)
    {
        option_map_type::iterator iter = _map.find(name);

        if (iter == _map.end())
            throw ConfigProcessFailure(STG(FMT("unknown option: %s") % name));

        (*iter).second.set(value);
    }

    string_set options(void);

    void process(const char *, const char *); /* process option from file */

    void           reset(void);         /* reset loaded opts */
    messages_type commit(void);         /* set defaults */

    const char ** values(const char *); /* option value */
    const char ** values(void);         /* values from options */

    bool          loaded(std::string);  /* return if config was loaded */

    void          copy_from(ConfigOptions &, std::string);

 protected:
    option_map_type::iterator find_option(std::string);

 protected:
    option_map_type      _map;
    syn_option_map_type  _syn_map;

    const char ** _values;
};

#endif /* _CONFIG_OPTIONS_HPP_ */

