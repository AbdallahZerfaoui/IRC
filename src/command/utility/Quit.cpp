#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_quit(int fd, const ParsedMessage& msg)
{
	(void)fd;
	(void)msg;
	return -1;
}