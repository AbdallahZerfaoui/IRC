#include "Server.hpp"
#include "ParsedMessage.hpp"


int Server::handle_help(int fd, const ParsedMessage &msg)
{
	(void)msg;
	Client &client = _clients.at(fd);
	std::string nickname = client.get_nickname();

	send_reply(fd, 704, {nickname, "*"}, "*** Available HELP topics ***");
	send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "HELP                                                     :show this list");
	send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "CHANNELS                                                 :list channels you are in");
	send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "JOIN <#chan1,#chan2,...> <optional:key1,key2,...>        :join/create channel");
	send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "PART <#chan1,#chan2,...> <optional:leaving_message>      :leave channel");
	send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "PRIVMSG <target1,target2,...> <text>                     :send a message");
	send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "QUIT                                                     :disconnect");

    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "NICK <new_nickname>                                      :change your nickname");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "MODE <#channel> <+/-k> <password>                        :set the channel password");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "MODE <#channel> <+/-i> <optional:password>               :set channel to invite only or public");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "MODE <#channel> <+/-t>                                   :set topic protection");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "MODE <#channel> <+/-o> <target_nick>                     :add or remove operator");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "MODE <#channel> <+/-l> <optional:limit>                  :set or remove user limit");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "KICK <#channel> <target_nick>                            :kick user from channel");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "INVITE <target_nick> <#channel>                          :invite user to channel");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "TOPIC <#channel> <new_topic>                             :get or set channel topic");
    send_reply(fd, RPL_YOUREOPER, {nickname, "*"}, "PING <server>                                            :ping the server to check connection");
    
	send_reply(fd, 706, {nickname, "*"}, "*** End of HELP ***\n");
	return 0;
}