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
