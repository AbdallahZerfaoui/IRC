#include "Server.hpp"
#include "ParsedMessage.hpp"

int Server::parse_user(int fd, const ParsedMessage& msg)
{
    Client& client = _clients.at(fd);
    std::string nickname = client.get_nickname();
    if (nickname.empty()) nickname = "*";

    if (client.get_passed_user()) {
        send_reply(fd, 462, { nickname, "USER" }, "You may not reregister");
        return 0;
    }

    if (msg.params.size() < 4) {
        send_reply(fd, 461, { nickname, "USER" }, "Not enough parameters");
        return 0;
    }

    const std::string& username   = msg.params[0];
    std::string realname          = msg.params[3];

    if (!realname.empty() && realname[0] == ':')
        realname.erase(0, 1);

    if (username.empty() ||
        username.find_first_of(" \t\r\n\v\f") != std::string::npos)
    {
        send_reply(fd, 461, { nickname, "USER" }, "Erroneous username");
        return 0;
    }

    client.set_passed_user(username);
    client.set_passed_realname(realname);

    std::cout << GREEN
              << "Client FD " << fd << " set user to '" << username
              << "' with real name: '" << realname << "'.\n"
              << RESET;

    return 0;
}
