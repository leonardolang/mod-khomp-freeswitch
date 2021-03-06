/*
    KHOMP generic endpoint/channel library.
    Copyright (C) 2007-2009 Khomp Ind. & Com.

  The contents of this file are subject to the Mozilla Public License Version 1.1
  (the "License"); you may not use this file except in compliance with the
  License. You may obtain a copy of the License at http://www.mozilla.org/MPL/

  Software distributed under the License is distributed on an "AS IS" basis,
  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
  the specific language governing rights and limitations under the License.

  Alternatively, the contents of this file may be used under the terms of the
  "GNU Lesser General Public License 2.1" license (the “LGPL" License), in which
  case the provisions of "LGPL License" are applicable instead of those above.

  If you wish to allow use of your version of this file only under the terms of
  the LGPL License and not to allow others to use your version of this file under
  the MPL, indicate your decision by deleting the provisions above and replace them
  with the notice and other provisions required by the LGPL License. If you do not
  delete the provisions above, a recipient may use your version of this file under
  either the MPL or the LGPL License.

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
    along with this library; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

#ifndef _FORMAT_H_
#define _FORMAT_H_

#include <cstring>
#include <cstdlib>
#include <typeinfo>
#include <stdexcept>
#include <string>
#include <queue>
#include <iostream>
#include <stdio.h>

#ifdef WIN32 // WINDOWS
#include <KHostSystem.h>
#endif


/* macros used for shortening lines and making the code clearer */
#define STG(x) (x).str()
#define FMT(x) Format(x)

struct Format
{
    static const unsigned int strings_base_length = 64;
    static const unsigned int generic_base_length = 64;

    struct InvalidFormat
    {
        InvalidFormat(std::string msg) : _msg(msg) {}

        std::string _msg;
    };

    explicit Format(const char  * format_string, bool raise_exception = false);
    explicit Format(std::string   format_string, bool raise_exception = false);

    void initialize(const char *);

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

            char temp[generic_base_length];

            if (!generic_verify(value, top.type()))
            {
                std::string msg;

                msg += "type mismatch: got type '";
                msg += typeid(value).name();
                msg += "' in format '";
                msg += top.fmts();
                msg += "' (";
                msg += _format;
                msg += ")";

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
                    char temp[generic_base_length];
                    snprintf(temp, sizeof(temp), top.fmts().c_str(), value);
                    _result += temp;
                    break;
                }

                case T_STRING:
                {
                    if ((typeid(const char)          == typeid(V)) ||
                        (typeid(char)                == typeid(V)) ||
                        (typeid(const unsigned char) == typeid(V)) ||
                        (typeid(unsigned char)       == typeid(V)) ||
                        (typeid(const void)          == typeid(V)) ||
                        (typeid(void)                == typeid(V)))
                    {
                        int len = strlen((const char*)value)+strings_base_length+1;

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
                        msg += "' in string format (";
                        msg += _format;
                        msg += ")";

                        mark_invalid(msg);
                    }
                    break;
                }

