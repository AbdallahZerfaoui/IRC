#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_join(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty())
	{
		queue_send_to(fd, buildReply(client, ERR_NEEDMOREPARAMS, {"JOIN"}));
		return 0;
	}

	std::vector<std::string> chans = split(msg.params[0], ',');
	std::vector<std::string> keys = (msg.params.size() > 1) ? split(msg.params[1], ',') : std::vector<std::string>();

	for (size_t i = 0; i < chans.size(); ++i)
	{
		const std::string& raw = chans[i];
		if (raw.empty() || raw[0] != '#')
		{
			queue_send_to(fd, buildReply(client, ERR_BADCHANMASK, {"JOIN"}));
			continue;
		}
		std::string name = raw.substr(1);
		std::string key = (i < keys.size()) ? keys[i] : "";

		// Add the channel to the channels map, if it doesn't exist
		if (!_channels.count(name))
			_channels.emplace(name, Channel(*this, name));
		Channel& ch = _channels.at(name);

        // if the channel already exists and the client is already a member
        if (ch.has_member(fd))
        {
			queue_send_to(fd, buildReply(client, ERR_USERONCHANNEL, {"#" + name}));
            continue;
        }

		// If the channel requires a key and the key is not provided or incorrect
		if (ch.requires_key() && (key.empty() || key != ch.get_channel_key()))
		{
			queue_send_to(fd, buildReply(client, ERR_BADCHANNELKEY, {"#" + name}));
			continue;
		}

		// If the channel is invite-only and the user was not invited
		if (ch.is_invite_only() && !ch.is_invited(fd))
		{
			queue_send_to(fd, buildReply(client, ERR_INVITEONLYCHAN, {"#" + name}));
            continue;
        }

		// If the channel has a user limit and the channel is full
        // AND the user was not invited
        if (ch.get_limit() != -1 && (int)ch.get_members().size() >= ch.get_limit() && !ch.is_invited(fd))
		{
			queue_send_to(fd, buildReply(client, ERR_CHANNELISFULL, {"#" + name}));
            continue;
        }

		ch.add_client(fd);

		if (ch.is_invited(fd))
			ch.remove_invited_client(fd);
		
		// Send the JOIN message to the client and broadcast it to other members
		ch.broadcast_message(buildAction(client, "JOIN", {"#" + name}), -1);

		if (ch.get_members().size() == 1)
		{
            ch.add_operator(fd);
            ch.broadcast_message(buildServerMode("#" + name, "+o", nickname), -1);
		}

		if (ch.topic().empty())
			queue_send_to(fd, buildReply(client, RPL_NOTOPIC, {"#" + name}));
		else
			queue_send_to(fd, buildReply(client, RPL_TOPIC, {"#" + name, ch.topic()}));
	}
	return 0;
}
