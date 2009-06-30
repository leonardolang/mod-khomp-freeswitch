#include <string>
#include <map>

#include <format.hpp>

#include <k3l.h>

#ifndef _MONITOR_HPP_
#define _MONITOR_HPP_

    /* -x- */

    #define EV_COMM_BITS                 0x0
    #define EV_COMM_BASE                0x00

    #define EV_COMM_BUF_END             0x00
    #define EV_COMM_BUF_ERR             0x01

    /* -x- */

    #define EV_IG_BASE_1                0x20
    #define EV_IG_BASE_2                0x30

    #define EV_IG_LIVRE                 0x20
    #define    EV_IG_INICIADO           0x21
    #define    EV_IGE_AG_CND_TRM_LOC    0x22
    #define    EV_IGE_AG_RETIR_SINAL    0x23
    #define    EV_IGE_AG_RETIR_CATEG    0x24
    #define    EV_IGE_AG_RET_CAT_FIM    0x25
    #define    EV_IGE_AG_SINAL_FRENT    0x26
    #define    EV_IGE_AG_IDENTIDADE     0x27
    #define    EV_IGE_AG_CATEG_IDENT    0x28
    #define    EV_IGE_AG_CATEGORIA      0x29
    #define    EV_IGS_AG_CND_TRM_REM    0x2A
    #define    EV_IGS_AG_RETIRADA_A3    0x2B
    #define    EV_IGS_AG_DIGITO         0x2C
    #define    EV_IGS_AGSINAL_TRAS      0x2D
    #define    EV_IGS_AG_RETIR_SINAL    0x2E
    #define    EV_IGS_AG_RETIRADA_A5    0x2F
    #define    EV_IGS_AG_RET_SIN_FIM    0x30

    /* -x- */
    
    #define EV_JS_BASE_1                0x60
    #define EV_JS_BASE_2                0x70
     
    #define EV_JS_LIVRE                 0x60
    #define EV_JS_BLOQUEADO             0x61
    #define EV_JS_AG_CONFIRM_OCUP       0x62
    #define EV_JS_AG_CONF_OC_DESL       0x63
    #define EV_JS_AG_IGMFC              0x64
    #define EV_JS_AG_TROCA_MFC          0x65
    #define EV_JS_AG_ATENDIMENTO        0x66
    #define EV_JS_EM_CONVERSACAO        0x67
    #define EV_JS_AG_ATEND_DSL_LC       0x68
    #define EV_JS_AG_CONFIRM_DEWSC      0x69
    #define EV_JE_LIVRE                 0x6A
    #define EV_JE_BLOQUEADO             0x6B
    #define EV_JE_AG_IGMFC              0x6C
    #define EV_JE_AG_TROCA_MFC0         0x6D
    #define EV_JE_AG_TROCA_MFC1         0x6E
    #define EV_JE_AG_ATEND_DSL_FR       0x6F
    #define EV_JE_EM_CONVERSACAO        0x70

    /* -x- */

    #define EV_SIN_REG_RX_BASE         0x80
    
    #define EV_SIN_REG_RX_0            0x80
    #define EV_SIN_REG_RX_1            0x81
    #define EV_SIN_REG_RX_2            0x82
    #define EV_SIN_REG_RX_3            0x83
    #define EV_SIN_REG_RX_4            0x84
    #define EV_SIN_REG_RX_5            0x85
    #define EV_SIN_REG_RX_6            0x86
    #define EV_SIN_REG_RX_7            0x87
    #define EV_SIN_REG_RX_8            0x88
    #define EV_SIN_REG_RX_9            0x89
    #define EV_SIN_REG_RX_A            0x8A
    #define EV_SIN_REG_RX_B            0x8B
    #define EV_SIN_REG_RX_C            0x8C
    #define EV_SIN_REG_RX_D            0x8D
    #define EV_SIN_REG_RX_E            0x8E
    #define EV_SIN_REG_RX_F            0x8F

    /* -x- */

    #define EV_SIN_REG_TX_BASE         0xA0

    #define EV_SIN_REG_TX_0            0xA0
    #define EV_SIN_REG_TX_1            0xA1
    #define EV_SIN_REG_TX_2            0xA2
    #define EV_SIN_REG_TX_3            0xA3
    #define EV_SIN_REG_TX_4            0xA4
    #define EV_SIN_REG_TX_5            0xA5
    #define EV_SIN_REG_TX_6            0xA6
    #define EV_SIN_REG_TX_7            0xA7
    #define EV_SIN_REG_TX_8            0xA8
    #define EV_SIN_REG_TX_9            0xA9
    #define EV_SIN_REG_TX_A            0xAA
    #define EV_SIN_REG_TX_B            0xAB
    #define EV_SIN_REG_TX_C            0xAC
    #define EV_SIN_REG_TX_D            0xAD
    #define EV_SIN_REG_TX_E            0xAE
    #define EV_SIN_REG_TX_F            0xAF

    /* -x- */

    #define EV_INIT_BASE               0xB0
    
    #define EV_INIT_IGE                0xB0
    #define EV_INIT_IGS                0xB1

    /* -x- */
    
    #define EV_SIN_LIN_RX_BASE         0xC0

    #define EV_SIN_LIN_RX_0            0xC0
    #define EV_SIN_LIN_RX_1            0xC1
    #define EV_SIN_LIN_RX_2            0xC2
    #define EV_SIN_LIN_RX_3            0xC3
    #define EV_SIN_LIN_RX_4            0xC4
    #define EV_SIN_LIN_RX_5            0xC5
    #define EV_SIN_LIN_RX_6            0xC6
    #define EV_SIN_LIN_RX_7            0xC7
    #define EV_SIN_LIN_RX_8            0xC8
    #define EV_SIN_LIN_RX_9            0xC9
    #define EV_SIN_LIN_RX_A            0xCA
    #define EV_SIN_LIN_RX_B            0xCB
    #define EV_SIN_LIN_RX_C            0xCC
    #define EV_SIN_LIN_RX_D            0xCD
    #define EV_SIN_LIN_RX_E            0xCE
    #define EV_SIN_LIN_RX_F            0xCF

    /* -x- */

    #define EV_SIN_LIN_TX_BASE         0xE0

    #define EV_SIN_LIN_TX_0            0xE0
    #define EV_SIN_LIN_TX_1            0xE1
    #define EV_SIN_LIN_TX_2            0xE2
    #define EV_SIN_LIN_TX_3            0xE3
    #define EV_SIN_LIN_TX_4            0xE4
    #define EV_SIN_LIN_TX_5            0xE5
    #define EV_SIN_LIN_TX_6            0xE6
    #define EV_SIN_LIN_TX_7            0xE7
    #define EV_SIN_LIN_TX_8            0xE8
    #define EV_SIN_LIN_TX_9            0xE9
    #define EV_SIN_LIN_TX_A            0xEA
    #define EV_SIN_LIN_TX_B            0xEB
    #define EV_SIN_LIN_TX_C            0xEC
    #define EV_SIN_LIN_TX_D            0xED
    #define EV_SIN_LIN_TX_E            0xEE
    #define EV_SIN_LIN_TX_F            0xEF

    /* -x- */
    
