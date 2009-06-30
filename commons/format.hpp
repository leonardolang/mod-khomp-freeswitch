#ifndef _FORMAT_H_
#define _FORMAT_H_

#include <cstring>
#include <cstdlib>
#include <typeinfo>
#include <stdexcept>
#include <string>
#include <queue>
#include <iostream>

/* macros used for shortening lines and making the code clearer */
#define STG(x) (x).str()
#define FMT(x) Format(x)

struct Format
{
    struct InvalidFormat
    {
        InvalidFormat(std::string msg) : _msg(msg) {}

        std::string _msg;
    };

    Format(const char * format_string, bool raise_exception = false);
    
    std::string str(void);

    ////////////////////////////////////////////////////////////

 protected:

    enum Type
    {
        T_ANYTHING = 1,

        T_SIGNED_SHORT,
        T_SIGNED_SHORT_SHORT,
        T_SIGNED_INT,
        T_SIGNED_LONG,
        T_SIGNED_LONG_LONG,

        T_UNSIGNED_SHORT,
        T_UNSIGNED_SHORT_SHORT,
        T_UNSIGNED_INT,
        T_UNSIGNED_LONG,
        T_UNSIGNED_LONG_LONG,

        T_FLOAT,
        T_CHAR,

        T_POINTER,
        T_STRING,

        T_LITERAL
    };

    struct Argument
    {
        Argument(std::string fmts, Type type)
        : _fmts(fmts), _type(type) {};

        Type          type(void) { return _type; }
        std::string & fmts(void) { return _fmts; }

     protected:
        std::string _fmts;
        Type        _type;
    };

    typedef std::queue < Argument > ArgumentQueue;

    ////////////////////////////////////////////////////////////

 public:

    template < typename V >
    Format & operator%( V value )
    {
        if (!validity_check())
            return *this;

        try
        {
            Argument & top = next_argument();

            char temp[32];

            if (!generic_verify(value, top.type()))
            {
                std::string msg;

                msg += "type mismatch: got type '";
                msg += typeid(value).name();
                msg += "' in format '";
                msg += top.fmts();
                msg += "'";

                mark_invalid(msg);
                return *this;
            }

            snprintf(temp, sizeof(temp), top.fmts().c_str(), value);
            _result += temp;

            pop_argument();
        }
        catch (NoArgumentLeft & e)
        {
            std::string msg;

            msg += "too many arguments passed for format '";
            msg += _format;
            msg += "'";

            mark_invalid(msg);
        }

        raise_check();
        return *this;
    }

    template < typename V >
    Format & operator%( V * value )
    {
        if (!validity_check())
            return *this;

        try
        {
            Argument & top = next_argument();

            switch (top.type())
            {
                case T_POINTER:
                {
                    char temp[16];
                    snprintf(temp, sizeof(temp), top.fmts().c_str(), value);
                    _result += temp;
                    break;
                }

                case T_STRING:
                {
                    if ((typeid(const char)          == typeid(V)) ||
                        (typeid(char)                == typeid(V)) ||
                        (typeid(const unsigned char) == typeid(V)) ||
                        (typeid(unsigned char)       == typeid(V)))
                    {
                        int len = strlen((const char*)value)+1;

                        std::cerr << "len: " << len << std::endl;

                        char * temp = new char[len];

                        snprintf(temp, len, top.fmts().c_str(), value);
                        _result += temp;

                        delete[] temp;
                    }
                    else
                    {
                        std::string msg;

                        msg += "type mismatch: got type '";
                        msg += typeid(value).name();
                        msg += "' in string format";

                        mark_invalid(msg);
                    }
                    break;
                }

                default:
                {
                    std::string msg;

                    msg += "type mismatch: got pointer/string type in format '";
                    msg += top.fmts();
                    msg += "'";

                    mark_invalid(msg);
                    break;
                }
            }

            pop_argument();
        }
        catch (NoArgumentLeft & e)
        {
            std::string msg;

            msg += "too many arguments passed for format '";
            msg += _format;
            msg += "'";

            mark_invalid(msg);
        }

        raise_check();
        return *this;
    }

    Format & operator%( std::string & value )
    {
        if (!validity_check())
            return *this;

        try
        {
            Argument & top = next_argument();

            if (top.type() == T_STRING)
            {
                int len = value.length()+1;

                char * temp = new char[len];

                snprintf(temp, len, top.fmts().c_str(), value.c_str());
                _result += temp;

                delete[] temp;
            }
            else
            {
                std::string msg;

                msg += "type mismatch: got string type in format '";
                msg += top.fmts();
                msg += "'";

                mark_invalid(msg);
            }

            pop_argument();
        }
        catch (NoArgumentLeft & e)
        {
            std::string msg;

            msg += "too many arguments passed for format '";
            msg += _format;
            msg += "'";

            mark_invalid(msg);
        }

        raise_check();
        return *this;
    }

 protected:

    template < typename V >
    bool generic_verify( V value, Type type )
    {
        switch (type)
        {
            case T_SIGNED_SHORT_SHORT:
                return (typeid(V) == typeid(char));
            case T_SIGNED_SHORT:
                return (typeid(V) == typeid(short int)) || (typeid(V) == typeid(short));
            case T_SIGNED_INT:
                return typeid(V) == typeid(int);
            case T_SIGNED_LONG:
                return (typeid(V) == typeid(long int)) || (typeid(V) == typeid(long));
            case T_SIGNED_LONG_LONG:
                return (typeid(V) == typeid(long long int)) || (typeid(V) == typeid(long long));

            case T_UNSIGNED_SHORT_SHORT:
                return (typeid(V) == typeid(unsigned char));
            case T_UNSIGNED_SHORT:
                return (typeid(V) == typeid(unsigned short int)) || (typeid(V) == typeid(unsigned short));
            case T_UNSIGNED_INT:
                return typeid(V) == typeid(unsigned int);
            case T_UNSIGNED_LONG:
                return (typeid(V) == typeid(unsigned long int)) || (typeid(V) == typeid(unsigned long));
            case T_UNSIGNED_LONG_LONG:
                return (typeid(V) == typeid(unsigned long long int)) || (typeid(V) == typeid(unsigned long long));

            case T_FLOAT:
                return (typeid(V) == typeid(float)) || (typeid(V) == typeid(double));

            case T_CHAR:
                return (typeid(V) == typeid(char)) || (typeid(V) == typeid(unsigned char));

            case T_POINTER:
            case T_STRING:
                return false;

            case T_ANYTHING:
                return true;

            case T_LITERAL:
                return false;
        }
    };

    void mark_invalid(std::string &);

    bool validity_check(void);
    void raise_check(void);

    struct NoArgumentLeft {};

    Argument & next_argument(void);

    void pop_argument(void);
    void push_argument(std::string & data, Type type);

 private:
    const char * _format;

    bool         _valid;
    bool         _raise;

    std::string   _result;
    ArgumentQueue _args;
};

#endif /* _FORMAT_H_ */

