#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_kick(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.size() < 2)
	{
		queue_send_to(fd, buildReply(client, ERR_NEEDMOREPARAMS, {"KICK"}));
		return 0;
	}

	std::string chan_name = msg.params[0];
	std::string target_nick = msg.params[1];
	std::string reason = (msg.params.size() > 2) ? msg.params[2] : nickname;

	if (chan_name.empty() || chan_name[0] != '#')
	{
		queue_send_to(fd, buildReply(client, ERR_BADCHANMASK, {"KICK"}));
		return 0;
	}

	std::string chan = chan_name.substr(1);
	auto it = _channels.find(chan);
	if (it == _channels.end())
	{
		queue_send_to(fd, buildReply(client, ERR_NOSUCHCHANNEL, {chan_name}));
		return 0;
	}
	Channel &channel = it->second;

	if (!channel.is_operator(fd))
	{
		queue_send_to(fd, buildReply(client, ERR_CHANOPRIVSNEEDED, {chan_name}));
		return 0;
	}

	int target_fd = find_fd_by_nickname(target_nick);
	if (target_fd == -1 || !channel.has_member(target_fd))
	{
		queue_send_to(fd, buildReply(client, ERR_NOSUCHNICK, {target_nick}));
		return 0;
	}

	channel.remove_client(target_fd);

	std::string action = buildAction(_clients.at(target_fd), "PART", {chan_name, reason });
	channel.broadcast_message(action, target_fd);
	queue_send_to(target_fd, action);
	// delete the channel if it has no members left 
	if (channel.get_members().empty())
		_channels.erase(it);

	return 0;
}
