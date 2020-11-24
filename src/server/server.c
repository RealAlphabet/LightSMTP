#include "server.h"


///////////////////////////////////
//  BIND AND CLOSE
///////////////////////////////////


int server_bind(server_t *server, const char *host, int port)
{
    struct sockaddr_in addr;
    struct epoll_event event;
    int sfd;
    int efd;

    // Create the socket.
    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return(1);

    // Set SO_REUSE_ADDR.
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)))
        return (1);

    // We set the port and the interface to bind the socket on.
    addr.sin_addr.s_addr = inet_addr(host);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;

    // Bind the socket to the interface.
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        return(1);

    // Listen on the interface.
    if (listen(sfd, SOMAXCONN) == -1)
        return(1);

    // Create epoll to manage multi connections.
    if ((efd = epoll_create1(0)) == -1)
        return (1);

    // Add the server file descriptor to epoll.
    event.events = EPOLLIN;
    event.data.u64 = sfd;

    if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event))
        return (1);

    server->data = server;
    server->sfd = sfd;
    server->efd = efd;
    return (0);
}

void server_close(server_t *server)
{
    // Call the destroy callback.
    if (server->on_destroy)
        server->on_destroy(server);

    // Close the file descriptors.
    close(server->sfd);
    close(server->efd);
}

void server_close_client(server_t *server, int fd, void *data)
{
    // Close the connection.
    close(fd);

    // Call the close callback.
    server->on_close(server, data);
}


///////////////////////////////////
//  HANDLERS
///////////////////////////////////


void server_handle_accept(server_t *server)
{
    struct epoll_event event = { 0 };
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    int fd;

    // Accept the connection.
    if((fd = accept(server->sfd, (struct sockaddr*)&addr, &addr_len)) == -1)
        return;

    // Call the accept callback and retrieve a custom data pointer.
    event.events = EPOLLIN;
    event.data.ptr = server->on_accept(server->data, fd, addr);

    // Add the socket to epoll list.
    epoll_ctl(server->efd, EPOLL_CTL_ADD, fd, &event);
}

void server_handle_read(server_t *server, void *data)
{
    int fd = *(int*)data;
    size_t len = 0;

    // Get number of bytes to read.
    ioctl(fd, FIONREAD, &len);

    if (len) {
        // Call the read callback.
        server->on_read(server->data, data, len);

    } else {
        // Close the connection.
        close(fd);

        // Call the close callback.
        server->on_close(server->data, data);
    }
}


///////////////////////////////////
//  WAITER
///////////////////////////////////


int server_wait(server_t *server)
{
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int count = epoll_wait(server->efd, events, EPOLL_MAX_EVENTS, -1);

    // Check if epoll returned prematurely.
    if (count == -1)
        return (1);

    // Process the events.
    for (int i = 0; i < count; i++) {
        if (server->sfd == events[i].data.fd) {
            // Handle new connection.
            server_handle_accept(server);

        } else {
            // Handle IO events.
            server_handle_read(server, events[i].data.ptr);
        }
    }

    return (0);
}
