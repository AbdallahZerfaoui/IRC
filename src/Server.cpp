#include "../includes/Server.hpp"
#include "ParsedMessage.hpp"

bool Server::_signal_received = false;

const std::unordered_map<std::string, Server::CommandHandler> Server::handlers = {
	{"PASS", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.parse_pass(fd, msg);
    }},
	{"NICK", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.parse_nick(fd, msg);
    }},
	{"USER", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.parse_user(fd, msg);
    }},
    {"PRIVMSG", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_privmsg(fd, msg);
    }},
	{"PART", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_part(fd, msg);
    }},
	{"JOIN", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_join(fd, msg);
    }},
	{"HELP", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_help(fd, msg);
    }},
	{"CHANNELS", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_channels(fd, msg);
    }},
	{"QUIT", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_quit(fd, msg);
    }},
	{"PING", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_ping(fd, msg);
    }},
	{"MODE", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_mode(fd, msg);
    }},
	{"KICK", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_kick(fd, msg);
    }},
	{"INVITE", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_invite(fd, msg);
    }},
	{"TOPIC", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_topic(fd, msg);
    }},
};

// Helper functions
bool Server::is_duplicate_nickname(const std::string& nickname)
{
	// Check if the nickname is already taken by another client
	for (const auto& client : _clients)
	{
		if (client.second.get_nickname() == nickname)
		{
			return true;
		}
	}
	return false;
}

std::vector<std::string> Server::split(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}

bool Server::valid_inputs(int port, const std::string& password)
{
	if (port <= 0 || port > MAX_PORT_NBR) {
		std::cerr << "Error: Invalid port number." << std::endl;
		return false;
	}
	if (password.empty()) {
		std::cerr << "Error: Empty password provided." << std::endl;
		return false;
	}
	return true;
}

// a helper function that generate a sockaddr_in structure, fill it and returns it
sockaddr_in Server::create_sockaddr_in(int port)
{
	sockaddr_in server_addr;
	std::memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;         // IPv4. If you were using IPv6, you'd use AF_INET6 and the sockaddr_in6 structure
	server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
	server_addr.sin_port = htons(port);      // Convert port to network byte order
	//TODO: why htons? Is it necessary?
	return server_addr;
}

// Socket Server::get_listening_socket() const
// {
// 	return _listening_socket;
// }

// pollfd Server::create_pollfd()
// {
// 	pollfd listen_pfd;
// 	// Socket listening_socket = get_listening_socket(); // Get the listening socket
// 	listen_pfd.fd = _listening_socket.get_fd(); // The file descriptor to monitor
// 	listen_pfd.events = POLLIN; // POLLIN for read events
// 	listen_pfd.revents = 0;     // Initialize revents to 0
// 	return listen_pfd;
// }

