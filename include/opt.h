#ifndef _OPT_H_
#define _OPT_H_

#include <string>

struct Opt
{

    typedef enum {
            GFLAG_MY_CODEC_PREFS = (1 << 0)
    } GFLAGS;


    static void initialize(void);
    static void obtain(void);


protected:

    static void load_configuration(const char *, const char **, bool show_errors = true);
    static void clean_configuration(void);


public:
    static bool _debug;
    static std::string _ip;
    static unsigned int _port;
    static std::string _dialplan;
    static std::string _codec_string;
    //static char *_codec_order[]; //[SWITCH_MAX_CODECS]
    //static int _codec_order_last;
    static std::string _codec_rates_string;
    //static char *_codec_rates[]; //[SWITCH_MAX_CODECS]
    //static int _codec_rates_last;



};





#endif /* _OPT_H_ */
