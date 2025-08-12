#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::parse_nick(int fd, const ParsedMessage& msg)
{
	for (const auto& param : msg.params)
	{
		std::cout << param << " ";
	}
	std::cout << std::endl;

	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty() || msg.params[0].empty())
    {
		try
		{
			send_reply(fd, ERR_NONICKNAMEGIVEN, { nickname, "NICK" }, "No nickname given");
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
        return 0;
    }
	std::string nick = msg.params[0];
    if (!nick.empty() && nick[0] == ':') {
        nick.erase(0, 1);
	}

	if (![](const std::string &s)
		{ return !s.empty() &&
				 std::all_of(s.begin(), s.end(),
							 [](unsigned char c)
							 { return std::isalnum(c); }); }(nick))
	{
		try
		{
			send_reply(fd, ERR_ERRONEUSNICKNAME, { nickname, "NICK" }, "Erroneous nickname");
		}
		catch (const std::exception &e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
		return 0;
	}

	if (is_duplicate_nickname(nick))
    {
		try
		{
			send_reply(fd, ERR_NICKNAMEINUSE, { nickname, "NICK" }, "Nickname is already in use");
		}
		catch(const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
		}
        return 0;
    }
	std::string old = client.get_nickname();
    client.set_passed_nick(nick);
	if (!old.empty() && old != "anonymous" && old != nick)
    {
        std::string text = old + " is now known as " + nick;
		broadcast_to_all(text, fd);
    }
    std::cout << GREEN << "Client FD " << fd << " set nickname to " << nick << ".\n" << RESET;
    return (0);
}