#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_kick(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.size() < 2)
	{
		send_reply(fd, ERR_NEEDMOREPARAMS, { nickname, "KICK" }, "Not enough parameters");
		return 0;
	}

	std::string chan_name = msg.params[0];
	std::string target_nick = msg.params[1];
	std::string reason = (msg.params.size() > 2) ? msg.params[2] : nickname;

	if (chan_name.empty() || chan_name[0] != '#')
	{
		send_reply(fd, ERR_BADCHANMASK, { nickname, "KICK" }, "Bad channel name");
		return 0;
	}

	std::string chan = chan_name.substr(1);
	auto it = _channels.find(chan);
	if (it == _channels.end())
	{
		send_reply(fd, ERR_NOSUCHCHANNEL, { nickname, chan_name }, "No such channel");
		return 0;
	}
	Channel &channel = it->second;

	if (!channel.is_operator(fd))
	{
		send_reply(fd, ERR_CHANOPRIVSNEEDED, { nickname, chan_name }, "You're not channel operator");
		return 0;
	}

	int target_fd = find_fd_by_nickname(target_nick);
	if (target_fd == -1 || !channel.has_member(target_fd))
	{
		send_reply(fd, ERR_USERNOTINCHANNEL, { nickname, target_nick }, "Is not on that channel");
		return 0;
	}

	channel.remove_client(target_fd);

	std::string kick_msg = ":" + nickname + "!user@host KICK " + chan_name + " " + target_nick + " :" + reason + "\r\n";
	channel.broadcast_message(kick_msg, -1);

	_clients.at(target_fd).send(kick_msg);

	// delete the channel if it has no members left 
	if (channel.get_members().empty())
		_channels.erase(it);

	return 0;
}
