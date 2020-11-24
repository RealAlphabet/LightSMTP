#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "server.h"

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
    void (*callback)(server_t *server, client_t *client, int argc, char **argv);
} command_t;

//  globals.c
extern const command_t COMMANDS[];

//  events.c
void *on_smtp_accept(server_t *server, int fd, struct sockaddr_in addr);
void on_smtp_close(server_t *server, client_t *client);
void on_smtp_read(server_t *server, client_t *client, size_t len);

//  packets.c
void on_packet_ehlo(server_t *server, client_t *client, int argc, char **argv);
void on_packet_mail(server_t *server, client_t *client, int argc, char **argv);
void on_packet_rcpt(server_t *server, client_t *client, int argc, char **argv);
void on_packet_data(server_t *server, client_t *client, int argc, char **argv);
void on_packet_quit(server_t *server, client_t *client, int argc, char **argv);

//  utils.c
int split_arguments(char *buf, char **argv, int max);
int get_next_power(int i);