                default:
                {
                    std::string msg;

                    msg += "type mismatch: got pointer/string type in format '";
                    msg += top.fmts();
                    msg += "' (";
                    msg += _format;
                    msg += ")";

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

/*
    Format & operator%( std::string   value )
    {
        return operator%(value);
    }
*/

    Format & operator%( const std::string value )
    {
        if (!validity_check())
            return *this;

        try
        {
            Argument & top = next_argument();

            if (top.type() == T_STRING)
            {
                int len = value.length()+strings_base_length+1;

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
                msg += "' (";
                msg += _format;
                msg += ")";

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
    bool number_verify_signed_short( V value )
    {
        return
            (typeid(V) == typeid(short int) ||
             typeid(V) == typeid(short) ||
             typeid(V) == typeid(const short int) ||
             typeid(V) == typeid(const short) ||
             typeid(V) == typeid(volatile short int) ||
             typeid(V) == typeid(volatile short));
    }

    template < typename V >
    bool number_verify_unsigned_short( V value )
    {
        return
            (typeid(V) == typeid(unsigned short int) ||
             typeid(V) == typeid(unsigned short) ||
             typeid(V) == typeid(const unsigned short int) ||
             typeid(V) == typeid(const unsigned short) ||
             typeid(V) == typeid(volatile unsigned short int) ||
             typeid(V) == typeid(volatile unsigned short));
    }

    template < typename V >
    bool number_verify_signed_long( V value )
    {
        return
            (typeid(V) == typeid(long int) ||
             typeid(V) == typeid(long) ||
             typeid(V) == typeid(const long int) ||
             typeid(V) == typeid(const long) ||
             typeid(V) == typeid(volatile long int) ||
             typeid(V) == typeid(volatile long));
    }

    template < typename V >
    bool number_verify_unsigned_long( V value )
    {
        return
            (typeid(V) == typeid(unsigned long int) ||
             typeid(V) == typeid(unsigned long) ||
             typeid(V) == typeid(const unsigned long int) ||
             typeid(V) == typeid(const unsigned long) ||
             typeid(V) == typeid(volatile unsigned long int) ||
             typeid(V) == typeid(volatile unsigned long));
    }

    template < typename V >
    bool number_verify_signed_long_long( V value )
    {
        return
            (typeid(V) == typeid(long long int) ||
             typeid(V) == typeid(long long) ||
             typeid(V) == typeid(const long long int) ||
             typeid(V) == typeid(const long long) ||
             typeid(V) == typeid(volatile long long) ||
             typeid(V) == typeid(volatile long long int));
    }

    template < typename V >
    bool number_verify_unsigned_long_long( V value )
    {
        return
            (typeid(V) == typeid(unsigned long long int) ||
             typeid(V) == typeid(unsigned long long) ||
             typeid(V) == typeid(const unsigned long long int) ||
             typeid(V) == typeid(const unsigned long long) ||
             typeid(V) == typeid(volatile unsigned long long) ||
             typeid(V) == typeid(volatile unsigned long long int));
    }

    template < typename V >
    bool number_verify_signed_int( V value )
    {
        return
            (sizeof(V) <= 4 ||
             typeid(V) == typeid(int) ||
             typeid(V) == typeid(const int) ||
             typeid(V) == typeid(volatile int));
    }

    template < typename V >
    bool number_verify_unsigned_int( V value )
    {
        return
            (typeid(V) == typeid(unsigned int) ||
             typeid(V) == typeid(const unsigned int) ||
             typeid(V) == typeid(volatile unsigned int));
    }

    template < typename V >
    bool generic_verify( V value, Type type )
    {
        switch (type)
        {
            /* EXCEPTION: consider unsigned int an valid input. */
            case T_SIGNED_INT:
                return
                    (number_verify_signed_int(value)    ||
                     number_verify_unsigned_int(value)  ||
                     number_verify_signed_long(value)   ||
                     number_verify_unsigned_long(value) ||
                     number_verify_signed_short(value)  ||
                     number_verify_unsigned_short(value));

            case T_SIGNED_SHORT_SHORT:
                return (typeid(V) == typeid(char) || typeid(V) == typeid(const char));

            case T_SIGNED_SHORT:
                return number_verify_signed_short(value);

            case T_SIGNED_LONG:
                return number_verify_signed_long(value);

            case T_SIGNED_LONG_LONG:
                return number_verify_signed_long_long(value);

            case T_UNSIGNED_SHORT_SHORT:
                return (typeid(V) == typeid(unsigned char) || typeid(V) == typeid(unsigned char));

            case T_UNSIGNED_SHORT:
                return number_verify_unsigned_short(value);

            case T_UNSIGNED_INT:
                return
                    (number_verify_unsigned_int(value) ||
                     number_verify_unsigned_long(value) ||
                     number_verify_unsigned_short(value));

            case T_UNSIGNED_LONG:
                return number_verify_unsigned_long(value);

            case T_UNSIGNED_LONG_LONG:
                return number_verify_unsigned_long_long(value);

            case T_FLOAT:
                return (typeid(V) == typeid(float)) || (typeid(V) == typeid(double) ||
                    typeid(V) == typeid(const float)) || (typeid(V) == typeid(const double));

            case T_CHAR:
                return (typeid(V) == typeid(char)) || (typeid(V) == typeid(unsigned char) ||
                    typeid(V) == typeid(const char)) || (typeid(V) == typeid(const unsigned char));

            case T_POINTER:
            case T_STRING:
                return false;

            case T_ANYTHING:
                return true;

            case T_LITERAL:
                return false;
        }

        return false;
    };

    void mark_invalid(std::string &);

    bool validity_check(void);
    void raise_check(void);

    struct NoArgumentLeft {};

    Argument & next_argument(void);

    void pop_argument(void);
    void push_argument(std::string & data, Type type);

 private:
    std::string   _format;

    bool          _valid;
    bool          _raise;

    std::string   _result;
    ArgumentQueue _args;
};

#endif /* _FORMAT_H_ */

