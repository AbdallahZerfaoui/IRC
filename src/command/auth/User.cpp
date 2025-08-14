#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::parse_user(int fd, const ParsedMessage &msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (client.get_passed_user())
	{
		client.send(buildReply(ERR_ALREADYREGISTERED, "USER", nickname));
		return 0;
	}

	if (msg.params.size() < 4)
	{
		client.send(buildReply(ERR_NEEDMOREPARAMS, "USER", nickname));
		return 0;
	}

	const std::string &username = msg.params[0];
	std::string realname = msg.params[3];

	client.set_passed_user(username);
	client.set_passed_realname(realname);
	return 0;
}
