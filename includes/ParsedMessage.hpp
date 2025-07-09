#ifndef PARSED_MESSAGE_HPP
# define PARSED_MESSAGE_HPP

# include <string>
# include <vector>
# include <iostream>
# include <sstream>

struct ParsedMessage
{
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
	ParsedMessage();
	ParsedMessage(const std::string& line);
	ParsedMessage(const std::string& p, const std::string& c, const std::vector<std::string>& ps);
};

# endif