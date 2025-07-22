#include "Server.hpp"

void Server::process_client_data(size_t& index, int client_fd)
{
	char buffer[2];
	ssize_t bytes_read;
	while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0)
	{
		// Write to the buffer which is used to store data the client sends
		_clients.at(client_fd).write_output_buffer(std::string(buffer, bytes_read));
	}
	if (bytes_read == 0)
	{
		std::cout << "Client disconnected (recv returned 0)" << std::endl;
		handle_disconnection(index);
		return ;
	}
	else if (bytes_read < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
	{
		std::cerr << "recv() failed: " << std::strerror(errno) << std::endl;
		handle_disconnection(index);
		return ;
	}
	std::string line;
	line = _clients.at(client_fd).extract_output_line();

	// If there are no complete lines => just return
	if (line.empty())
		return ;
	
	ParsedMessage parsedmsg(line);
	handle_client_command(index, client_fd, parsedmsg);
}