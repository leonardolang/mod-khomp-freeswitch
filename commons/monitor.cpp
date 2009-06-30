#include <monitor.hpp>

monitor::user_printer  monitor::_printer = NULL;
monitor::mode_type     monitor::_mode    = monitor::ONLINE;
monitor::state_type    monitor::_state;
bool                   monitor::_lazyreg;

const char *monitor::finish_string =
    "Fim do buffer de comunicação";            // 0x00

const char *monitor::overflow_string =
    "Estouro do buffer de comunicação";        // 0x01

const char *monitor::est_reg_strings[] =
{
    "Fim da troca de sinalização MFC",            // EST_REG-00 // 0x20
    "Iniciando troca MFC canal entrada",          // EST_REG-01 // 0x21
    "Aguardando condição terminal local",         // EST_REG-02 // 0x22
       "Aguardando retirada sinal",                  // EST_REG-03 // 0x23
       "Aguardando retirada categoria",              // EST_REG-04 // 0x24
       "Aguardando retirada da categoria p/ final",  // EST_REG-05 // 0x25
       "Aguardando sinal para frente",               // EST_REG-06 // 0x26
       "Aguardando identidade",                      // EST_REG-07 // 0x27
       "Aguardando categoria/identidade",            // EST_REG-08 // 0x28
       "Aguardando categoria",                       // EST_REG-09 // 0x29
       "Aguardando condição do terminal remoto",     // EST_REG-10 // 0x2a
       "Aguardando retirada do 'A3'",                // EST_REG-11 // 0x2b
       "Aguardando dígito",                          // EST_REG-12 // 0x2c
       "Aguardando sinal para trás",                 // EST_REG-13 // 0x2d
       "Aguardando retirada do sinal",               // EST_REG-14 // 0x2e
       "Aguardando retirada do 'A5'",                // EST_REG-15 // 0x2f
       "Aguardando retirada do sinal p/ finalizar",  // EST_REG-16 // 0x30
};

const char *monitor::est_lin_strings[] =
{
    "Juntor de saída livre",                                  // EST_LIN-00 // 0x60
    "Juntor de saída bloqueado",                              // EST_LIN-01 // 0x61
    "Aguardando sinal de confirmação de ocupação",            // EST_LIN-02 // 0x62
       "Aguardando sinal de conf. de ocupação p/ desligamento",  // EST_LIN-03 // 0x63
    "Aguardando um IGMFC disponível",                         // EST_LIN-04 // 0x64
       "Aguardando o termino da troca MFC",                      // EST_LIN-05 // 0x65
    "Aguardando o sinal de atendimento",                      // EST_LIN-06 // 0x66
       "Em conversação",                                         // EST_LIN-07 // 0x67
    "Aguardando desligamento local ou sinal reatendimento",   // EST_LIN-08 // 0x68
       "Aguardando sinal de confirmação de desconexão",          // EST_LIN-09 // 0x69
    "Juntor de entrada (ou bidirecional) livre",              // EST_LIN-10 // 0x6a
       "Juntor de entrada bloqueado",                            // EST_LIN-11 // 0x6b
    "Aguardando IGMFC",                                       // EST_LIN-12 // 0x6c
    "Aguardando receber endereço, categoria ou identidade",   // EST_LIN-13 // 0x6d
    "Aguardando termino da troca MFC",                        // EST_LIN-14 // 0x6e
    "Aguardando atendimento local ou desligamento frente",    // EST_LIN-15 // 0x6f
    "Em conversação",                                         // EST_LIN-16 // 0x70
};

const char *monitor::ig_init_strings[] =
{
    "Inicia IGE (prepara p/juntor de entrada)", // 0xB0
    "Inicia IGS (prepara p/juntor de saída)",   // 0xB1
};

const char *monitor::sin_reg_a_strings[] =
{
    "A-00 Retirada do Sinal",                         // A-00
    "A-01 Pedido p/ enviar próximo algarismo",        // A-01
    "A-02 Pedido p/ enviar primeiro algarismo",       // A-02
    "A-03 Receber sinais do grupo B",                 // A-03
    "A-04 Sinal de congestionamento",                 // A-04
    "A-05 Pedido p/ enviar identidade ou categoria",  // A-05
    "A-06 Reservado",                                 // A-06
    "A-07 Pedido p/ enviar penúltimo algarismo",      // A-07
    "A-08 Pedido p/ enviar ante-penúltimo algarismo", // A-08
    "A-09 Pedido p/ enviar último algarismo",         // A-09
    "A-10 Reservado",                                 // A-10
    "A-11 Reservado",                                 // A-11
    "A-12 Reservado",                                 // A-12
    "A-13 Reservado",                                 // A-13
    "A-14 Reservado",                                 // A-14
    "A-15 Reservado"                                  // A-15
};

