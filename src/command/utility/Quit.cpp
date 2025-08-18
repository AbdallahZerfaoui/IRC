#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_quit(int fd, const ParsedMessage& msg)
{
	(void)msg;
	// remove the client from the channels he is in
	for (auto& channel : _channels)
	{
		channel.second.remove_client(fd);
	}
	return -1;
}