#include "Server.hpp"

void Server::process_client_data(size_t &index, int client_fd)
{
	char buffer[512];
	ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
	if (n > 0)
	{
		_clients.at(client_fd).write_output_buffer(std::string(buffer, n));
	}
	else if (n == 0)
	{
		handle_disconnection(index);
		return;
	}
	else
	{
		// recv() failed, check errno
		// If the error is EWOULDBLOCK or EAGAIN, it means no data is available right now, which is normal for non-blocking sockets.
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return;
		// If recv() failed for another reason, print the error and handle disconnection
		std::cerr << "recv() failed: " << std::strerror(errno) << std::endl;
		handle_disconnection(index);
		return;
	}

	// Loop as long as there are complete lines in the output buffer and process them
	for (;;)
	{
		std::string line = _clients.at(client_fd).extract_output_line();
		if (line.empty())
			break;
		ParsedMessage parsedmsg(line);
		if (handle_client_command(index, client_fd, parsedmsg) == 1)
			return;
	}
}

void Server::process_client_output(size_t &index, int client_fd)
{
	Client& client = _clients.at(client_fd);
    std::string& out = client.get_output_buffer();

	// If the output buffer is empty, deactivate POLLOUT again
    if (out.empty())
	{
        deactivate_pollout(client_fd);
        return;
    }

	// Try to send data from the output buffer
	try
	{
		// Limit the data to send, because send() can only send a limited amount of data at once
		if (out.size() > 512)
			out.resize(512);
		ssize_t n = ::send(client_fd, out.data(), out.size(), 0);
		if (n > 0)
		{
			out.erase(0, static_cast<size_t>(n));
			if (out.empty())
				deactivate_pollout(client_fd);
			return;
		}
		// If the connection was closed by the client
		if (n == 0)
		{
			handle_disconnection(index);
			return;
		}
		// If the socket is not ready to send data right now, just return
		if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			return;
		// If send() failed for another reason, print the error and handle disconnection
		handle_disconnection(index);
		return;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error sending data to client (FD " << client_fd << "): " << e.what() << std::endl;
		handle_disconnection(index);
		return;
	}
}

void Server::queue_send_to(int fd, const std::string& msg)
{
    _clients.at(fd).queue_send(msg);
    activate_pollout(fd);
}

int Server::get_index_by_fd(int fd) const
{
	for (size_t i = 0; i < _pollfds.size(); ++i)
	{
		if (_pollfds[i].fd == fd)
			return static_cast<int>(i);
	}
	return -1;
}

void Server::activate_pollout(int fd)
{
	int index = get_index_by_fd(fd);
	if (index == -1)
		return;
	_pollfds[index].events |= POLLOUT;
}

void Server::deactivate_pollout(int fd)
{
	int index = get_index_by_fd(fd);
	if (index == -1)
		return;
	_pollfds[index].events &= ~POLLOUT;
}
