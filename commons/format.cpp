#include "format.hpp"
//#include <iostream>

Format::Format(const char * format_string, bool raise_exception)
: _format(format_string), _valid(true), _raise(raise_exception)
{
    std::string txt;

    const char * ptr = format_string;

    while (*ptr != '\0')
    {
        if (*ptr != '%')
        {
            txt += *ptr;
            ++ptr;
            continue;
        } 

        const char * ptr2 = ptr+1;

        if (*ptr2 == '%') 
        {
            txt += *ptr;
            txt += *ptr2;

            ptr += 2;
            continue;
        }

        if (!txt.empty())
            push_argument(txt, T_LITERAL);

        std::string arg(1, *ptr);

        ++ptr;

        bool finished = false;

        short long_count = 0;
        short short_count = 0;

        while(*ptr != '\0' && !finished) 
        {
            switch (*ptr)
            {
                case ' ':
                    // uncomplete format with ' ', make it a literal.
                    arg += *ptr;
                    push_argument(arg, T_LITERAL);
                    finished = true;
                    break;
                
                case '%':
                    // uncomplete format with '%', make it a literal and start a new format.
                    push_argument(arg, T_LITERAL);
                    arg += *ptr;
                    break;

                case 'h':
                    short_count = std::min<short>(short_count+1, 2);
                    long_count = 0;
                    arg += *ptr;
                    break;

                case 'l':
                    long_count = std::min<short>(long_count+1, 2);
                    short_count = 0;
                    arg += *ptr;
                    break;

                case 'd':
                case 'i':
                    arg += *ptr;
                    switch (long_count - short_count)
                    {
                        case  2:
                            push_argument(arg, T_SIGNED_LONG_LONG);
                            break;
                        case  1:
                            push_argument(arg, T_SIGNED_LONG);
                            break;
                        case  0:
                            push_argument(arg, T_SIGNED_INT);
                            break;
                        case -1:
                            push_argument(arg, T_SIGNED_SHORT);
                            break;
                        case -2:
                            push_argument(arg, T_SIGNED_SHORT_SHORT);
                            break;
                        default:
                            break;
                    }
                    finished = true;
                    break;

                case 'o':
                case 'u':
                case 'x':
                case 'X':
                    arg += *ptr;
                    switch (long_count - short_count)
                    {
                        case  2:
                            push_argument(arg, T_UNSIGNED_LONG_LONG);
                            break;
                        case  1:
                            push_argument(arg, T_UNSIGNED_LONG);
                            break;
                        case  0:
                            push_argument(arg, T_UNSIGNED_INT);
                            break;
                        case -1:
                            push_argument(arg, T_UNSIGNED_SHORT);
                            break;
                        case -2:
                            push_argument(arg, T_UNSIGNED_SHORT_SHORT);
                            break;
                        default:
                            break;
                    }
                    finished = true;
                    break;

                case 'e':
                case 'E':
                case 'f':
                case 'F':
                case 'g':
                case 'G':
                case 'a':
                case 'A':
                    arg += *ptr;
                    push_argument(arg, T_FLOAT);
                    finished = true;
                    break;

                case 'c':
                    arg += *ptr;
                    push_argument(arg, T_CHAR);
                    finished = true;
                    break;

                case 's':
                    arg += *ptr;
                    push_argument(arg, T_STRING);
                    finished = true;
                    break;

                case 'p':
                case 'C':
                case 'S':
                case 'n':
                case 'm':
                    arg += *ptr;
                    push_argument(arg, T_ANYTHING);
                    finished = true;
                    break;

                default:
                    arg += *ptr;
                    break;
            }
        
            ++ptr;
        }

        if (!arg.empty())
            push_argument(arg, T_LITERAL);
    }
    
    if (!txt.empty())
        push_argument(txt, T_LITERAL);
}

void Format::mark_invalid(std::string & msg)
{
    if (_valid)
    {
        _valid = false;

        std::string finalmsg;

        finalmsg += "** INVALID FORMAT: ";
        finalmsg += msg;
        finalmsg += " **";

        _result = finalmsg;
    }
}

void Format::raise_check(void)
{
    if (!_valid && _raise)
        throw InvalidFormat(_result);
}

bool Format::validity_check(void)
{
    raise_check();

    return _valid;
}

Format::Argument & Format::next_argument(void)
{
//    std::cerr << "size: " << _args.size() << std::endl;

    while (true)
    {
//        std::cerr << "loop size: " << _args.size() << std::endl;

        if (_args.empty())
            throw NoArgumentLeft();

        Argument & top = _args.front();

        if (top.type() == T_LITERAL)
        {
//            std::cerr << "top type == LITERAL, looping..." << std::endl;
            _result += top.fmts();
            _args.pop();
        }
        else
        {
//            std::cerr << "top type: " << top.type() << std::endl;
            return top;
        }
    }
}

void Format::pop_argument(void)
{
    _args.pop();
}

void Format::push_argument(std::string & data, Format::Type type)
{
//    std::cerr << "pushing type: " << type << std::endl;

    _args.push(Argument(data, type));
    data.clear();
}

std::string Format::str()
{
    if (!validity_check())
        return _result;

    try
    {
        Argument & top = next_argument();

        std::string msg;

        msg += "too few arguments passed for format '";
        msg += _format;
        msg += "'";

        mark_invalid(msg);

        return _result;
    }
    catch (NoArgumentLeft & e)
    {
        return _result;
    }
}


