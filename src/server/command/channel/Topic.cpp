#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_topic(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.empty())
	{
		send_reply(fd, 461, { nickname, "TOPIC" }, "Not enough parameters");
		return 0;
	}

	std::string chan_name = msg.params[0];
	if (chan_name.empty() || chan_name[0] != '#')
	{
		send_reply(fd, 476, { nickname, "TOPIC" }, "Bad channel name");
		return 0;
	}

	std::string chan = chan_name.substr(1);
	auto it = _channels.find(chan);
	if (it == _channels.end())
	{
		send_reply(fd, 403, { nickname, chan_name }, "No such channel");
		return 0;
	}
	Channel &channel = it->second;

	if (!channel.has_member(fd))
	{
		send_reply(fd, 442, { nickname, chan_name }, "You're not on that channel");
		return 0;
	}

	// just return the topic if no new topic is provided
	if (msg.params.size() == 1)
	{
		std::string topic = channel.get_topic();
		if (topic.empty())
			send_reply(fd, 331, { nickname, chan_name }, "No topic is set");
		else
			send_reply(fd, 332, { nickname, chan_name }, topic);
		return 0;
	}

    // If a new topic is provided, check if the user is allowed to change it
	if (channel.is_topic_protected() && !channel.is_operator(fd))
	{
		send_reply(fd, 482, { nickname, chan_name }, "You're not channel operator");
		return 0;
	}

	std::string new_topic = msg.params[1];
	if (!new_topic.empty() && new_topic[0] == ':')
		new_topic.erase(0, 1);
	channel.set_topic(new_topic);

	std::string topic_msg = ":" + nickname + "!user@host TOPIC " + chan_name + " :" + new_topic + "\r\n";
	channel.broadcast_message(topic_msg, -1);
	return 0;
}
