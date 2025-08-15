#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_privmsg(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();
	if (msg.params.size() < 2)
	{
		client.send(buildReply(client, ERR_NEEDMOREPARAMS, {"PRIVMSG"}));
		return 0;
	}

	std::vector<std::string> targets = split(msg.params[0], ',');
    std::string text = msg.params[1];
    if (!text.empty() && text[0] == ':') {
        text.erase(0, 1);
	}

	for (size_t i = 0; i < targets.size(); ++i)
    {
        if (!targets[i].empty() && targets[i][0] == '#')
        {
            std::string chan = targets[i].substr(1);
            auto it = _channels.find(chan);
            if (it == _channels.end())
            {
				client.send(buildReply(client, ERR_NOSUCHCHANNEL, {nickname, targets[i]}));
                continue;
            }

            Channel& ch = it->second;
            if (!ch.has_member(fd))
            {
				client.send(buildReply(client, ERR_CANNOTSENDTOCHAN, {nickname, targets[i]}));
                continue;
            }

			ch.broadcast_message(buildAction(client, "PRIVMSG", {targets[i], text}), fd);
            continue;
        }

		int fdtg = find_fd_by_nickname(targets[i]);
		if (fdtg == -1)
		{
			client.send(buildReply(client, ERR_NOSUCHNICK, {targets[i]}));
			continue ;
		}

		_clients.at(fdtg).send(buildAction(client, "PRIVMSG", {targets[i], text}));
	}
	return 0;
}