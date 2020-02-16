#pragma once

#include <cstring>
#include <stdio.h>

#ifdef _WIN32

#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>

#elif __linux__

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

const auto& closesocket = close;

using SOCKET = int;
constexpr int INVALID_SOCKET = -1;
constexpr int SOCKET_ERROR = -1;

#endif

#include "exceptions.hpp"


namespace eldstar {

#ifdef _WIN32
class wsa_handler {
    public:
        wsa_handler() {
            if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
                throw load_failure("WSA winsock failed to complete startup.");
            }
        }

        ~wsa_handler() {
            WSACleanup();
        }

    private:
        WSADATA wsa_data;
};
#endif

class tcp_socket {
    public:
        tcp_socket(const char *port = "5617") {
            memset(&hints, 0, sizeof(hints));

            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;

            struct addrinfo *result;

            if (getaddrinfo(NULL, port, &hints, &result) != 0) {
                throw load_failure("Failed to get address info (getaddrinfo) for socket server.");
            }

            listen_socket_id = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

            if (listen_socket_id == INVALID_SOCKET) {
                freeaddrinfo(result);
                closesocket(listen_socket_id);

                throw load_failure("Failed to open a socket server with the retrieved address info.");
            }

            if (bind(listen_socket_id, result->ai_addr, static_cast<int>(result->ai_addrlen)) == SOCKET_ERROR) {
                freeaddrinfo(result);
                closesocket(listen_socket_id);

                throw load_failure("Failed to bind onto the allocated socket server.");
            }

            freeaddrinfo(result);
        }

        ~tcp_socket() {
            closesocket(listen_socket_id);
        }

        SOCKET id() const {
            return listen_socket_id;
        }

    private:
        #ifdef _WIN32
        wsa_handler wsa;
        #endif

        struct addrinfo hints;
        SOCKET listen_socket_id;
};

class tcp_client_session {
    public:
        tcp_client_session(const tcp_socket& s, long timeout = -1) {
            if (listen(s.id(), SOMAXCONN) == SOCKET_ERROR) {
                throw load_failure("Failed to listen to the master socket server.");
            }

            fd_set read_set;
            FD_ZERO(&read_set);
            FD_SET(s.id(), &read_set);

            timeval timeout_struct;
            timeout_struct.tv_sec = timeout;
            timeout_struct.tv_usec = 0;

            if (timeout >= 0 && select(static_cast<int>(s.id()), &read_set, NULL, NULL, &timeout_struct) != 1) {
                throw socket_timeout();
            }

            client_socket_id = accept(s.id(), NULL, NULL);

            if (client_socket_id == INVALID_SOCKET) {
                throw load_failure("Couldn't accept an incoming socket connection.");
            }
        }

        ~tcp_client_session() {
            closesocket(client_socket_id);
        }

        int receive(char* buffer, int length) {
            return recv(client_socket_id, buffer, length, 0);
        }

        int send(const char* buffer, int length) {
            return ::send(client_socket_id, buffer, length, 0);
        }

    private:
        SOCKET client_socket_id;
};

} // eldstar
