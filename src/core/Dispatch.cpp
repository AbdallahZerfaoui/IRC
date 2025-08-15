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

	bool preauth_ok =
		parsedmsg.command == "PASS" ||
		parsedmsg.command == "NICK" ||
		parsedmsg.command == "USER" ||
		parsedmsg.command == "CAP" ||
		parsedmsg.command == "PING" ||
		parsedmsg.command == "QUIT";

	if (!preauth_ok && !client.get_passed_pass())
	{
		client.send(buildReply(client, ERR_NOTREGISTERED, {parsedmsg.command}));
		return 0;
	}

	if (!client.is_authenticated() && !preauth_ok)
	{
		client.send(buildReply(client, ERR_NOTREGISTERED, {parsedmsg.command}));
		return 0;
	}

	auto it = handlers.find(parsedmsg.command);
	if (it == handlers.end())
	{
		client.send(buildReply(client, ERR_UNKNOWNCOMMAND, {nickname, parsedmsg.command}));
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
		client.send(buildReply(client, RPL_WELCOME, {client.get_nickname()}));
		client.set_authenticated();
	}
	return 0;
}
