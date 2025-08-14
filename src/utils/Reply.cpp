#include "Server.hpp"
#include <iomanip>

void Server::enable_pollout(int fd, bool enable)
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
		{
			if (enable)
				_pollfds[i].events |= POLLOUT;
			else
				_pollfds[i].events &= ~POLLOUT;
			break;
		}
	}
}


void Server::send_reply(int fd, int code, const std::vector<std::string> &params, const std::string &msg)
{
	std::ostringstream oss;
	oss << ':' << _hostname << ' ' << std::setw(3) << std::setfill('0') << code << ' ';
	for (size_t i = 0; i < params.size(); ++i)
	{
		if (i)
			oss << ' ';
		oss << params[i];
	}
	oss << BLUE << BOLD << " :" << msg << RESET << "\r\n";
	std::string text = oss.str();
	// _clients.at(fd).send(text);
	Client &c = _clients.at(fd);
	c.queue_send(text);
	if (!c.try_flush())
	{
		// If the output buffer is not empty, we need to enable POLLOUT for this client
		enable_pollout(fd, true);
	}
}
