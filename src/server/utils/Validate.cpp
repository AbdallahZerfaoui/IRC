#include "Server.hpp"

bool Server::valid_inputs(int port, const std::string& password)
{
	if (port <= 0 || port > MAX_PORT_NBR) {
		std::cerr << "Error: Invalid port number." << std::endl;
		return false;
	}
	if (password.empty()) {
		std::cerr << "Error: Empty password provided." << std::endl;
		return false;
	}
	return true;
}

bool Server::is_duplicate_nickname(const std::string& nickname)
{
	// Check if the nickname is already taken by another client
	for (const auto& client : _clients)
	{
		if (client.second.get_nickname() == nickname)
		{
			return true;
		}
	}
	return false;
}