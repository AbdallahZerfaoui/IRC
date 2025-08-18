#include "Server.hpp"
#include "ParsedMessage.hpp"
#include "Colors.hpp"


int Server::parse_pass(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);

	if (client.get_passed_pass())
	{
		client.send(buildReply(client, ERR_ALREADYREGISTERED, {"PASS"}));
		return 0;
	}

	if (msg.params.size() != 1 || msg.params[0].empty())
	{
		client.send(buildReply(client, ERR_NEEDMOREPARAMS, {"PASS"}));
		return 0;
	}

	if (msg.params[0] != this->_password)
	{
		client.send(buildReply(client, ERR_PASSWDMISMATCH, {"PASS"}));
		return 0;
	}
	client.set_passed_pass(msg.params[0]);
	return 0;
}
