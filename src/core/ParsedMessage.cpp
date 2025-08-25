#include "ParsedMessage.hpp"


ParsedMessage::ParsedMessage() : prefix(""), command(""), params() {}

ParsedMessage::ParsedMessage(const std::string& line)
{
	// Example line:
	// :alice!~user@localhost PRIVMSG #42 :Hello everyone, welcome to the channel!\r\n
	// {prefix}: ":alice!~user@localhost"
	// {command}: "PRIVMSG"
	// {Params}: {"#42", "Hello everyone, welcome to the channel!"}

	std::string token;
	std::istringstream ss(line);

	// Getting the prefix
	// if (!token.empty() && token[0] == ':')
	// {
	// 	ss >> token;
	// 	prefix = token.substr(1);
	// }

	// Getting the command
	ss >> command;

	for (size_t i = 0; i < command.size(); ++i)
		command[i] = std::toupper(static_cast<unsigned char>(command[i]));

	// Getting the parameters
	bool trailing = false;
    while (ss >> token) {
        if (!trailing && !token.empty() && token[0] == ':') {
            trailing = true;
            std::string rest;
            std::getline(ss, rest);
            if (!rest.empty() && rest[0] == ' ')
                rest.erase(0, 1);
			std::string tail = token.substr(1);
			if (!rest.empty()) tail += " " + rest;
			params.push_back(tail);
			break;
        } else {
            params.push_back(token);
        }
    }
}

ParsedMessage::ParsedMessage(const std::string& p, const std::string& c, const std::vector<std::string>& ps) : prefix(p), command(c), params(ps) {}