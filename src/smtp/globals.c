#include "smtp.h"


///////////////////////////////////
//  GLOBALS
///////////////////////////////////


const command_t COMMANDS[] = {
    { "HELO", on_packet_ehlo },
    { "EHLO", on_packet_ehlo },
    { "MAIL", on_packet_mail },
    { "RCPT", on_packet_rcpt },
    { "DATA", on_packet_data },
    { "QUIT", on_packet_quit },
    { 0 }
};
