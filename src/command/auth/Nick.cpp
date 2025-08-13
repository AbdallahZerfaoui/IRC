#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::parse_nick(int fd, const ParsedMessage &msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
	{
		send_reply(fd, ERR_NONICKNAMEGIVEN, {nickname}, "No nickname given");
		return 0;
	}

	std::string nick = msg.params[0];
	if (!nick.empty() && nick[0] == ':')
	{
		nick.erase(0, 1);
	}

	auto is_valid_nick = [](const std::string &s) -> bool
	{
		if (s.empty() || s.size() > 15)
			return false;
		if (!(std::isalpha(s[0]) || std::string("-_[]\\`^{}|").find(s[0]) != std::string::npos))
			return false;
		for (size_t i = 1; i < s.size(); ++i)
		{
			unsigned char c = s[i];
			if (!(std::isalpha(c) || std::isdigit(c) || std::string("-_[]\\`^{}|").find(c) != std::string::npos))
				return false;
		}
		return true;
	};

	if (!is_valid_nick(nick))
	{
		send_reply(fd, ERR_ERRONEUSNICKNAME, {nickname, nick}, "Erroneous nickname");
		return 0;
	}

	if (is_duplicate_nickname(nick))
	{
		send_reply(fd, ERR_NICKNAMEINUSE, {nickname, "NICK"}, "Nickname is already in use");
		return 0;
	}

	std::string old = client.get_nickname();
	const bool is_change = !old.empty() && old != "anonymous" && old != nick;
	_clients.at(fd).set_passed_nick(nick);

	const std::string prefix = make_prefix(client);

	if (is_change)
	{
		// RFC: :<oldnick>!user@host NICK :<newnick>
		// std::string line = ":" + prefix + " NICK :" + nick + "\r\n";
		std::string line = ":" + old + "!" + client.get_username() + "@" + client.get_hostname() + " NICK :" + nick + "\r\n";
		std::cout << line << std::endl;
		send_raw(fd, line);
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