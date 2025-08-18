#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_part(int fd, const ParsedMessage& msg)
{
	Client &client = _clients.at(fd);

    if (msg.params.empty() || msg.params.size() > 2)
    {
		client.send(buildReply(client, ERR_NEEDMOREPARAMS, {"PART"}));
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
			client.send(buildReply(client, ERR_BADCHANMASK, {"PART"}));
            continue;
        }
		chans[i].erase(0, 1);

		auto it = _channels.find(chans[i]);
		if (it == _channels.end())
		{
			client.send(buildReply(client, ERR_NOSUCHCHANNEL, {"PART", "#" + chans[i]}));
			continue ;
		}

		if (!it->second.remove_client(fd))
		{
			client.send(buildReply(client, ERR_NOTONCHANNEL, {"PART", "#" + chans[i]}));
			continue ;
		}
		std::string action = buildAction(client, "PART", { "#" + chans[i], reason });
		_channels.at(chans[i]).broadcast_message(action, fd);
		client.send(action);

		if (it->second.get_members().empty())
			_channels.erase(it);
	}
	return 0;
}