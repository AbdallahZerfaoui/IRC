#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_quit(int fd, const ParsedMessage& msg)
{
	(void)msg;
	// remove the client from the channels he is in
	for (auto& channel : _channels)
	{
		// First send the part message to the channel and to the client
		std::string action = buildAction(_clients.at(fd), "PART", { "#" + channel.second.get_name(), "Client disconnected" });
		_channels.at(channel.second.get_name()).broadcast_message(action, fd);
		queue_send_to(fd, action);

		// Then remove the client from the channel
		channel.second.remove_client(fd);

		// If the channel has no members left, remove it from the server
		if (channel.second.get_members().empty())
			_channels.erase(channel.first);
	}
	return -1;
}
