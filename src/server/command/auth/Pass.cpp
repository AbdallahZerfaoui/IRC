#include "Server.hpp"
#include "ParsedMessage.hpp"
#include "Colors.hpp"


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
			send_reply(fd, ERR_NEEDMOREPARAMS, { nickname, "PASS" }, "Not enough parameters");
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