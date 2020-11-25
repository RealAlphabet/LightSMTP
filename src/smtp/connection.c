#include "smtp.h"


///////////////////////////////////
//  READERS
///////////////////////////////////


void read_mail(smtp_t *server, client_t *client, size_t len)
{
    size_t act_len;
    size_t new_len;

    // Assign actual and new length.
    act_len = client->body_len;
    new_len =  act_len + len;

    // Realloc the buffer if we need to append more data.
    if (client->body_max < new_len) {
        client->body_max = get_next_power(new_len);
        client->body = realloc(client->body, client->body_max);
    }

    // Append the message to the actual body buffer.
    read(client->fd, &client->body[act_len], len);

    // Assign the new len.
    client->body[new_len] = 0;
    client->body_len = new_len;

    // Check the end of mail.
    if (strncmp(&client->body[new_len - 5], "\r\n.\r\n", 5) == 0) {
        // Send success message.
        send(client->fd, "250 OK\r\n", 8, MSG_DONTWAIT);

        // Show mail in the console.
        printf("[M]\tFrom:\t%s\n", client->from);
        printf("\tTo:\t%s\n", client->to);

        // #
        mongoc_collection_t *collection = mongoc_client_get_collection(server->client, "lumz-dev", "mails");
        bson_t *doc = bson_new();
        bson_error_t error;

        BSON_APPEND_UTF8(doc, "from", client->from);
        BSON_APPEND_UTF8(doc, "to", client->to);
        BSON_APPEND_UTF8(doc, "content", client->body);

        mongoc_collection_insert_one(collection, doc, NULL, NULL, &error);
        mongoc_collection_destroy(collection);
        bson_destroy(doc);
        // #

        // Free message.
        free(client->body);

        // Reset client state to RCPT and reset fields.
        client->state = 3;
        client->body = NULL;
        client->body_max = 0;
        client->body_len = 0;
    }
}

void read_command(smtp_t *server, client_t *client, size_t len)
{
    char buf[256];
    char *argv[8];
    int argc;

    // Read data from the connection.
    len = read(client->fd, buf, 256);

    // Set the null terminated string.
    buf[len - 1] = 0;

    // Split arguments.
    argc = split_arguments(buf, argv, 8);

    // Ignore empty message.
    if (argc == 0)
        return;

    // Call the command callback.
    for (int i = 0; COMMANDS[i].name; i++)
        if (strcmp(COMMANDS[i].name, argv[0]) == 0) {
            COMMANDS[i].callback(server, client, argc, argv);
            return;
        }

    // The command isn't a valid command.
    send(client->fd, "502 Unrecognized command.\r\n", 27, MSG_DONTWAIT);
}


///////////////////////////////////
//  CONNECTION
///////////////////////////////////


void *on_smtp_accept(smtp_t *server, int fd, struct sockaddr_in addr)
{
    client_t *client = calloc(1, sizeof(client_t));

    // Set file descriptor.
    client->fd = fd;

    // Send welcome message.
    send(fd, "220 smtp.lumzapp.com ESMTP\r\n", 28, MSG_DONTWAIT);

    // Print debug message.
    puts("[+] Client connected.");

    return (client);
}

void on_smtp_close(smtp_t *server, client_t *client)
{
    // Free the body message.
    if (client->body)
        free(client->body);

    // Free the custom data pointer.
    free(client);

    // Print debug message.
    puts("[-] Client disconnected.");
}

void on_smtp_read(smtp_t *server, client_t *client, size_t len)
{
    // The client is currently sending an email.
    if (client->state == 4) {
        read_mail(server, client, len);

    } else {
        read_command(server, client, len);
    }
}