const char *monitor::sin_reg_b_strings[] =
{
    "B-00 Retirada do Sinal",                        // B-00
    "B-01 Assinante livre com tarifação",            // B-01
    "B-02 Assinante ocupado",                        // B-02
    "B-03 Assinante com número mudado",              // B-03
    "B-04 Sinal de congestionamento" ,               // B-04
    "B-05 Assinante livre sen tarifação",            // B-05
    "B-06 Assinante livre com tarifação e retenção", // B-06
    "B-07 Nível ou número vago",                     // B-07
    "B-08 Assinante fora de serviço",                // B-08
    "B-09 Reservado",                                // B-09
    "B-10 Reservado",                                // B-10
    "B-11 Reservado",                                // B-11
    "B-12 Reservado",                                // B-12
    "B-13 Reservado",                                // B-13
    "B-14 Reservado",                                // B-14
    "B-15 Reservado",                                // B-15
};
    
const char *monitor::sin_reg_1_strings[] =
{
    "I-00 Retirada do Sinal",                    // I-00
    "I-01 Algarismo: '1'",                       // I-01
    "I-02 Algarismo: '2'",                       // I-02
    "I-03 Algarismo: '3'",                       // I-03
    "I-04 Algarismo: '4'",                       // I-04
    "I-05 Algarismo: '5'",                       // I-05
    "I-06 Algarismo: '6'",                       // I-06
    "I-07 Algarismo: '7'",                       // I-07
    "I-08 Algarismo: '8'",                       // I-08
    "I-09 Algarismo: '9'",                       // I-09
    "I-10 Algarismo: '0'",                       // I-10
    "I-11 Inserção de semi supressor de eco",    // I-11
    "I-12 Pedido Recusado",                      // I-12
    "I-13 Acesso a equipamento de teste",        // I-13
    "I-14 Trânsito Internacional",               // I-14
    "I-15 Fim de número"                         // I-15
};
    
const char *monitor::sin_reg_2_strings[] =
{
    "II-00 Retirada do Sinal",                    // II-00
    "II-01 Assinante comum",                      // II-01
    "II-02 Assinante com tarifação especial",     // II-02
    "II-03 Equipamento de manutenção",            // II-03
    "II-04 Telefone público local",               // II-04
    "II-05 Telefonista",                          // II-05
    "II-06 Equipamentos de comunicação de dados", // II-06
    "II-07 Telefone público interurbano",         // II-07
    "II-08 Chamada a cobrar",                     // II-08
    "II-09 Assinante comum internacional",        // II-09
    "II-10 Reservado",                            // II-10
    "II-11 Indicativo de chamada transferida",    // II-11
    "II-12 Reserva",                              // II-12
    "II-13 Reserva",                              // II-13
    "II-14 Reserva",                              // II-14
    "II-15 Reserva"                               // II-15
};

/* mensagens de erro de estados/etc inválidos */

const char *monitor::error_unhandled_string =
    "Erro: evento desconhecido";

const char *monitor::error_direction_string =
    "Erro: direção de canal de sinalização não configurada";

const char *monitor::error_no_group_string =
    "Erro: grupo de MFCs não definido";

stt_code monitor::start(monitor::user_printer printer, monitor::mode_type mode, bool lazyreg)
{
    _printer = printer;
    _mode    = mode;
    _lazyreg = lazyreg;

    /* we do not initialize the map here ( operator[] does that for us, later ) */

    if (_mode == ONLINE && !_lazyreg)
        return k3lRegisterMonitor(NULL, NULL, monitor::internal_mon_handler);
    else
        return ksSuccess;
};

stt_code monitor::stop()
{
    if (_mode == ONLINE || _lazyreg)
        return k3lRegisterMonitor(NULL, NULL, NULL);
    else
        return ksSuccess;
};