struct monitor
{
    struct command 
    { 
        unsigned char code; 
        unsigned char first_obj; 
        unsigned char obj_count; 
        unsigned char config; 
    }; 
 
    struct config_bits
    { 
        unsigned char line_signal:1; 
        unsigned char line_state:1; 
        unsigned char register_signal:1; 
        unsigned char register_state:1; 
        unsigned char reserved:4; 
    }; 
 
    union config
    { 
        config_bits        bits;
        unsigned char    byte;
    };

    typedef enum { OFFLINE, ONLINE } mode_type;

 protected:        
    typedef enum
    {
        mfcNoGroup   = 0x0,
        mfcGroupI    = 0x1,
        mfcGroupII   = 0x2,
        mfcGroupA    = 0x3,
        mfcGroupB    = 0x4,
    }
    mfc_group;

    typedef enum
    {
        KIND_TX   = 0x1,
        KIND_RX   = 0x2,
        KIND_NONE = 0x3,
    }
    kind_msg;

    struct r2_fios
    {
        r2_fios(unsigned char &bits)
        : a(bits & 0x8), b(bits & 0x4), c(bits & 0x2), d(bits & 0x1) {};
        
        bool a, b, c, d;
    };
                            
    struct channel_state
    {
        typedef enum { JUN_NONE, JUN_IGE, JUN_IGS } juntor_direction;

        channel_state(): juntor(JUN_NONE), mfcA3(false), mfcA5(false) {};
        
        juntor_direction   juntor;
        bool               mfcA3, mfcA5;
    };

    typedef std::pair< unsigned int, unsigned int >     index_type;
    typedef std::map< index_type, channel_state >       state_type;

 public:
 
    /* user printer function */    
    typedef void (*user_printer)(const char *);
    
    /* registers monitor handler */
    static stt_code start(user_printer, mode_type mode = ONLINE, bool lazyreg = true);
    
    /* unregisters monitor handler */
    static stt_code stop();
    
    /* enable/disable trace on device, starting at 'object' */
    static stt_code trace(unsigned int device, unsigned int object, unsigned int obj_count, bool enable = true);

    /* process an external buffer directly, used for offline processing */
    static bool process( const char *, unsigned int );

 protected:
 
    static stt_code Kstdcall internal_mon_handler( byte *, byte );

    static void call_printer( const char *, unsigned char, kind_msg, unsigned int, unsigned int );
    static const char * group_string( mfc_group, unsigned int );
    
    static const char *finish_string;
    static const char *overflow_string;
    
    static const char *est_reg_strings[];
    static const char *est_lin_strings[];
    static const char *ig_init_strings[];

    static const char *sin_reg_a_strings[];
    static const char *sin_reg_b_strings[];
    static const char *sin_reg_1_strings[];
    static const char *sin_reg_2_strings[];

    static const char *error_unhandled_string;
    static const char *error_direction_string;
    static const char *error_no_group_string;
    
    static user_printer  _printer;
    static mode_type     _mode;

    static bool          _lazyreg;

    static state_type    _state;

 private:
     monitor();
    ~monitor();
};

#endif /* _MONITOR_HPP_ */
