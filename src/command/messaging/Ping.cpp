// #include "Server.hpp"
// #include "ParsedMessage.hpp"


// int Server::handle_ping(int fd, const ParsedMessage& msg)
// {
// 	Client& client = _clients.at(fd);
// 	std::string nickname = client.get_nickname();

// 	if (msg.params.empty())
// 	{
// 		send_reply(fd, ERR_NEEDMOREPARAMS, { nickname, "PING" }, "Not enough parameters");
// 		return 0;
// 	}

// 	std::string target = msg.params[0];
// 	if (target.empty() || target[0] != ':')
// 	{
// 		send_reply(fd, ERR_NOORIGIN, { nickname, "PING" }, "Invalid PING format. Use: PING :target");
// 		return 0;
// 	}

// 	std::string response = ':' + _hostname + ' ' + "PONG" + ' ' + target + "\r\n";
// 	client.send(response);
// 	return 0;
// }