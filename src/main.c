#include <stdio.h>
#include <signal.h>

#include "server.h"
#include "smtp.h"


///////////////////////////////////
//  RUNNER
///////////////////////////////////


static volatile sig_atomic_t running = 1;

void intHandler() {
    running = 0;
}


///////////////////////////////////
//  SMTP
///////////////////////////////////


int main(int argc ,char **argv)
{
    smtp_t smtp = { 0 };
    server_t *server = &smtp.server;

    // Catch CTRL + C.
    signal(SIGINT, intHandler);

    // Verify parameters.
    if (argc < 2) {
        printf("Usage : %s [MongoDB URI]\n", argv[0]);
        return (1);
    }

    // Init MongoDB and database.
    mongoc_init();

    if ((smtp.client = mongoc_client_new(argv[1])) == NULL) {
        puts("[Error] Unable to connect to the database.");
        return (1);
    }

    // Register events.
    server->data = &smtp;
    server->on_accept = (server_event_accept)on_smtp_accept;
    server->on_close = (server_event_close)on_smtp_close;
    server->on_read = (server_event_read)on_smtp_read;

    // Bind SMTP port.
    if (server_bind(server, "0.0.0.0", 25)) {
        puts("[Error] Unable to listen on port 25.");
        return (1);
    }

    puts("[SMTP] Server listening on port 25.");

    // Wait for events.
    while (running)
        server_wait(server);

    // Close server and free ressources.
    server_close(server);
    mongoc_client_destroy(smtp.client);
    mongoc_cleanup();

    puts("[SMTP] Server closed.");

    return (0);
}
