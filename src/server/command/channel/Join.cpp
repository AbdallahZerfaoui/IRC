#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_join(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty() || msg.params.size() > 2)
	{
		send_reply(fd, ERR_NEEDMOREPARAMS, { nickname, "JOIN" }, "Wrong number of parameters");
		return 0;
	}

	std::vector<std::string> chans = split(msg.params[0], ',');
	std::vector<std::string> keys = (msg.params.size() > 1) ? split(msg.params[1], ',') : std::vector<std::string>();

	std::string chan;
	for (size_t i = 0; i < chans.size(); ++i)
	{
		if (chans[i].empty() || chans[i][0] != '#')
		{
			send_reply(fd, 476, { nickname, "JOIN" }, "Bad channel name");
			continue;
		}
		chan = chans[i].substr(1); // Remove the '#' character
		std::string key = (i < keys.size()) ? keys[i] : "";

		// Add the channel to the channels map, if it doesn't exist
		if (!_channels.count(chan))
			_channels.emplace(chan, Channel(chan, _clients));

        // if the channel already exists and the client is already a member
        if (_channels.at(chan).has_member(fd))
        {
            send_reply(fd, 443, { nickname, chan }, "You are already on that channel");
            continue;
        }

		// If the channel requires a key and the key is not provided or incorrect
		if (_channels.at(chan).requires_key() && (key.empty() || key != _channels.at(chan).get_channel_key()))
		{
			send_reply(fd, 475, { nickname, chans[i] }, "Cannot join, bad key");
			continue;
		}

        // If the channel has a user limit and the channel is full
        // AND the user was not invited
        if (_channels.at(chan).get_limit() != -1 && static_cast<int>(_channels.at(chan).get_members().size()) >= _channels.at(chan).get_limit() && !_channels.at(chan).is_invited(fd))
		{
			send_reply(fd, 471, { nickname, chans[i] }, "Cannot join, channel is full");
			continue;
		}

        // If the channel is invite-only and the user was not invited
        if (_channels.at(chan).is_invite_only() && !_channels.at(chan).is_invited(fd))
        {
            send_reply(fd, 473, { nickname, chans[i] }, "Cannot join, channel is invite-only");
            continue;
        }

		// Finally add the client to the channel
		_channels.at(chan).add_client(fd);
		try
		{
			send_reply(fd, 476, { nickname }, "You have joined the channel " + chan);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Error sending message: " << e.what() << std::endl;
			return 0;
		}
		// Notify other clients in the channel (forward a message to all other clients in the channel
		std::string message = ':' + _clients.at(fd).get_nickname() + "@host PRIVMSG #" + _channels.at(chan).get_name() + " :" + " has joined the channel" + "\r\n";
		_channels.at(chan).broadcast_message(message, fd);

		if (_channels.at(chan).get_members().size() == 1)
		{
			// Make the client an operator if they are the first to join the channel
			_channels.at(chan).add_operator(fd);
			try
			{
				send_reply(fd, 705, { nickname, "JOIN", "#" + chan }, "You are now an operator of the channel");
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error sending message: " << e.what() << std::endl;
				return 0;
			}
			// Notify other clients in the channel that the client is now an operator
			std::string message = ':' + _hostname + "MODE #" + _channels.at(chan).get_name() + " +o " + client.get_nickname() + "\r\n";
			_channels.at(chan).broadcast_message(message, -1);
		}
	}
	return 0;
}