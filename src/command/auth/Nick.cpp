#include "Server.hpp"
#include "ParsedMessage.hpp"

bool is_valid_nick(const std::string &nick)
{
	if (nick.empty() || nick.size() > 15)
		return false;
	if (!(std::isalpha(nick[0]) || std::string("-_[]\\`^{}|").find(nick[0]) != std::string::npos))
		return false;
	for (size_t i = 1; i < nick.size(); ++i)
	{
		unsigned char c = nick[i];
		if (!(std::isalpha(c) || std::isdigit(c) || std::string("-_[]\\`^{}|").find(c) != std::string::npos))
			return false;
	}
	return true;
}

int Server::parse_nick(int fd, const ParsedMessage &msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		client.send(buildReply(ERR_NONICKNAMEGIVEN, "NICK", client));
		return 0;
	}

	std::string nick = msg.params[0];
	if (!nick.empty() && nick[0] == ':')
	{
		nick.erase(0, 1);
	}

	auto valid_nick = is_valid_nick(nick);

	if (!valid_nick)
	{
		client.send(buildReply(ERR_ERRONEUSNICKNAME, "NICK", client));
		return 0;
	}

	if (is_duplicate_nickname(nick))
	{
		client.send(buildReply(ERR_NICKNAMEINUSE, "NICK", client));
		return 0;
	}

	std::string old = client.get_nickname();
	const bool is_change = old != nick;
	_clients.at(fd).set_passed_nick(nick);

	if (is_change)
	{
		// RFC: ":<oldnick>!user@host NICK :<newnick>"
		std::string line = buildAction(client, "NICK", nick);
		// std::string line = ":" + old + "!" + client.get_username() + "@" + _hostname + " NICK :" + nick + "\r\n";
		client.send(line);
		for (const auto &ch : _channels)
		{
			if (ch.second.has_member(fd))
			{
				ch.second.broadcast_message(line, fd);
			}
		}
	}
	return (0);
}