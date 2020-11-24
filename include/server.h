#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#ifndef EPOLL_MAX_EVENTS
    #define EPOLL_MAX_EVENTS 16
#endif


///////////////////////////////////
//  STRUCTURE
///////////////////////////////////


typedef struct server_t server_t;
typedef void* (*server_event_accept)(server_t *server, int fd, struct sockaddr_in addr);
typedef void (*server_event_read)(server_t *server, void *client, size_t len);
typedef void (*server_event_close)(server_t *server, void *client);
typedef void (*server_event_destroy)(server_t *server);

struct server_t
{
    void *data;
    int sfd;
    int efd;
    server_event_accept on_accept;
    server_event_read on_read;
    server_event_close on_close;
    server_event_destroy on_destroy;
};


///////////////////////////////////
//  PROTOTYPE
///////////////////////////////////


int server_bind(server_t *server, const char *host, int port);
int server_wait(server_t *server);
void server_close(server_t *server);
void server_close_client(server_t *server, int fd, void *data);
