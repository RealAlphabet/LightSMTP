#pragma once

#include <mongoc/mongoc.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "server.h"


typedef struct{
    server_t server;
    mongoc_client_t *client;
} smtp_t;

typedef struct{
    int fd;
    uint8_t state;
    char from[48];
    char to[48];
    char *body;
    size_t body_len;
    size_t body_max;
} client_t;

typedef struct
{
    char *name;
    void (*callback)(smtp_t *server, client_t *client, int argc, char **argv);
} command_t;

//  globals.c
extern const command_t COMMANDS[];

//  events.c
void *on_smtp_accept(smtp_t *server, int fd, struct sockaddr_in addr);
void on_smtp_close(smtp_t *server, client_t *client);
void on_smtp_read(smtp_t *server, client_t *client, size_t len);

//  packets.c
void on_packet_ehlo(smtp_t *server, client_t *client, int argc, char **argv);
void on_packet_mail(smtp_t *server, client_t *client, int argc, char **argv);
void on_packet_rcpt(smtp_t *server, client_t *client, int argc, char **argv);
void on_packet_data(smtp_t *server, client_t *client, int argc, char **argv);
void on_packet_quit(smtp_t *server, client_t *client, int argc, char **argv);

//  utils.c
int split_arguments(char *buf, char **argv, int max);
int get_next_power(int i);
