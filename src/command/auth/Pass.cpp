#include "Server.hpp"
#include "ParsedMessage.hpp"
#include "Colors.hpp"


int Server::parse_pass(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (client.get_passed_pass())
	{
		try
		{
			client.send(buildReply(client, ERR_ALREADYREGISTERED, {"PASS"}));
		}
		catch (const std::exception& e)  //TODO: should we kepp it??
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
		return 0;
	}

	if (msg.params.size() != 1 || msg.params[0].empty())
	{
		try
		{
			client.send(buildReply(client, ERR_NEEDMOREPARAMS, {"PASS"}));
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
			client.send(buildReply(client, ERR_PASSWDMISMATCH, {"PASS"}));
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