/* util macros */
#define    IG_BASE(n) (n - EV_IG_BASE_1)
#define    JS_BASE(n) (n - EV_JS_BASE_1)

#define SIN_REG_RX(n) (n - EV_SIN_REG_RX_BASE)
#define SIN_REG_TX(n) (n - EV_SIN_REG_TX_BASE)

/* ------------- */

stt_code monitor::trace( unsigned int device, unsigned int object, unsigned int obj_count, bool enable )
{
    /* we cant trace when offline! */
    if (_mode == OFFLINE)
        return ksFail;

    /* checks if devices support r2 signaling */
    for (unsigned int i = object; i < obj_count; i++)
    {
        K3L_CHANNEL_CONFIG config;

        if (k3lGetDeviceConfig(device, object + ksoChannel, &config, sizeof(config)) != ksSuccess)
            return ksFail;

        switch (config.Signaling)
        {
            case ksigR2Digital:
            case ksigUserR2Digital:
            case ksigOpenCAS:
            case ksigOpenR2:
                continue;
            default:
                return ksNotAvailable;
        }
    }

    if (enable && _lazyreg)
    {
        KLibraryStatus stt = (KLibraryStatus)k3lRegisterMonitor(NULL, NULL, monitor::internal_mon_handler);

        if (stt != ksSuccess)
            return stt;

        _lazyreg = false;
    }
        
    command cmd;
    config  cfg;
 
    cfg.bits.line_signal     = enable;
    cfg.bits.line_state      = enable;
    cfg.bits.register_signal = enable;
    cfg.bits.register_state  = enable;

    cmd.code = 0x30;
    cmd.config  = cfg.byte;
    cmd.first_obj = object + 1;
    cmd.obj_count = obj_count;

    const int dsp = 0; /* DSP A */

    return k3lSendRawCommand( (int)device, dsp, ( byte * ) &cmd, sizeof( cmd ) );
}

