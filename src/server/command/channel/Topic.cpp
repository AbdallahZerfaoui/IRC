#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_topic(int fd, const ParsedMessage& msg)
{
	(void)msg;
	(void)fd;
	return 0;
}