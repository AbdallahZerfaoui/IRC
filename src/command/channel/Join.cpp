#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_join(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname().empty() ? "*" : client.get_nickname();

	if (msg.params.empty())
	{
		send_reply(fd, ERR_NEEDMOREPARAMS, { nickname, "JOIN" }, "Not enough parameters");
		return 0;
	}

	std::vector<std::string> chans = split(msg.params[0], ',');
	std::vector<std::string> keys = (msg.params.size() > 1) ? split(msg.params[1], ',') : std::vector<std::string>();

	for (size_t i = 0; i < chans.size(); ++i)
	{
		const std::string& raw = chans[i];
		if (raw.empty() || raw[0] != '#')
		{
			send_reply(fd, ERR_BADCHANMASK, { nickname, "JOIN" }, "Bad Channel Mask");
			continue;
		}
		std::string name = raw.substr(1); // Remove the '#' character
		std::string key = (i < keys.size()) ? keys[i] : "";

		// Add the channel to the channels map, if it doesn't exist
		if (!_channels.count(name))
			_channels.emplace(name, Channel(name, _clients));

		Channel& ch = _channels.at(name);
        // if the channel already exists and the client is already a member
        if (ch.has_member(fd))
        {
            send_reply(fd, ERR_USERONCHANNEL, { nickname, "#" + name }, "You are already on that channel");
            continue;
        }

		// If the channel requires a key and the key is not provided or incorrect
		if (ch.requires_key() && (key.empty() || key != ch.get_channel_key()))
		{
			send_reply(fd, ERR_BADCHANNELKEY, { nickname, "#" + name }, "Cannot join channel (+k)");
			continue;
		}

		// If the channel is invite-only and the user was not invited
		if (ch.is_invite_only() && !ch.is_invited(fd)) {
            send_reply(fd, ERR_INVITEONLYCHAN, { nickname, "#" + name }, "Cannot join channel (+i)");
            continue;
        }

		// If the channel has a user limit and the channel is full
        // AND the user was not invited
        if (ch.get_limit() != -1 && (int)ch.get_members().size() >= ch.get_limit() && !ch.is_invited(fd)) {
            send_reply(fd, ERR_CHANNELISFULL, { nickname, "#" + name }, "Cannot join channel (+l)");
            continue;
        }

		// Finally add the client to the channel
		ch.add_client(fd);
		std::string joinLine = ":" + make_prefix(client) + " JOIN #" + name + "\r\n";
        send_raw(fd, joinLine);
		ch.broadcast_message(joinLine, fd);

		if (ch.get_members().size() == 1)
		{
            std::string modeLine = ":" + _hostname + " MODE #" + name + " +nt\r\n";
            send_raw(fd, modeLine);
            ch.broadcast_message(modeLine, -1);
            ch.add_operator(fd);
            std::string opLine = ":" + _hostname + " MODE #" + name + " +o " + nickname + "\r\n";
            send_raw(fd, opLine);
            ch.broadcast_message(opLine, -1);
		}

		if (ch.topic().empty())
			send_reply(fd, 331, { nickname, "#" + name }, "No topic is set");
		else
			send_reply(fd, 332, { nickname, "#" + name }, ch.topic());
	}
	return 0;
}