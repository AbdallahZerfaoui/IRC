#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::handle_notice(int fd, const ParsedMessage& msg)
{
    Client& client = _clients.at(fd);
    
    // NOTICE should not generate replies if there's an error (RFC 2812)
    if (msg.params.size() < 2)
        return 0;

    std::vector<std::string> targets = split(msg.params[0], ',');
    std::string text = msg.params[1];
    if (!text.empty() && text[0] == ':') {
        text.erase(0, 1);
    }

    for (const auto& target : targets)
    {
        if (!target.empty() && target[0] == '#')
        {
            // Channel notice
            std::string chan = target.substr(1);
            auto it = _channels.find(chan);
            if (it == _channels.end())
                continue;  // Silently ignore nonexistent channels

            Channel& ch = it->second;
            if (!ch.has_member(fd))
                continue;  // Silently ignore

            ch.broadcast_message(buildAction(client, "NOTICE", {target, text}), fd);
        }
        else
        {
            // User notice
            int target_fd = find_fd_by_nickname(target);
            if (target_fd == -1)
                continue;  // Silently ignore nonexistent users

            _clients.at(target_fd).send(buildAction(client, "NOTICE", {target, text}));
        }
    }
    return 0;
}
