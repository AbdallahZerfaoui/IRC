#include "Server.hpp"

// RFC1459 case mapping: {}|^ map to []\~ for nickname comparison
std::string Server::rfc1459_lowercase(const std::string& str)
{
    std::string result;
    result.reserve(str.size());
    
    for (char c : str)
    {
        if (c >= 'A' && c <= 'Z')
            result.push_back(c + 32); // Convert uppercase to lowercase
        else if (c == '{')
            result.push_back('[');
        else if (c == '}')
            result.push_back(']');
        else if (c == '|')
            result.push_back('\\');
        else if (c == '^')
            result.push_back('~');
        else
            result.push_back(c);
    }
    
    return result;
}

// Enhanced nickname duplicate check using RFC1459 case mapping
bool Server::is_duplicate_nickname(const std::string& nickname)
{
    if (nickname.empty())
        return false;
        
    std::string lowercase_nick = rfc1459_lowercase(nickname);
    
    for (const auto& client_pair : _clients)
    {
        std::string client_nick = client_pair.second.get_nickname();
        if (!client_nick.empty() && client_nick != "*" && 
            rfc1459_lowercase(client_nick) == lowercase_nick)
            return true;
    }
    
    return false;
}
