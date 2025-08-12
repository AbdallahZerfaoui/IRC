#include "Server.hpp"

void Server::send_reply(int fd, int code, const std::vector<std::string>& params, const std::string& msg)
{
	std::string text = ':' + _hostname + ' ' + std::to_string(code) + ' ';
    for (size_t i = 0; i < params.size(); ++i)
	{
        if (i)
			text += ' ';
		text += params[i];
    }
    text += " :" + msg + "\r\n";
	_clients.at(fd).send(text);
}
