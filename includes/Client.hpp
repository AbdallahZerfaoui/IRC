#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Socket.hpp"
#include <string>
#include <vector>
#include <poll.h>
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>

// CHANGED (tobias)
class Client
{
private:
    std::unique_ptr<Socket> _socket;
    std::string read_buffer;
    std::string write_buffer;

public:
    Client() = delete;
    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

	// Move constructor and assignment operator
	Client(Client&&);
	Client& operator=(Client&&);

    Client(std::unique_ptr<Socket> socket);
    ~Client() = default;

    int get_fd() const;

    void read_data();
    void write_data();
    void close();
};

#endif
