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

#include <regex.hpp>

#include <string.h>

void Regex::Expression::initialize(void)
{
    unsigned int tmplen = strlen(_expression);

    bool extflag = (_flags & E_EXTENDED);

    for (unsigned int i = 0; i < tmplen; ++i)
    {
        switch (_expression[i])
        {
            case '\\':
                ++i;

                if (!extflag && i < tmplen)
                    if (_expression[i] == '(')
                        ++_subcounter;

                break;

            case '(':
                if (extflag)
                    ++_subcounter;

            default:
                break;
        }
    }

    _errorstate = regcomp(&_comp_regex, _expression, _flags);
}

std::string Regex::Expression::regerror_as_string(void)
{
    unsigned int count = regerror(_errorstate, &_comp_regex, 0, 0) + 1;

    char * msg = new char[count];

    regerror(_errorstate, &_comp_regex, msg, count);

    std::string tmp(msg, count);

    delete[] msg;

    return tmp;
}

void Regex::Match::initialize(void)
{
    if (_expression.valid())
    {
        _subcounter = _expression.subcount() + 1;
        _submatches = new regmatch_t[_subcounter];
        _have_match = (regexec(_expression.repr(), _basestring.c_str(),
            _subcounter, _submatches, _flags) == 0);
    }
}

std::string Regex::Match::replace(std::string rp)
{
    ReplaceVector tmp;
    tmp.push_back(rp);
    return replace(tmp);
}

std::string Regex::Match::replace(Regex::ReplaceVector vec)
{
    _replaced_string = _basestring;

    std::string rp;
    std::string buffer;

    try
    {
        // trick to prevent error when parentesis on expression is not present
        unsigned int start_at = (_subcounter < 2)? 0 : 1;
        for (unsigned int i = start_at;i < _subcounter;i++)
        {
            // trick to prevent access out of range
            rp = vec.at((start_at == 0)?i:i-1);
            buffer = _replaced_string.replace(_submatches[i].rm_so,_submatches[i].rm_eo - _submatches[i].rm_so,rp);
            _replaced_string = buffer;
        }
    }
    catch(std::out_of_range)
    {
        return "";
    }

    return buffer;
}
