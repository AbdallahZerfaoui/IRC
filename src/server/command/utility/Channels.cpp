#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_channels(int fd, const ParsedMessage& msg)
{
	(void)msg;
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	try
	{
		std::string list;
		for (const auto& channel : _channels)
		{
			if (channel.second.get_members().count(fd))
			{
				std::cout << "Client " << nickname << " is in channel: " << channel.first << std::endl;
				// list + '#' + channel.first + ' ';
			}
		}
		if (list.empty())
		{
			list = "None";
		}
		send_reply(fd, RPL_YOUREOPER, { nickname, "CHANNELS" }, list);

	}
	catch (const std::exception& e)
	{
		std::cerr << "Error sending message: " << e.what() << std::endl;
		return 0;
	}
	return 0;
}