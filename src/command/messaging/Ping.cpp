#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_ping(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);

	if (msg.params.empty())
	{
		queue_send_to(fd, buildReply(client, ERR_NEEDMOREPARAMS, {"PING"}));
		return 0;
	}

	std::string target = msg.params[0];

	// send a pong back to the client

	std::string response = "PONG " + _server_name + " :" + target + "\r\n";
	queue_send_to(fd, response);
	return (1);
}
