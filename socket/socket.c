#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <unistd.h> // getopt
#include <string.h> // strncpy
#include <assert.h>
#include <errno.h>

#include "clog.h"

#define IP_SIZE 32
#define BUFF_SIZE 128

#define MODE_CLIENT 0
#define MODE_SERVER 1

#define SOCK_DGRAM_L  0
#define SOCK_STREAM_L 1

#define DELAY 0

struct args
{
    unsigned short mode; // server--1 or client--0
    unsigned short port;
    unsigned short sock; // stream--1 or graph--0
    unsigned int   delay;
    char *ip;
};

const static char *default_ip = "127.0.0.1";

static struct args args_s = {
    .mode = MODE_SERVER,
    .port = 8888,
    .sock = SOCK_STREAM_L,
    .delay = DELAY,
    .ip   = NULL,
};

static void parse_args(int argc, char *argv[])
{
    int opt;
    char *ip = NULL;
    while ((opt = getopt(argc, argv, "cstup:a:d:")) != -1)
    {
        switch (opt){
        case 'c':
            args_s.mode = MODE_CLIENT;
            break;
        case 's':
            args_s.mode = MODE_SERVER;
            break;
        case 't':
            args_s.sock = SOCK_STREAM_L;
            break;
        case 'u':
            args_s.sock = SOCK_DGRAM_L;
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
        case 'd':
            args_s.delay = atoi(optarg);
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

static void create_stream_client()
{
    int sock = stream_socket();
    struct sockaddr_in client;
    memset(&client, 0, sizeof(struct sockaddr_in));
    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(args_s.ip);
    client.sin_port = htons(args_s.port);

    int ret = connect(sock, (struct sockaddr*)&client, sizeof(client));
    if (ret < 0)
        lerror_exit("connect failed, %s", strerror(errno));

    char buff[BUFF_SIZE] = {0};
    recv(sock, buff, BUFF_SIZE, 0);
    linfo("recv: %s", buff);

    memset(buff, 0, BUFF_SIZE);
    strncpy(buff, "from create_stream_client", BUFF_SIZE);
    linfo("send: %s", buff);
    send(sock, buff, BUFF_SIZE, 0);

    if (args_s.delay > 0)
    {
        sleep(args_s.delay);
    }

    linfo("begin to close %d", sock);
    close(sock);
}

static void create_dgram_client()
{
    linfo("not implemented");
    return;
}

static void create_client()
{
    assert(args_s.mode == MODE_CLIENT);

    int sock = args_s.sock;
    if (sock == SOCK_STREAM_L)
    {
        create_stream_client();
    }
    else {
        create_dgram_client();
    }
}

static void create_stream_server()
{
    int sock = stream_socket();

    struct sockaddr_in server, client;
    memset(&server, 0, sizeof(struct sockaddr_in));
    memset(&client, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(args_s.ip);
    server.sin_port = htons(args_s.port);

    bind(sock, (struct sockaddr*)&server, sizeof(server));
    listen(sock, 5);

    socklen_t len = sizeof(client);
    int sock_accept = accept(sock, (struct sockaddr*)&client, &len);

    linfo("client: %s %d", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    char buff[BUFF_SIZE] = {0};
    strncpy(buff, "from server", BUFF_SIZE);
    send(sock_accept, buff, BUFF_SIZE, 0);
    memset(buff, 0, BUFF_SIZE);
    int ret = 0;

    while ((ret = recv(sock_accept, buff, BUFF_SIZE, 0)) != 0)
    {
        linfo("recv: %s", buff);
        memset(buff, 0, BUFF_SIZE);
    }
    linfo("client close");

    close(sock_accept);
    close(sock);
}

static void create_dgram_server()
{
    linfo("not implemented");
    return;
}

static void create_server()
{
    assert(args_s.mode == MODE_SERVER);

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
    unsigned short mode = args_s.mode;
    if (mode == MODE_CLIENT)
    {
        create_client();
    }
    else if (mode == MODE_SERVER)
    {
        create_server();
    }
    else {
        lerror_exit("unknown mode %u", mode);
    }
}

static void clean_up()
{
    if (args_s.ip != default_ip)
    {
        free(args_s.ip);
    }
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    create_socket();

    clean_up();

    return 0;
}