#include <string.h>
#include "smtp.h"


///////////////////////////////////
//  UTILS
///////////////////////////////////


int parse_mail(const char *prefix, int len, const char *str, char *dst)
{
    uint8_t size = 0;
    // Verify argument.
    if (strncmp(prefix, str, len))
        return (1);

    // Move the pointer over argument.
    str += len;

    // @FIX
    // Some servers add a space after "FROM:" (AKA Epic Games not RFC compliant).
    while (*str)
        if (*str++ == '<')
            break;

    // Copy email address to dst.
    while (*str && size < 127) {

        // All is good, we finished reading.
        if (*str == '>') {
            *dst = 0;
            return (0);
        }

        *dst++ = *str++;
        size++;
    }

    return (1);
}


///////////////////////////////////
//  PACKETS
///////////////////////////////////


void on_packet_ehlo(smtp_t *server, client_t *client, int argc, char **argv)
{
    // Update client state.
    client->state = 1;

    // @TODO: Support more extensions and save actual SMTP.
    send(client->fd, "250-smtp.lumzapp.com at your service\r\n250-SIZE 10240000\r\n250-8BITMIME\r\n250-AUTH PLAIN LOGIN\r\n250-ENHANCEDSTATUSCODES\r\n250 SMTPUTF8\r\n", 132, MSG_DONTWAIT);

    // Print debug message.
    puts("[+] Client state = 1 (EHLO)");
}

void on_packet_mail(smtp_t *server, client_t *client, int argc, char **argv)
{
    char *test;

    // Verify connection state.
    switch (client->state) {
        case 0:
            send(client->fd, "503 - EHLO/HELO first.\r\n", 24, MSG_DONTWAIT);
            return;

        default:
            // #
            test = argv[1];
            for (int i = 2; i < argc; i++)
                test = strcat(test, argv[i]);
            // #

            // Verify arguments.
            if (argc < 2 || parse_mail("FROM:", 5, argv[1], client->from)) {
                send(client->fd, "555 - Syntax error.\r\n", 21, MSG_DONTWAIT);
                return;
            }

            // Update client state.
            client->state = 2;

            // Send success message.
            send(client->fd, "250 2.1.0 OK\r\n", 14, MSG_DONTWAIT);

            // Print debug message.
            puts("[+] Client state = 2 (MAIL)");
    }
}

void on_packet_rcpt(smtp_t *server, client_t *client, int argc, char **argv)
{
    char *test;

    switch (client->state) {
        case 0:
            send(client->fd, "503 - EHLO/HELO first.\r\n", 24, MSG_DONTWAIT);
            return;

        case 1:
            send(client->fd, "503 - MAIL first.\r\n", 19, MSG_DONTWAIT);
            return;

        default:
            // #
            test = argv[1];
            for (int i = 2; i < argc; i++)
                test = strcat(test, argv[i]);
            // #

            // Verify arguments.
            if (argc < 2 || parse_mail("TO:", 3, argv[1], client->to)) {
                send(client->fd, "555 - Syntax error.\r\n", 21, MSG_DONTWAIT);
                return;
            }

            // Update client state.
            client->state = 3;

            // Send success message.
            send(client->fd, "250 2.1.0 OK\r\n", 14, MSG_DONTWAIT);

            // Print debug message.
            puts("[+] Client state = 3 (RCPT)");
    }

}

void on_packet_data(smtp_t *server, client_t *client, int argc, char **argv)
{
    switch (client->state) {
        case 0:
            send(client->fd, "503 - EHLO/HELO first.\r\n", 24, MSG_DONTWAIT);
            return;

        case 1:
            send(client->fd, "503 - MAIL first.\r\n", 19, MSG_DONTWAIT);
            return;

        case 2:
            send(client->fd, "503 - RCPT first.\r\n", 19, MSG_DONTWAIT);
            return;

        default:
            // Update client state.
            client->state = 4;

            // Send success message.
            send(client->fd, "354 - Go ahead !\r\n", 18, MSG_DONTWAIT);

            // Print debug message.
            puts("[+] Client state = 4 (DATA)");
    }
}

void on_packet_quit(smtp_t *server, client_t *client, int argc, char **argv)
{
    // Send quit message.
    send(client->fd, "384 - Good bye !\r\n", 18, MSG_DONTWAIT);

    // Close connection.
    server_close_client((server_t*)server, client->fd, client);
}
