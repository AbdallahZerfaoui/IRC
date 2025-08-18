#include "Server.hpp"

void Server::process_client_data(size_t &index, int client_fd)
{
	char buffer[4096];
	ssize_t n;
	while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0)
	{
		_clients.at(client_fd).write_output_buffer(std::string(buffer, n));
	}
	if (n == 0)
	{
		handle_disconnection(index);
		return;
	}
	if (n < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
	{
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
