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
	if (!token.empty() && token[0] == ':')
	{
		ss >> token;
		prefix = token.substr(1);
	}

	// Getting the command
	ss >> command;

	// Getting the parameters
	bool trailing = false;
	while (ss >> token)
	{
		if (token[0] == ':' && !trailing) {
			trailing = true;
			std::string rest;
			std::getline(ss, rest);
			params.push_back(token.substr(1) + rest);
			break;
		}
		else
		{
			params.push_back(token);
		}
	}
}

ParsedMessage::ParsedMessage(const std::string& p, const std::string& c, const std::vector<std::string>& ps) : prefix(p), command(c), params(ps) {}