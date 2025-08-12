#include "Server.hpp"
#include "ParsedMessage.hpp"

// Quick fix to make the irssi client work
// The irssi server sends a CAP LS command before the PASS command to list capabilities
int Server::handle_cap(int fd, const ParsedMessage& msg)
{
    if (msg.params.empty())
        return 0;

    const std::string sub = msg.params[0];
	// LS is used to list capabilities
	// REQ is used to request capabilities
	// END is used to end the capability negotiation
    if (sub == "LS") {
        std::ostringstream r;
		r << ":" << _hostname << " CAP * LS :\r\n";
        _clients.at(fd).queue_send(r.str());
        _clients.at(fd).try_flush();
    }
	else if (sub == "REQ") {
        std::string requested = (msg.params.size() >= 2 ? msg.params[1] : "");
        std::ostringstream r;
		r << ":" << _hostname << " CAP * NAK " << requested << "\r\n";
        _clients.at(fd).queue_send(r.str());
        _clients.at(fd).try_flush();
    } else if (sub == "END") {
    }
    return 0;
}
