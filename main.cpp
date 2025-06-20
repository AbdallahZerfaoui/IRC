#include "includes/Server.hpp"
#include <iostream>
#include <cstdlib> // For exit()

int main(int argc, char** argv)
{
	if (argc != 3) 
	{
		std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
		return 1;
	}

	int port;
	try 
	{
		port = std::stoi(argv[1]);
	} 
	catch (const std::invalid_argument& e) 
	{
		std::cerr << "Invalid port number format." << std::endl;
		return 1;
	} 
	catch (const std::out_of_range& e) 
	{
		std::cerr << "Port number out of range." << std::endl;
		return 1;
	}

	std::string password = argv[2];
    
	//TODO: should we validate the password??
	
	// >>> ADDED FOR SIGNAL HANDLING <<<
	Server::setup_signal_handlers();

	try 
	{
		Server server(port, password); // Create the server object
		server.run(); // Start the server's main loop
	}
	// Catch any exceptions thrown during setup or runtime
	catch (const std::exception& e) 
	{
		std::cerr << "Server error: " << e.what() << std::endl;
		return 1;
	}
	// The server loop is infinite, so this point is theoretically unreachable
	// unless the loop is broken or a signal is caught.
	return 0;
}