#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_privmsg(int fd, const ParsedMessage& msg)
{
	Client& client = _clients.at(fd);
	std::string nickname = client.get_nickname();
	if (msg.params.size() < 2)
	{
		send_reply(fd, ERR_NORECIPIENT, { nickname, "PRIVMSG" }, "No recipient given");
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
                send_reply(fd, ERR_NOSUCHCHANNEL, { nickname, targets[i] }, "No such channel");
                continue;
            }

            Channel& ch = it->second;
            if (!ch.has_member(fd))
            {
                send_reply(fd, ERR_CANNOTSENDTOCHAN, { nickname, targets[i] }, "Cannot send to channel");
                continue;
            }

			std::string message1 = ':' + _clients.at(fd).get_nickname() + "!user@host PRIVMSG #" + _channels.at(chan).get_name() + " :" + text + "\r\n";
            ch.broadcast_message(message1, fd);
            continue;
        }

		int fdtg = find_fd_by_nickname(targets[i]);
		if (fdtg == -1)
		{
			send_reply(fd, ERR_NOSUCHNICK, { nickname, "PRIVMSG", targets[i] }, "No such nickname");
			continue ;
		}

		std::string message = ':' + client.get_nickname() + "!user@host PRIVMSG " + targets[i] + " :" + text + "\r\n";
		_clients.at(fdtg).send(message);
	}
	return 0;
}