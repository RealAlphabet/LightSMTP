#include "smtp.h"


///////////////////////////////////
//  UTILS
///////////////////////////////////


int parse_mail(const char *prefix, int len, const char *str, char *dst)
{
    uint8_t size = 0;

    // Verify argument.
    if (strncasecmp(prefix, str, len))
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
    send(client->fd, "250-smtp.shellcode.sh at your service\r\n250-SIZE 10240000\r\n250-8BITMIME\r\n250-AUTH PLAIN LOGIN\r\n250-ENHANCEDSTATUSCODES\r\n250 SMTPUTF8\r\n", 133, MSG_DONTWAIT);

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
                if (argc >= 2)
                    fprintf(stderr, "[C][MAIL] %s\n", argv[1]);
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

// #
const char* MAILBOXES[] = {
    "48600000.xyz",
    "animeotv.fr",
    "animeovf.fr",
    "blastrush.net",
    "cloudkit.fr",
    "cornflex.co",
    "lumz.app",
    "lumzapp.com",
    "oauth3.me",
    "pepo.world",
    "phietoutenue.fr",
    "shellcode.club",
    "shellcode.sh",
    "shellcode.tools",
    "shiroi.fr",
    "sommet.app",
    "streamflixvf.fr",
    "thepixels.fr",
    NULL,
};

int check_mailbox(const char *domain)
{
    for (int i = 0; MAILBOXES[i]; i++)
        if (strcasecmp(MAILBOXES[i], domain) == 0)
            return (0);
    return (1);
}
// #

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
                if (argc >= 2)
                    fprintf(stderr, "[C][RECPT] %s -> %s\n", client->from, argv[1]);
                send(client->fd, "555 - Syntax error.\r\n", 21, MSG_DONTWAIT);
                return;
            }

            // Check if the mailbox exists.
            if (check_mailbox(strchr(client->to, '@') + 1)) {
                fprintf(stderr, "[M] %s -> %s\n", client->from, client->to);
                send(client->fd, "550 - The email account that you tried to reach does not exist.\r\n", 69, MSG_DONTWAIT);
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
