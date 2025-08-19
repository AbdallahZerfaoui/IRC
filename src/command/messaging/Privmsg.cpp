#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_privmsg(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();
	if (msg.params.size() < 2)
	{
		queue_send_to(fd, buildReply(client, ERR_NEEDMOREPARAMS, {"PRIVMSG"}));
		return 0;
	}

	std::vector<std::string> targets = split(msg.params[0], ',');
    std::string text = msg.params[1];

	for (size_t i = 0; i < targets.size(); ++i)
    {
        if (!targets[i].empty() && targets[i][0] == '#')
        {
            std::string chan = targets[i].substr(1);
            auto it = _channels.find(chan);
            if (it == _channels.end())
            {
				queue_send_to(fd, buildReply(client, ERR_NOSUCHCHANNEL, {nickname, targets[i]}));
                continue;
            }

            Channel& ch = it->second;
            if (!ch.has_member(fd))
            {
				queue_send_to(fd, buildReply(client, ERR_NOTONCHANNEL, {nickname, targets[i]}));
                continue;
            }

			ch.broadcast_message(buildAction(client, "PRIVMSG", {targets[i], text}), fd);
            continue;
        }

		int fdtg = find_fd_by_nickname(targets[i]);
		if (fdtg == -1)
		{
			queue_send_to(fd, buildReply(client, ERR_NOSUCHNICK, {targets[i]}));
			continue ;
		}

		if (fdtg == fd)
		{
			queue_send_to(fd, buildReply(client, ERR_CANNOTSENDTOCHAN, {targets[i]}));
			continue;
		}

		if (!_clients.at(fdtg).is_authenticated())
		{
			queue_send_to(fd, buildReply(client, ERR_NOTREGISTERED, {targets[i]}));
			continue;
		}

		queue_send_to(fdtg, buildAction(client, "PRIVMSG", {targets[i], text}));
	}
	return 0;
}