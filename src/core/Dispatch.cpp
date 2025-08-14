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
		std::ostringstream oss;
		oss << ':' << _hostname << ' ' << std::setw(3) << std::setfill('0') << RPL_WELCOME << ' ' << client.get_nickname() << ' ' << ":Welcome to the server\r\n";
		std::string joinLine = oss.str();
        send_raw(client_fd, joinLine);
		client.set_authenticated();
	}
	return 0;
}