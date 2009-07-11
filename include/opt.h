#ifndef _OPT_H_
#define _OPT_H_

#include <string>
#include <map>
#include "switch.h"

struct CSpan {
    std::string _dialplan;
    std::string _context;
    std::string _dialstring;
};

struct Opt
{
    typedef enum
    {
            GFLAG_MY_CODEC_PREFS = (1 << 0)
    }
    GFLAGS;

    static void initialize(void);
    static void obtain(void);
    static void printConfiguration(switch_stream_handle_t*);

protected:

    static void load_configuration(const char *, const char **, bool show_errors = true);
    static void clean_configuration(void);


public:

    typedef std::pair < std::string, CSpan >  span_pair_type;
    
    static bool         _debug;
    static std::string  _dialplan;
    static std::string  _context;
    static std::map < std::string, CSpan > _spans;

};





#endif /* _OPT_H_ */