bool monitor::process( const char * buffer, unsigned int dev )
{
    for (unsigned int i = 0;;)
    {
        unsigned char full =  buffer[i++];
        unsigned char bits = (full & 0xf0);

        unsigned int  chan = ((bits != EV_COMM_BASE) ? (buffer[i++] - 1) : 0);
        
        switch (bits)
        {
            case EV_COMM_BASE:
            {
                switch (full)
                {
                    case EV_COMM_BUF_END:
                        // call_printer(finish_string, full, KIND_NONE, dev, 0);
                        return ksSuccess;
                
                    case EV_COMM_BUF_ERR:
                        call_printer(overflow_string, full, KIND_NONE, dev, chan);
                        break;
                        
                    default:
                        call_printer(error_unhandled_string, full, KIND_NONE, dev, chan);
                        break;
                }
                break;
            }
            
            case EV_IG_BASE_1:
            case EV_IG_BASE_2:
                call_printer(est_reg_strings[IG_BASE(full)], full, KIND_NONE, dev, chan);
                break;

            case EV_JS_BASE_1:
            case EV_JS_BASE_2:
                call_printer(est_lin_strings[JS_BASE(full)], full, KIND_NONE, dev, chan);
                break;

            case EV_SIN_REG_RX_BASE:
            {
                channel_state & state = _state[ index_type(dev, chan) ];
                
                mfc_group grp = mfcNoGroup;
                
                switch (state.juntor)
                {
                    case channel_state::JUN_IGE:
                    {
                        grp = ((state.mfcA3 || state.mfcA5) ? mfcGroupII : mfcGroupI);
                        
                        if (state.mfcA5)
                            state.mfcA5 = false;

                        break;
                    }
                            
                    case channel_state::JUN_IGS:
                    {
                        grp = ((state.mfcA3 || state.mfcA5) ? mfcGroupB : mfcGroupA);
                        
                        if (state.mfcA5)
                            state.mfcA5 = false;

                        switch (full)
                        {
                            case EV_SIN_REG_RX_5: { state.mfcA5 = true; break; }
                            case EV_SIN_REG_RX_3: { state.mfcA3 = true; break; }
                            default: break;
                        }

                        break;
                    }
                    
                    case channel_state::JUN_NONE:
                        call_printer(error_direction_string, full, KIND_NONE, dev, chan);
                        break;
                }

                call_printer(group_string(grp, SIN_REG_RX(full)), full, KIND_RX, dev, chan);
                break;
            }
            
            case EV_SIN_REG_TX_BASE:
            {
                channel_state & state = _state[ index_type(dev, chan) ];

                mfc_group grp = mfcNoGroup;

                switch (state.juntor)
                {
                    case channel_state::JUN_IGE:
                    {
                        grp = ((state.mfcA3 || state.mfcA5) ? mfcGroupB : mfcGroupA);
                        
                        if (state.mfcA5)
                            state.mfcA5 = false;

                        switch (full)
                        {
                            case EV_SIN_REG_TX_5: { state.mfcA5 = true; break; }
                            case EV_SIN_REG_TX_3: { state.mfcA3 = true; break; }
                            default: break;
                        }
                        break;
                    }
                            
                    case channel_state::JUN_IGS:
                    {
                        grp = ((state.mfcA3 || state.mfcA5) ? mfcGroupII : mfcGroupI);
                        
                        if (state.mfcA5)
                            state.mfcA5 = false;

                        break;
                    }
                    
                    case channel_state::JUN_NONE:
                        call_printer(error_direction_string, full, KIND_NONE, dev, chan);
                        break;
                }

                call_printer(group_string(grp, SIN_REG_TX(full)), full, KIND_TX, dev, chan);
                break;
            }

            case EV_INIT_BASE:
            {
                channel_state &state = _state[ index_type(dev, chan) ];

                const char *msg = error_unhandled_string;
                                
                switch (full)
                {
                    case EV_INIT_IGE:
                        state.juntor = channel_state::JUN_IGE;
                        state.mfcA3  = false;
                        state.mfcA5  = false;

                        msg = ig_init_strings[full-EV_INIT_BASE];
                        break;
                        
                    case EV_INIT_IGS:
                        state.juntor = channel_state::JUN_IGS;
                        state.mfcA3  = false;
                        state.mfcA5  = false;

                        msg = ig_init_strings[full - EV_INIT_BASE];
                        break;

                    default:
                        break;
                };
                
                call_printer(msg, full, KIND_NONE, dev, chan);
                break;
            }

            case EV_SIN_LIN_RX_BASE:
            {
                struct r2_fios fios(full);
                
                std::string msg =
                    STG(FMT("Recebidos fios R2, a=%d,b=%d,c=%d,d=%d")
                        % fios.a % fios.b % fios.c % fios.d);

                call_printer(msg.c_str(), full, KIND_RX, dev, chan);
                break;
            }

            case EV_SIN_LIN_TX_BASE:
            {
                struct r2_fios fios = (struct r2_fios)full;
                
                std::string msg =
                    STG(FMT("Enviados fios R2, a=%d,b=%d,c=%d,d=%d")
                        % fios.a % fios.b % fios.c % fios.d);
                
                call_printer(msg.c_str(), full, KIND_TX, dev, chan);
                break;
            }

            default:
                call_printer(error_unhandled_string, full, KIND_NONE, dev, chan);
                break;
        }
    }

    return ksFail;
}

/* ------------- */

void monitor::call_printer( const char * msg, unsigned char byte, kind_msg kind, unsigned int dev, unsigned int obj )
{
    const char * str_kind = "";
    
    switch (kind)
    {
        case KIND_TX:   { str_kind = "--(TX)->"; break; }
        case KIND_RX:   { str_kind = "<-(RX)--"; break; }
        case KIND_NONE: { str_kind = "--------"; break; }
    }
    
    std::string tmp = STG(FMT("(%d,%02d) [0x%02x] %s %s")
         % dev % obj % ((unsigned int) byte) % str_kind % msg);

    if (_printer)
    {
        /* just print if we have a printer */
        (*_printer)(tmp.c_str());
    }
}

const char * monitor::group_string( mfc_group mfc, unsigned int index )
{
    switch (mfc)
    {
        case mfcGroupA:
            return sin_reg_a_strings[index];
        case mfcGroupB:
            return sin_reg_b_strings[index];
        case mfcGroupI:
            return sin_reg_1_strings[index];
        case mfcGroupII:
            return sin_reg_2_strings[index];
        default:
            break;
    }

    return error_no_group_string;
}

stt_code monitor::internal_mon_handler( byte *buf, byte dev )
{
    return (monitor::process( (const char *)buf, (unsigned int)dev ) ? ksSuccess : ksFail);
}
