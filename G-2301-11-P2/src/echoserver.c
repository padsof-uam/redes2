#include "errors.h"
#include "ssltrans.h"
#include "argparser.h"
#include "log.h"
#include "sysutils.h"
#include "sockutils.h"
#include "messager.h"

#include <unistd.h>

const char *_argp_progname = "echoserver";

int main(int argc, char const *argv[])
{
    const char *ca_path, *key_path;
    int port = 10000;
    int sock = -1;
    int lsock;
    char *buffer;
    ssize_t bytes_read;

    struct arginfo options [] =
    {
        {'v', "verbose", 0, "verbose/debug output"},
        {'k', "ca-cert", arg_req_param, "CA certificate ", ".pem", 0, 0},
        {'c', "client-cert", arg_req_param, "Client certificate", ".pem", 0, 0 },
        {'p', "port", arg_req_param, "port to listen on (default 10000)"}
    };

    struct arginfo *verbose_opt = options;
    struct arginfo *ca_cert_opt = options + 1;
    struct arginfo *client_cert_opt = options + 2;
    struct arginfo *port_opt = options + 3;

    argp_parse(argc, argv, options, sizeof(options) / sizeof(struct arginfo));

    slog_set_output(stderr);

    if (verbose_opt->present)
        slog_set_level(LOG_DEBUG);
    else
        slog_set_level(LOG_NOTICE);

    install_stop_handlers();

    if (ca_cert_opt->present)
        ca_path = ca_cert_opt->param;
    else
        ca_path = CA_PATH;

    if (client_cert_opt->present)
        key_path = client_cert_opt->param;
    else
        key_path = CA_KEY;

    if (port_opt->present)
        port = atoi(port_opt->param);

    init_all_ssl(ca_path, key_path, 1);

    slog(LOG_NOTICE, "Servidor de echo empezando.");

    lsock = server_open_socket(port, 1, 1);

    if (lsock < 0)
    {
        slog_sys(LOG_CRIT, "server_open_socket");
        return EXIT_FAILURE;
    }

    slog(LOG_NOTICE, "Servidor escuchando en puerto %d", port);

    sock_set_block(lsock, 1);

    while (sock == -1)
    {
        slog(LOG_NOTICE, "Esperando conexión...");
        sock = server_listen_connect(lsock);
        sock_set_block(sock, 1);

        if (sock == -1)
        {
            slog_sys(LOG_ERR, "server_listen_connect");
            continue;
        }
        slog(LOG_NOTICE, "Recibida conexión.");

        while ((sock_wait_data(sock, -1) > 0) && 
            (bytes_read = rcv_message(sock, (void **)&buffer)) > 0)
        {
            send_message(sock, buffer, bytes_read);
            slog(LOG_DEBUG, "recibido mensaje de tamaño %d: %s", bytes_read, buffer);
        }

        if (bytes_read == 0)
        {
            slog(LOG_NOTICE, "Conexión finalizada");
            close(sock);
            sock = -1;
        }
        else if(bytes_read < 0)
            slog_sys(LOG_ERR, "rcv_message");
    }

    if(sock != -1)
        close(sock);
    close(lsock);

    return EXIT_SUCCESS;
}

