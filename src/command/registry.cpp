#include "Server.hpp"


const std::unordered_map<std::string, Server::CommandHandler> Server::handlers = {
	{"PASS", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.parse_pass(fd, msg);
    }},
	{"NICK", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.parse_nick(fd, msg);
    }},
	{"USER", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.parse_user(fd, msg);
    }},
    {"PRIVMSG", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_privmsg(fd, msg);
    }},
	{"PART", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_part(fd, msg);
    }},
	{"JOIN", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_join(fd, msg);
    }},
	{"HELP", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_help(fd, msg);
    }},
	{"CHANNELS", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_channels(fd, msg);
    }},
	{"QUIT", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_quit(fd, msg);
    }},
	{"PING", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_ping(fd, msg);
    }},
	{"MODE", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_mode(fd, msg);
    }},
	{"KICK", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_kick(fd, msg);
    }},
	{"INVITE", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_invite(fd, msg);
    }},
	{"TOPIC", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_topic(fd, msg);
    }},
	{"CAP", [](Server& srv, int fd, const ParsedMessage& msg) {
        return srv.handle_cap(fd, msg);
    }},
};
