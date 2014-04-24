#include "errors.h"
#include "ssltrans.h"
#include "argparser.h"
#include "log.h"
#include "sysutils.h"
#include "sockutils.h"
#include "messager.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/file.h>
#include <time.h>

#define irc_exit(code) do { retval = code; goto cleanup; } while (0);

sig_atomic_t stop = 0;
const char *_argp_progname = "echoserver";

int main(int argc, char const *argv[])
{
    const char *ca_path, *key_path;
    int sock = -1;
    char buffer[400];
    ssize_t bytes_read;
    char addr[100];

    struct arginfo options [] =
    {
        {'v', "verbose", 0, "verbose/debug output"},
        {'k', "ca-cert", arg_req_param, "CA certificate ", ".pem", CA_PATH},
        {'c', "client-cert", arg_req_param, "Client certificate", ".pem", CA_KEY},
        {0, NULL, 0, "Target host.", "ip/servname", "localhost"},
        {0, NULL, 0, "Target port.", "port/portname", "10000"}

    };

    struct arginfo *verbose_opt = options;
    struct arginfo *ca_cert_opt = options + 1;
    struct arginfo *client_cert_opt = options + 2;
    struct arginfo *host_opt = options + 3;
    struct arginfo *port_opt = options + 4;

    argp_parse(argc, argv, options, sizeof(options) / sizeof(struct arginfo));

    slog_set_output(stderr);

    if (verbose_opt->present)
        slog_set_level(LOG_DEBUG);
    else
        slog_set_level(LOG_NOTICE);

    install_stop_handlers();

    ca_path = ca_cert_opt->param;
    key_path = client_cert_opt->param;


    init_all_ssl(ca_path, key_path, 1);

    slog(LOG_NOTICE, "Cliente de echo empezando.");

    sock = client_connect_to(host_opt->param, port_opt->param, addr, 100, 1);

    if (sock < 0)
    {
        slog_sys(LOG_CRIT, "client_connect_to");
        return EXIT_FAILURE;
    }

    sock_set_block(sock, 1);
    slog(LOG_NOTICE, "Conectado a %s", addr);

    while (1)
    {
        fgets(buffer, 400, stdin);
        buffer[strlen(buffer) - 1] = '\0';

        if(strlen(buffer) == 0)
            continue;

        if (send_message(sock, buffer, strlen(buffer)) != OK)
        {
            slog_sys(LOG_ERR, "send_message");
            continue;
        }

        sock_wait_data(sock, -1);
        bytes_read = rcv_message_staticbuf(sock, buffer, 399);

        if (bytes_read == 0)
        {
            slog(LOG_NOTICE, "Conexión finalizada");
            break;
        }
        else if (bytes_read < 0)
        {
            slog_sys(LOG_ERR, "rcv_message");
        }

        buffer[bytes_read] = '\0';
        printf("-> %s\n", buffer);
    }

    close(sock);

    return EXIT_SUCCESS;
}

