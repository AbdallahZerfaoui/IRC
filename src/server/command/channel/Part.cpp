#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_part(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

    if (msg.params.empty() || msg.params.size() > 2)
    {
        send_reply(fd, ERR_NEEDMOREPARAMS, { nickname, "PART" }, "Wrong number of parameters");
        return 0;
    }

    std::vector<std::string> chans = split(msg.params[0], ',');
	std::string reason = (msg.params.size() == 2) ? msg.params[1] : "Leaving the channel";
    if (!reason.empty() && reason[0] == ':') {
        reason.erase(0,1);
	}

	for (size_t i = 0; i < chans.size(); ++i)
	{
		if (chans[i].empty() || chans[i][0] != '#')
        {
            send_reply(fd, 476, { nickname, "PART" }, "Bad channel mask");
            continue;
        }
		chans[i].erase(0, 1); // Remove the '#' character

		auto it = _channels.find(chans[i]);
		if (it == _channels.end())
		{
			send_reply(fd, 403, { nickname, "PART", "#" + chans[i] }, "No such channel");
			continue ;
		}

		if (!it->second.remove_client(fd))
		{
			send_reply(fd, 442, { nickname, "PART", "#" + chans[i] }, "You're not on that channel");
			continue ;
		}
		std::string message = ':' + _clients.at(fd).get_nickname() + "@host PRIVMSG #" + _channels.at(chans[i]).get_name() + " :" + reason + "\r\n";
		_channels.at(chans[i]).broadcast_message(message, fd);

		if (it->second.get_members().empty())
			_channels.erase(it);
	}
	return 0;
}