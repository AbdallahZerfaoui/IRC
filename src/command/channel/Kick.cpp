#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_kick(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.size() < 2)
	{
		client.send(buildReply(client, ERR_NEEDMOREPARAMS, {"KICK"}));
		return 0;
	}

	std::string chan_name = msg.params[0];
	std::string target_nick = msg.params[1];
	std::string reason = (msg.params.size() > 2) ? msg.params[2] : nickname;

	if (chan_name.empty() || chan_name[0] != '#')
	{
		client.send(buildReply(client, ERR_BADCHANMASK, {"KICK"}));
		return 0;
	}

	std::string chan = chan_name.substr(1);
	auto it = _channels.find(chan);
	if (it == _channels.end())
	{
		client.send(buildReply(client, ERR_NOSUCHCHANNEL, {chan_name}));
		return 0;
	}
	Channel &channel = it->second;

	if (!channel.is_operator(fd))
	{
		client.send(buildReply(client, ERR_CHANOPRIVSNEEDED, {chan_name}));
		return 0;
	}

	int target_fd = find_fd_by_nickname(target_nick);
	if (target_fd == -1 || !channel.has_member(target_fd))
	{
		client.send(buildReply(client, ERR_USERNOTINCHANNEL, {target_nick, chan_name}));
		return 0;
	}

	channel.remove_client(target_fd);
	channel.broadcast_message(buildAction(client, "PART", {chan_name, target_nick, reason}), fd);

	std::string action = buildAction(client, "PART", {chan_name, reason });
	channel.broadcast_message(action, fd);
	_clients.at(target_fd).send(action);
	// delete the channel if it has no members left 
	if (channel.get_members().empty())
		_channels.erase(it);

	return 0;
}
