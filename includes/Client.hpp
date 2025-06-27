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
// Authentication in the client terminal:
// 
// PASS <password>
// NICK <nickname> // This name must be unique among connected clients
// USER <username> 0 * :realname // The realname can contain spaces and is optional, but if provided, it must be after a colon (:) to allow spaces.
class Client
{
private:
    std::unique_ptr<Socket> _socket;
	std::string input_buffer = ""; // When the server sends data to the client, it is stored here
    std::string output_buffer = ""; // When the client sends data to the server, it is stored here

	// Authentication data
	std::string _nickname = ""; // from NICK
	std::string _username = ""; // from USER
	std::string _realname = "";  // from USER after ':'
	std::string _password = ""; // from PASS
	bool passed_pass = false;
	bool passed_nick = false;
	bool passed_user = false;
	bool passed_realname = false;
    bool authenticated = false;

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
    void close();
	bool get_passed_pass() const;
	bool get_passed_nick() const;
	bool get_passed_user() const;
	bool get_passed_realname() const;

	std::string const &get_nickname() const;
	void set_passed_pass(std::string const &pass);
	void set_passed_nick(std::string const &nick);
	void set_passed_user(std::string const &user);
	void set_passed_realname(std::string const &realname);

	bool is_authenticated() const;
	void set_authenticated();

	// std::string const &get_read_buffer() const;
	// std::string const &get_write_buffer() const;

	void send(std::string const &msg); // Append data to the input_buffer to send to the client
	void write_output_buffer(std::string const &data); // Append data to the output_buffer to send to the server
	std::string extract_output_line();
};

#endif
