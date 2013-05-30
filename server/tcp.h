#ifndef _TCP_H
#define _TCP_H

/* TCP socket structure. Used mainly to hold buffers from incomplete
 * recv()s. */
struct tcp_socket {
    int fd;
    struct sockaddr_in clisa;
    socklen_t clilen;
    struct event *evt;

    unsigned char *buf;
    size_t pktsize;
    size_t len;
    struct req_info req;
    size_t excess;

    void *appdata;
    void (*on_close)(void *appdata);
};

int tcp_init(const char* ip, int port);
void tcp_close(int fd);
void tcp_newconnection(int fd, short event, void *arg);
void tcp_socket_free(struct tcp_socket *tcpsock);

#endif

