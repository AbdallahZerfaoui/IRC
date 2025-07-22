#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_help(int fd, const ParsedMessage &msg)
{
	(void)msg;
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	send_reply(fd, 704, {nickname, "*"}, "*** Available HELP topics ***");
	send_reply(fd, 705, {nickname, "*"}, "HELP                                                     :show this list");
	send_reply(fd, 705, {nickname, "*"}, "CHANNELS                                                 :list channels you are in");
	send_reply(fd, 705, {nickname, "*"}, "JOIN <#chan1,#chan2,...> <optional:key1,key2,...>        :join/create channel");
	send_reply(fd, 705, {nickname, "*"}, "PART <#chan1,#chan2,...> <optional:leaving_message>      :leave channel");
	send_reply(fd, 705, {nickname, "*"}, "PRIVMSG <target1,target2,...> <text>                     :send a message");
	send_reply(fd, 705, {nickname, "*"}, "QUIT                                                     :disconnect");
	send_reply(fd, 706, {nickname, "*"}, "*** End of HELP ***\n");
	return 0;
}