// Constructor: Sets up the server
Server::Server(int port, const std::string& password)
	: _listening_socket(), // Initialize the listening socket (calls Socket::Socket())
	_hostname(""),
	_port(port),
	_password(password)
{
	char hostname_buffer[256];
	if (gethostname(hostname_buffer, sizeof(hostname_buffer)) != 0)
	{
		throw std::runtime_error(std::string("Failed to get hostname: ") + std::strerror(errno));
	}
	_hostname = hostname_buffer;
	if (!valid_inputs(port, password))
		return;
	// Setup the server address structure
	sockaddr_in server_addr = create_sockaddr_in(port);
	// std::memset(&server_addr, 0, sizeof(server_addr));
	// server_addr.sin_family = AF_INET;         // IPv4. 
	// server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
	// 										//The server will accept connections coming to any of them on the specified port
	// server_addr.sin_port = htons(_port);      // Convert port to network byte order
	

	// Bind the socket
	// This is like officially claiming the address and port number for your server.
	// bind function returns 0 on success, -1 on error
	if (bind(_listening_socket.get_fd(), (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
	{
		throw std::runtime_error(std::string("Socket bind failed: ") + std::strerror(errno));
	}
	std::cout << "Socket bound to port " << _port << std::endl;

	// Start listening
	// When your server is busy processing one connection, new incoming connection requests from other clients don't get immediately rejected. 
	// Instead, the operating system's TCP/IP stack queues them up to a certain limit. 
	// The backlog argument suggests to the system how many pending connections it should queue.
	// Once this queue is full, new connection attempts might be rejected or time out. 
	// A value of 10 is a common, reasonable starting point for many servers.
	// Very high-traffic servers might use larger values.
	if (listen(_listening_socket.get_fd(), BACKLOG) < 0) //TODO: is 10 a good value for backlog?
	{
		throw std::runtime_error(std::string("Socket listen failed: ") + std::strerror(errno));
	}
	std::cout << "Server listening on port " << _port << std::endl;

	// Add the listening socket to the pollfd vector
	// For Block 1, this is the only FD we monitor initially
	// listen_pfd.fd = _listening_socket.get_fd(); -> The file descriptor to monitor
	// listen_pfd.events = POLLIN; // We are interested in read events (new connections)
	// Initialize revents to 0
	_pollfds.push_back({_listening_socket.get_fd(), POLLIN, 0});
	std::cout << GREEN << "Server initialized and listening." << RESET << std::endl;
}

// Destructor (basic cleanup, although RAII handles most sockets)
Server::~Server()
{
	// The Socket destructor handles _listening_socket
	// In later blocks, you'd iterate _clients and _channels here for cleanup
	std::cout << "Server shutting down." << std::endl;
}

void Server::handle_signal(int signum)
{
	// Handle the signal (e.g., SIGINT, SIGTERM)
	std::cout << RED << "Signal " << signum << " received. Shutting down server." << RESET << std::endl;
	_signal_received = true; // Set the flag to indicate a signal was received
}

/*
 * This function sets up the handlers for SIGINT (Ctrl+C) and SIGQUIT (Ctrl+\).
 */
void Server::setup_signal_handlers()
{
	struct sigaction sa;
	std::memset(&sa, 0, sizeof(sa));

	// Set our handle_signal function as the handler
	sa.sa_handler = Server::handle_signal;

	sigemptyset(&sa.sa_mask); // Initialize sa_mask to an empty set (no signals blocked)

	sa.sa_flags = 0;

	// Register the handler for SIGINT (Ctrl+C)
	if (sigaction(SIGINT, &sa, NULL) == -1)
	{
		std::cerr << RED << "Error: Could not set up SIGINT handler: " << std::strerror(errno) << RESET << std::endl;
		exit(EXIT_FAILURE);
	}
	// Register the handler for SIGQUIT (Ctrl+\)
	if (sigaction(SIGQUIT, &sa, NULL) == -1)
	{
		std::cerr << RED << "Error: Could not set up SIGQUIT handler: " << std::strerror(errno) << RESET << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << GREEN << "Signal handlers for SIGINT and SIGQUIT set up." << RESET << std::endl;
}

void Server::handle_new_connection()
{
	// Accept a new connection
    std::unique_ptr<Socket> client_socket = _listening_socket.accept();
    if (!client_socket)
    {
        std::cerr << "Error accepting new connection: " << std::strerror(errno) << std::endl;
        return ;
    }
    int client_fd = client_socket->get_fd();

	// Create a new Client with the accepted socket and store the client in the clients map
	// Client(std::move(client_socket)): Creates a temporary Client object that takes ownsership of the socket
	// _client.emplace(...): Inserts the client in the map and therefore the client is accessible even after the function returns
	_clients.emplace(client_fd, Client(std::move(client_socket)));
	std::cout << "New connection accepted on FD " << client_fd << std::endl;

    // std::cout << "Was it inserted? " << (a.second ? "Yes" : "No") << std::endl;
	// Add the new client socket to the pollfd vector
	// We are interested in read events (client data) -> POLLIN
	// Initialize revents to 0
	_pollfds.push_back({client_fd, POLLIN, 0});
	try
	{	
		send_reply(client_fd, 704, { _clients.at(client_fd).get_nickname(), "*" }, "*** Available Commands ***");
		send_reply(client_fd, 705, { _clients.at(client_fd).get_nickname(), "*" }, "PASS <password>");
		send_reply(client_fd, 705, { _clients.at(client_fd).get_nickname(), "*" }, "NICK <nickname>");
		send_reply(client_fd, 705, { _clients.at(client_fd).get_nickname(), "*" }, "USER <username> 0 * :realname\n");
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error sending message: " << e.what() << std::endl;
		return ;
	}
	std::cout << GREEN << "New client added to poll list." << RESET << std::endl;
}

void Server::handle_disconnection(size_t& index)
{
	// Handle disconnection of a client
	std::cout << "Client on FD " << _pollfds[index].fd << " disconnected." << std::endl;

    //ADDED (tobias): Remove the client from the _clients map
    // _clients.erase(client_fd);
	_clients.erase(_pollfds[index].fd);

	// Remove the client socket from the pollfd vector
	close(_pollfds[index].fd);
	_pollfds.erase(_pollfds.begin() + index); // Remove from pollfd vector
	--index;
	std::cout << GREEN << "Client removed from poll list." << RESET << std::endl;
}

void Server::send_reply(int fd, int code, const std::vector<std::string>& params, const std::string& msg)
{
	std::string text = ':' + _hostname + ' ' + std::to_string(code) + ' ';
    for (size_t i = 0; i < params.size(); ++i)
	{
        if (i)
			text += ' ';
		text += params[i];
    }
    text += " :" + msg + "\r\n";
	_clients.at(fd).send(text);
}

int Server::parse_pass(int fd, const ParsedMessage& msg)
{
	for (const auto& param : msg.params)
	{
		std::cout << param << " ";
	}
	std::cout << std::endl;

	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (client.get_passed_pass())
	{
		try
		{
			send_reply(fd, 462, { nickname, "PASS" }, "You may not reregister");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
		return 0;
	}

	if (msg.params.size() != 1 || msg.params[0].empty())
	{
		try
		{
			send_reply(fd, 461, { nickname, "PASS" }, "Not enough parameters");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
		return 0;
	}

	if (msg.params[0] != this->_password)
	{
		try
		{
			send_reply(fd, 464, { nickname, "PASS" }, "Password incorrect");
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
		return 0;
	}

	client.set_passed_pass(msg.params[0]);
	return 0;
}

int Server::parse_nick(int fd, const ParsedMessage& msg)
{
	for (const auto& param : msg.params)
	{
		std::cout << param << " ";
	}
	std::cout << std::endl;

	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
    {
		try
		{
			send_reply(fd, 431, { nickname, "NICK" }, "No nickname given");
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
        return 0;
    }
	std::string nick = msg.params[0];
    if (!nick.empty() && nick[0] == ':')
        nick.erase(0, 1);

	if (![](const std::string &s)
		{ return !s.empty() &&
				 std::all_of(s.begin(), s.end(),
							 [](unsigned char c)
							 { return std::isalnum(c); }); })
	{
		try
		{
			send_reply(fd, 432, { nickname, "NICK" }, "Erroneous nickname");
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
		return 0;
	}

	if (is_duplicate_nickname(nick))
    {
		try
		{
			send_reply(fd, 433, { nickname, "NICK" }, "Nickname is already in use");
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
        return 0;
    }
	std::string old = client.get_nickname();
    client.set_passed_nick(nick);
	if (!old.empty() && old != "anonymous" && old != nick)
    {
        std::string text = old + " is now known as " + nick;
		broadcast_to_all(text, fd);
    }
    std::cout << GREEN << "Client FD " << fd << " set nickname to " << nick << ".\n" << RESET;
    return (0);
}

void Server::broadcast_to_all(const std::string& message, int sender_fd)
{
	for (const auto& client : _clients)
	{
		if (client.first != sender_fd)
		{
			std::string nickname = client.second.get_nickname();
			try
			{
				send_reply(client.first, 462, { nickname }, message);
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error sending message: " << e.what() << '\n';
			}
		}
	}
}

int Server::parse_user(int fd, const ParsedMessage& msg)
{
	for (const auto& param : msg.params)
	{
		std::cout << param << " ";
	}
	std::cout << std::endl;

	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();
	if (client.get_passed_user())
    {
        try
		{
            send_reply(fd, 462, { nickname, "USER" }, "You may not reregister");
        }
		catch (const std::exception& e)
		{
            std::cerr << "Error sending message: " << e.what() << '\n';
        }
        return 0;
    }
    if (msg.params.size() < 4)
    {
        try
		{
            send_reply(fd, 461, { nickname, "USER" }, "Not enough parameters");
        }
		catch (const std::exception& e)
		{
            std::cerr << "Error sending message: " << e.what() << '\n';
        }
        return 0;
    }
	const std::string& username = msg.params[0];
    const std::string& hostname = msg.params[1];
    const std::string& servername = msg.params[2];
    std::string  realname = msg.params[3];
	if (!realname.empty() && realname[0] == ':')
		realname.erase(0, 1);

	if (username.empty() ||
		hostname != "0" ||
		servername != "*" ||
		username.find_first_of(" \t\r\n\v\f") != std::string::npos ||
		!std::all_of(username.begin(), username.end(),
					 [](unsigned char c)
					 { return std::isalnum(c); }))
	{
		try
		{
			send_reply(fd, 461, { nickname, "USER" }, "Invalid USER format. Use: USER <username> 0 * :realname");
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error sending message: " << e.what() << '\n';
		}
		return 0;
	}

	client.set_passed_user(username);
    client.set_passed_realname(realname);
    std::cout << GREEN << "Client FD " << fd << " set user to " << username << " with real name: " << realname << ".\n" << RESET;
    return (0);
}

int Server::handle_mode(int fd, const ParsedMessage& msg)
{
	(void)msg;
	(void)fd;
	// Client &client = _clients.at(fd);
	// std::string nickname = client.get_nickname();

	// if (msg.params.empty())
	// {
	// 	send_reply(fd, 461, { nickname, "MODE" }, "Not enough parameters");
	// 	return 0;
	// }
	// std::string param = msg.params[0];

	// if (param != "o" && )
	// {

	// }
	return 0;
}

int Server::handle_kick(int fd, const ParsedMessage& msg)
{
	(void)msg;
	(void)fd;
	return 0;
}

int Server::handle_invite(int fd, const ParsedMessage& msg)
{
	(void)msg;
	(void)fd;
	return 0;
}

int Server::handle_topic(int fd, const ParsedMessage& msg)
{
	(void)msg;
	(void)fd;
	return 0;
}

int Server::handle_help(int fd, const ParsedMessage &msg)
{
	(void)msg;
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	send_reply(fd, 704, {nickname, "*"}, "*** Available HELP topics ***");
	send_reply(fd, 705, {nickname, "*"}, "HELP                                                     :show this list");
	send_reply(fd, 705, {nickname, "*"}, "CHANNELS                                                 :list channels you are in");
	send_reply(fd, 705, {nickname, "*"}, "JOIN <#chan1,#chan2,...> <optional:key1,key2,...>        :join/create channel");
	send_reply(fd, 705, {nickname, "*"}, "PART <#chan1,#chan2,...> <optional:leaving_message>      :leave channel");
	send_reply(fd, 705, {nickname, "*"}, "PRIVMSG <target1,target2,...> <text>                     :send a message");
	send_reply(fd, 705, {nickname, "*"}, "QUIT                                                     :disconnect");
	send_reply(fd, 706, {nickname, "*"}, "*** End of HELP ***\n");
	return 0;
}

int Server::handle_channels(int fd, const ParsedMessage& msg)
{
	(void)msg;
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	try
	{
		std::string list;
		for (const auto& channel : _channels)
		{
			if (channel.second.get_members().count(fd))
			{
				list + '#' + channel.first + ' ';
			}
		}
		if (list.empty())
		{
			list = "None";
		}
		send_reply(fd, 705, { nickname, "CHANNELS" }, list);

	}
	catch (const std::exception& e)
	{
		std::cerr << "Error sending message: " << e.what() << std::endl;
		return 0;
	}
	return 0;
}

int Server::handle_join(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty() || msg.params.size() > 2)
	{
		send_reply(fd, 461, { nickname, "JOIN" }, "Wrong number of parameters");
		return 0;
	}

	std::vector<std::string> chans = split(msg.params[0], ',');
	std::vector<std::string> keys = (msg.params.size() > 1) ? split(msg.params[1], ',') : std::vector<std::string>();

	std::string chan;
	for (size_t i = 0; i < chans.size(); ++i)
	{
		if (chans[i].empty() || chans[i][0] != '#')
		{
			send_reply(fd, 476, { nickname, "JOIN" }, "Bad channel name");
			continue;
		}
		chan = chans[i].substr(1); // Remove the '#' character
		std::string key = (i < keys.size()) ? keys[i] : "";

		// Add the channel to the channels map, if it doesn't exist
		if (!_channels.count(chan))
			_channels.emplace(chan, Channel(chan, _clients));

		// If the channel requires a key and the key is not provided or incorrect
		if (_channels.at(chan).requires_key() && (key.empty() || key != _channels.at(chan).get_channel_key()))
		{
			send_reply(fd, 475, { nickname, chans[i] }, "Cannot join, bad key");
			continue;
		}

		// Add the client to the channel
		_channels.at(chan).add_client(fd);
		try
		{
			send_reply(fd, 476, { nickname }, "You have joined the channel " + chan);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
			return 0;
		}
		// Notify other clients in the channel (forward a message to all other clients in the channel
		std::string message = ':' + _clients.at(fd).get_nickname() + "@host PRIVMSG #" + _channels.at(chan).get_name() + " :" + " has joined the channel" + "\r\n";
		_channels.at(chan).broadcast_message(message, fd);

		if (_channels.at(chan).get_members().size() == 1)
		{
			// Make the client an operator if they are the first to join the channel
			_channels.at(chan).add_operator(fd);
			try
			{
				send_reply(fd, 705, { nickname, "JOIN", "#" + chan }, "You are now an operator of the channel");
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error sending message: " << e.what() << std::endl;
				return 0;
			}
			// Notify other clients in the channel that the client is now an operator
			std::string message = ':' + _hostname + "MODE #" + _channels.at(chan).get_name() + " +o" + client.get_nickname() + "\r\n";
			_channels.at(chan).broadcast_message(message, -1);
		}
	}
	return 0;
}

int Server::handle_part(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

    if (msg.params.empty() || msg.params.size() > 2)
    {
        send_reply(fd, 461, { nickname, "PART" }, "Wrong number of parameters");
        return 0;
    }

    std::vector<std::string> chans = split(msg.params[0], ',');
	std::string reason = (msg.params.size() == 2) ? msg.params[1] : "Leaving the channel";
    if (!reason.empty() && reason[0] == ':')
        reason.erase(0,1);
	
	for (size_t i = 0; i < chans.size(); ++i)
	{
		if (chans[i].empty() || chans[i][0] != '#')
        {
            send_reply(fd, 476, { nickname, "PART" }, "Bad channel mask");
            continue;
        }
		chans[i].erase(0, 1); // Remove the '#' character

		auto it = _channels.find(chans[i]);
		if (it == _channels.end())
		{
			send_reply(fd, 403, { nickname, "PART", "#" + chans[i] }, "No such channel");
			continue ;
		}

		if (!it->second.remove_client(fd))
		{
			send_reply(fd, 442, { nickname, "PART", "#" + chans[i] }, "You're not on that channel");
			continue ;
		}
		std::string message = ':' + _clients.at(fd).get_nickname() + "@host PRIVMSG #" + _channels.at(chans[i]).get_name() + " :" + reason + "\r\n";
		_channels.at(chans[i]).broadcast_message(message, fd);

		if (it->second.get_members().empty())
			_channels.erase(it);
	}
	return 0;
}

int Server::find_fd_by_nickname(std::string const &nickname) const
{
	for (const auto& client : _clients)
	{
		if (client.second.get_nickname() == nickname)
			return client.first;
	}
	return -1;
}

int Server::handle_privmsg(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();
	if (msg.params.size() < 2)
	{
		send_reply(fd, 411, { nickname, "PRIVMSG" }, "No recipient given");
		return 0;
	}

	std::vector<std::string> targets = split(msg.params[0], ',');
    std::string text = msg.params[1];
    if (!text.empty() && text[0] == ':')
        text.erase(0, 1);

	
	for (size_t i = 0; i < targets.size(); ++i)
    {
        if (!targets[i].empty() && targets[i][0] == '#')
        {
            std::string chan = targets[i].substr(1);
            auto it = _channels.find(chan);
            if (it == _channels.end())
            {
                send_reply(fd, 403, { nickname, targets[i] }, "No such channel");
                continue;
            }

            Channel& ch = it->second;
            if (!ch.has_member(fd))
            {
                send_reply(fd, 404, { nickname, targets[i] }, "Cannot send to channel");
                continue;
            }

			std::string message1 = ':' + _clients.at(fd).get_nickname() + "@host PRIVMSG #" + _channels.at(chan).get_name() + " :" + text + "\r\n";
            ch.broadcast_message(message1, fd);
            continue;
        }

		int fdtg = find_fd_by_nickname(targets[i]);
		if (fdtg == -1)
		{
			send_reply(fd, 401, { nickname, "PRIVMSG", targets[i] }, "No such nickname");
			continue ;
		}

		std::string message = ':' + client.get_nickname() + "@host PRIVMSG " + targets[i] + " :" + text + "\r\n";
		client.send(message);
	}
	return 0;
}

int Server::handle_quit(int fd, const ParsedMessage& msg)
{
	(void)fd;
	(void)msg;
	return -1;
}

int Server::handle_client_command(size_t &index, int client_fd, const ParsedMessage& parsedmsg)
{
	if (parsedmsg.command.empty())
	{
		return 0;
	}

	Client& client = _clients.at(client_fd);
	std::string nickname = client.get_nickname();

	if (parsedmsg.command != "PASS" && !client.get_passed_pass())
    {
        send_reply(client_fd, 451, { nickname }, "You have not registered");
        return 0;
    }

	if (!client.is_authenticated() &&
		parsedmsg.command != "PASS" &&
		parsedmsg.command != "NICK" &&
		parsedmsg.command != "USER")
	{
		send_reply(client_fd, 451, { nickname }, "You have not registered");
		return 0;
	}

	auto it = handlers.find(parsedmsg.command);
    if (it == handlers.end())
    {
        send_reply(client_fd, 421, { nickname, parsedmsg.command }, "Unknown command");
        return 0;
    }
    if (it->second(*this, client_fd, parsedmsg) == -1)
    {
        handle_disconnection(index);
        return 1;
    }

	if (!client.is_authenticated() &&
		client.get_passed_pass() &&
		client.get_passed_nick() &&
		client.get_passed_user())
	{
		client.set_authenticated();

		send_reply(client_fd, 001, { nickname },
				   "Welcome to ft_irc, " + client.get_nickname());

		handle_help(client_fd, ParsedMessage(""));
	}
	return 0;
}

void Server::process_client_data(size_t& index, int client_fd)
{
	char buffer[2];
	ssize_t bytes_read;
	while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		// Write to the buffer which is used to store data the client sends
		_clients.at(client_fd).write_output_buffer(std::string(buffer, bytes_read));
	}
	if (bytes_read == 0)
	{
		std::cout << "Client disconnected (recv returned 0)" << std::endl;
		handle_disconnection(index);
		return ;
	}
	else if (bytes_read < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
	{
		std::cerr << "recv() failed: " << std::strerror(errno) << std::endl;
		handle_disconnection(index);
		return ;
	}
	std::string line;
	line = _clients.at(client_fd).extract_output_line();

	// If there are no complete lines => just return
	if (line.empty())
		return ;
	
	ParsedMessage parsedmsg(line);
	handle_client_command(index, client_fd, parsedmsg);
}

int Server::handle_ping(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty())
	{
		send_reply(fd, 461, { nickname, "PING" }, "Not enough parameters");
		return 0;
	}

	std::string target = msg.params[0];
	if (target.empty() || target[0] != ':')
	{
		send_reply(fd, 409, { nickname, "PING" }, "Invalid PING format. Use: PING :target");
		return 0;
	}

	std::string response = ':' + _hostname + ' ' + "PONG" + ' ' + target + "\r\n";
	client.send(response);
	return 0;
}

// The main server loop for Block 1
void Server::run()
{
	std::cout << "Entering server loop..." << std::endl;
	while (true)
	{
		// Block indefinitely (-1 timeout) waiting for events on file descriptors in _pollfds
		int num_events = poll(_pollfds.data(), _pollfds.size(), -1); // C++11 data() needed, or &(_pollfds[0]) for C++98

		if (_signal_received)
			break;

		if (num_events < 0)
		{
			// Handle poll errors, ignoring EINTR which means interrupted by signal
			if (errno == EINTR)
				continue; // Signal received, poll again
			throw std::runtime_error(std::string("Poll failed: ") + std::strerror(errno));
		}
		if (num_events == 0)
		{
			// Timeout occurred (shouldn't happen with -1 timeout), poll again
			continue;
		}

		// --- Handle events ---
		// Check the listening socket (it's always the first one we added)
		// Make sure _pollfds is not empty before accessing _pollfds[0]
		if (!_pollfds.empty() && _pollfds[0].fd == _listening_socket.get_fd())
		{
			if (_pollfds[0].revents & POLLIN)
			{
				// A new connection is ready to be accepted
				handle_new_connection();
				num_events--; // Decrement counter as we've handled one event
			}
		}
		// Entering the loop to check for events on client sockets like sending data, disconnections, errors...
		for (size_t i = 1; i < _pollfds.size(); ++i)
		{
			if (_pollfds[i].revents & POLLHUP)
			{
				std::cout << "Event on client socket (FD " << _pollfds[i].fd << "): Disconnection detected." << std::endl;
				// handle disconnection
				handle_disconnection(i);
				--num_events;
			}
			else if (_pollfds[i].revents & POLLIN)
			{
				std::cout << "Event on client socket (FD " << _pollfds[i].fd << "): Data ready to read." << std::endl;
				process_client_data(i, _pollfds[i].fd);
				--num_events;
			}
			else if (_pollfds[i].revents & (POLLERR | POLLNVAL))
			{
				// Check for errors on listening socket (rare but possible)
				std::cerr << "Error event on listening socket (FD " << _listening_socket.get_fd() << ")." << std::endl;
				// Depending on the error, you might want to exit or try to recover
				throw std::runtime_error("Fatal error on listening socket.");
			}
		}
	}
}
