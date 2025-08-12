#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_client_command(size_t &index, int client_fd, const ParsedMessage &parsedmsg)
{
	if (parsedmsg.command.empty())
	{
		return 0;
	}

	Client &client = _clients.at(client_fd);
	std::string nickname = client.get_nickname();
	if (nickname.empty())
		nickname = "*";

	bool preauth_ok =
		parsedmsg.command == "PASS" ||
		parsedmsg.command == "NICK" ||
		parsedmsg.command == "USER" ||
		parsedmsg.command == "CAP" ||
		parsedmsg.command == "PING" ||
		parsedmsg.command == "QUIT";

	if (!preauth_ok && !client.get_passed_pass())
	{
		send_reply(client_fd, ERR_USERNOTINCHANNEL, {nickname}, "You have not registered");
		return 0;
	}

	if (!client.is_authenticated() && !preauth_ok)
	{
		send_reply(client_fd, ERR_USERNOTINCHANNEL, {nickname}, "You have not registered");
		return 0;
	}

	auto it = handlers.find(parsedmsg.command);
	if (it == handlers.end())
	{
		send_reply(client_fd, ERR_UNKNOWNCOMMAND, {nickname, parsedmsg.command}, "Unknown command");
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
		std::string nick_for_welcome = client.get_nickname();
		if (nick_for_welcome.empty())
			nick_for_welcome = "*";

		send_reply(client_fd, RPL_WELCOME, {nick_for_welcome},
				   "Welcome to ft_irc, " + client.get_nickname());

		handle_help(client_fd, ParsedMessage(""));
	}
	return 0;
}