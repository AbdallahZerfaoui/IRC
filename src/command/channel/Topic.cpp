#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_topic(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty())
	{
		client.send(buildReply(client, ERR_NEEDMOREPARAMS, {"TOPIC"}));
		return 0;
	}

	std::string chan_name = msg.params[0];
	if (chan_name.empty() || chan_name[0] != '#')
	{
		client.send(buildReply(client, ERR_BADCHANMASK, {"TOPIC"}));
		return 0;
	}

	std::string chan = chan_name.substr(1);
	auto it = _channels.find(chan);
	if (it == _channels.end())
	{
		client.send(buildReply(client, ERR_NOSUCHCHANNEL, {"TOPIC", chan_name}));
		return 0;
	}
	Channel &channel = it->second;

	if (!channel.has_member(fd))
	{
		client.send(buildReply(client, ERR_NOTONCHANNEL, {"TOPIC", chan_name}));
		return 0;
	}

	if (msg.params.size() == 1)
	{
		std::string topic = channel.get_topic();
		if (topic.empty())
			client.send(buildReply(client, RPL_NOTOPIC, {chan_name}));
		else
			client.send(buildReply(client, RPL_TOPIC, {chan_name, topic}));
		return 0;
	}

    // If a new topic is provided, check if the user is allowed to change it
	if (channel.is_topic_protected() && !channel.is_operator(fd))
	{
		client.send(buildReply(client, ERR_CHANOPRIVSNEEDED, {chan_name}));
		return 0;
	}

	std::string new_topic = msg.params[1];
	if (!new_topic.empty() && new_topic[0] == ':')
		new_topic.erase(0, 1);
	channel.set_topic(new_topic);

	// Broadcast the new topic to all members of the channel
	channel.broadcast_message(buildUserMode(client, chan_name, "TOPIC", new_topic), fd);
	return 0;
}
