#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>

#include <unistd.h> // getopt
#include <string.h> // strncpy
#include <assert.h>
#include <errno.h>

#include "clog.h"

#define IP_SIZE 32
#define BUFF_SIZE 128

#define MODE_SELECT 0
#define MODE_POLL   1
#define MODE_EPOLL  2

#define SOCK_DGRAM_L  0
#define SOCK_STREAM_L 1

#define MAX_EVENT 10

struct args
{
    unsigned short mode; // select--1 or poll--2 or epoll--3
    unsigned short port;
    unsigned short sock; // stream--1 or graph--0
    char *ip;
};

const static char *default_ip = "127.0.0.1";

static struct args args_s = {
    .mode = MODE_SERVER,
    .port = 8888,
    .sock = SOCK_STREAM_L,
    .ip   = NULL,
};

static void parse_args(int argc, char *argv[])
{
    int opt;
    int val;
    char *ip = NULL;
    while ((opt = getopt(argc, argv, "mtup:a:")) != -1)
    {
        switch (opt){
        case 'm':
            val = atoi(optarg);
            if (val != MODE_SELECT ||
                val != MODE_POLL ||
                val != MODE_EPOLL)
                lerror_exit("unknown mode %d", val);
            args_s.mode = val;
            break;
        case 't':
            args_s.sock = SOCK_STREAM_L;
            break;
        case 'u':
            args_s.mode = SOCK_DGRAM_L;
            break;
        case 'p':
            args_s.port = atoi(optarg);
            break;
        case 'a':
            ip = malloc(IP_SIZE);
            if (ip == NULL)
                lerror_exit("malloc");
            strncpy(ip, optarg, IP_SIZE);
            args_s.ip = ip;
            break;
        default:
            lerror_exit("unknown opt %c, %d", opt, opt);
        }
    }

    if (args_s.ip == NULL) {
        args_s.ip = default_ip;
    }
}

static int stream_socket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        lerror_exit("stream_socket failed");

    return sock;
}

static int dgram_socket()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
        lerror_exit("dgram_socket failed");

    return sock;
}

static int set_noblock(int fd)
{
    int flags, ret = 0;
    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        lerror_exit("fcntl %s", strerror(errno));

    flags = flags | O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
        lerror_exit("fcntl %s", strerror(errno));

    return 0;
}

static void create_stream_server()
{
    int ret = 0;
    int nfd = 0;
    int i = 0;
    int listen_sock, epoll_fd, accept_sock;
    listen_sock = stream_socket();

    struct sockaddr_in server, client;
    memset(&server, 0, sizeof(struct sockaddr_in));
    memset(&client, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(args_s.ip);
    server.sin_port = htons(args_s.port);

    bind(listen_sock, (struct sockaddr*)&server, sizeof(server));
    set_noblock(listen_sock);
    listen(listen_sock, 5);

    epoll_fd = epoll_create(10);
    if (epoll_fd == -1)
        lerror_exit("epoll_create %s", strerror(errno));

    struct epoll_event ev, events[MAX_EVENT];
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_sock, &ev);
    if (ret == -1)
        lerror_exit("epoll_ctl %s", strerror(errno));

    for(;;)
    {
        nfd = epoll_wait(epoll_fd, events, MAX_EVENT, -1);
        if (nfd == -1)
            lerror_exit("epoll_wait %s", strerror(errno));

        char buff[BUFF_SIZE] = {0};
        for(i = 0; i < nfd; ++i)
        {
            // get new connection
            if (events[i].data.fd == listen_sock)
            {
                socklen_t len = sizeof(client);
                accept_sock = accept(listen_sock, (struct sockaddr*)&client, &len);
                linfo("client: %s %d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

                char buff[BUFF_SIZE] = {0};
                strncpy(buff, "from server", BUFF_SIZE);
                send(accept_sock, buff, BUFF_SIZE, 0);
                memset(buff, 0, BUFF_SIZE);

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = accept_sock;
                ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, accept_sock, &ev);
                if (ret == -1)
                    lerror("epoll_ctl %s", strerror(errno));
            }
            // read
            else if (events[i].events & EPOLLIN)
            {
                while ((ret = recv(events[i].data.fd, buff, BUFF_SIZE, 0)) != 0)
                {
                    if (ret == -1)
                    {
                        if (errno == EAGAIN)
                            break;
                        lerror("recv %d %s", events[i].data.fd, strerror(error));
                        break;
                    }
                    if (ret == 0)
                    {
                        linfo("close %d", events[i].data.fd);
                        ev.data.fd = events[i].data.fd;
                        ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &ev);
                        if (ret == -1)
                            lerror("epoll_ctl EPOLL_CTL_DEL %s", strerror(errno));
                        
                        close(events[i].data.fd);
                        break;
                    }
                    else
                    {
                        linfo("recv %d %s", events[i].data.fd, buff);
                    }
                }
            }
        }
    }

    close(epoll_fd);
}

static void create_dgram_server()
{
    linfo("not implemented");
    return;
}

static void create_server()
{
    int sock = args_s.sock;
    if (sock == SOCK_STREAM_L)
    {
        create_stream_server();
    }
    else {
        create_dgram_server();
    }
}

static void create_socket()
{
    create_server();
}


int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    create_socket();
    return 0;
}