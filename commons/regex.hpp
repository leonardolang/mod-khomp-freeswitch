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

#include <sys/types.h>
#include <regex.h>

#include <limits.h>

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include <refcounter.hpp>
#include <noncopyable.hpp>
#include <stdarg.h>

#ifndef _REGEX_HPP_
#define _REGEX_HPP_

struct Regex
{
    enum
    {
        E_EXTENDED         = REG_EXTENDED,
        E_IGNORE_CASE      = REG_ICASE,
        E_NO_SUB_MATCH     = REG_NOSUB,
        E_EXPLICIT_NEWLINE = REG_NEWLINE,
    };

    enum
    {
        M_NO_BEGIN_OF_LINE    = REG_NOTBOL,
        M_NO_END_OF_LINE      = REG_NOTEOL,
    };

    typedef std::vector < std::string >   ReplaceVector;

    struct Expression : public NonCopyable
    {
        Expression(const char * expression, unsigned int flags = 0)
        :  _expression(expression), _subcounter(0),
           _errorstate(INT_MAX), _flags(flags)
        {
            initialize();
        }

        ~Expression()
        {
            if (_errorstate != INT_MAX)
                regfree(&_comp_regex);
        }

        bool               valid(void) { return (_errorstate == 0); }

        unsigned int    subcount(void) { return  _subcounter; }
        const regex_t *     repr(void) { return &_comp_regex; }

        std::string error(void)
        {
            switch (_errorstate)
            {
                case 0:       return "";
                case INT_MAX: return "uninitialized";
                default:      return regerror_as_string();
            }
        }

     private:
        void                initialize(void);
        std::string regerror_as_string(void);

     protected:
        const char   * _expression;

        unsigned int   _subcounter;

        int            _errorstate;
        regex_t        _comp_regex;

        unsigned int   _flags;
    };

    struct Match: NEW_REFCOUNTER(Match)
    {
        Match(const char * basestring, Expression & expression, unsigned int flags = 0)
        : _basestring(basestring), _expression(expression), _subcounter(0), _submatches(0),
           _have_match(false), _flags(flags)
        {
            initialize();
        }

        Match(std::string & basestring, Expression & expression, unsigned int flags = 0)
        : _basestring(basestring), _expression(expression), _subcounter(0), _submatches(0),
          _have_match(false), _flags(flags)
        {
            initialize();
        }

        Match(const Match & o)
        : INC_REFCOUNTER(o, Match),
          _basestring(o._basestring), _expression(o._expression),
          _subcounter(o._subcounter), _submatches(o._submatches),
          _have_match(o._have_match), _flags(o._flags)
        {
        }

        void unreference()
        {
            delete[] _submatches;
        }

        bool matched(void)
        {
            return _have_match;
        }

        bool matched(unsigned int number)
        {
            if (_have_match && number < _subcounter)
                return (_submatches[number].rm_so != -1);

            return false;
        }

        std::string submatch(int number)
        {
            if (!matched(number))
                return "";

            return _basestring.substr(_submatches[number].rm_so,
                _submatches[number].rm_eo - _submatches[number].rm_so);
        }

        /**
        * \brief replaces strings matched by parentesis
        * \param each item of the vector is a parentesis replaced
        * \return string replaced
        * \note The overload method match only one string in parentesis.
        * \author Eduardo Nunes Pereira
        *
        * If fails the empty string is returned.
        */
        std::string replace(ReplaceVector);
        std::string replace(std::string);

        std::string operator[](int number)
        {
            return submatch(number);
        }

        unsigned int get_subcounter()
        {
            return _subcounter;
        }

     private:
        void initialize(void);

     protected:
        std::string    _replaced_string;
        std::string    _basestring;
        Expression   & _expression;

        unsigned int   _subcounter;
        regmatch_t   * _submatches;

        bool           _have_match;
        unsigned int   _flags;
    };
};

#endif /* _REGEX_HPP_ */

