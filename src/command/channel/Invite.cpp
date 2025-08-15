#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_invite(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	if (msg.params.size() < 2)
	{
		client.send(buildReply(client, ERR_NEEDMOREPARAMS, {"INVITE"}));
		return 0;
	}

	std::string target_nick = msg.params[0];
	std::string chan_name = msg.params[1];

	if (chan_name.empty() || chan_name[0] != '#')
	{
		client.send(buildReply(client, ERR_BADCHANMASK, {"INVITE"}));
		return 0;
	}

	std::string chan = chan_name.substr(1);
	auto it = _channels.find(chan);
	if (it == _channels.end())
	{
		client.send(buildReply(client, ERR_NOSUCHCHANNEL, {"#" + chan_name}));
		return 0;
	}
	Channel &channel = it->second;

	if (!channel.is_operator(fd))
	{
		client.send(buildReply(client, ERR_CHANOPRIVSNEEDED, {"#" + chan_name}));
		return 0;
	}

	int target_fd = find_fd_by_nickname(target_nick);
	if (target_fd == -1)
	{
		client.send(buildReply(client, ERR_NOSUCHNICK, {target_nick}));
		return 0;
	}

	if (channel.has_member(target_fd))
	{
		client.send(buildReply(client, ERR_USERONCHANNEL, {target_nick}));
		return 0;
	}

    channel.add_invited_client(target_fd);
	// Send RPL_INVITING to the inviting user
	client.send(buildReply(client, RPL_INVITING, {target_nick, "#" + chan_name}));
    
	// send it to the invited user
	_clients.at(target_fd).send(buildAction(client, "INVITE", { _clients.at(target_fd).get_nickname(), "#" + chan_name}));

	return 0;
}
