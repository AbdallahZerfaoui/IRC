#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_invite(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.size() < 2)
	{
		send_reply(fd, ERR_NEEDMOREPARAMS, { nickname, "INVITE" }, "Not enough parameters");
		return 0;
	}

	std::string target_nick = msg.params[0];
	std::string chan_name = msg.params[1];

	if (chan_name.empty() || chan_name[0] != '#')
	{
		send_reply(fd, ERR_BADCHANMASK, { nickname, "INVITE" }, "Bad channel name");
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
	if (target_fd == -1)
	{
		send_reply(fd, 401, { nickname, target_nick }, "No such nick");
		return 0;
	}

	if (channel.has_member(target_fd))
	{
		send_reply(fd, ERR_USERONCHANNEL, { nickname, target_nick, chan_name }, "User already on channel");
		return 0;
	}

    channel.add_invited_client(target_fd);
    
	// send it to the invited user
	std::string invite_msg = ":" + nickname + "!user@host INVITE " + target_nick + " :" + chan_name + "\r\n";
	_clients.at(target_fd).send(invite_msg);

	return 0;
}
