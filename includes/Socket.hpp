#ifndef SOCKET_HPP
# define SOCKET_HPP

# include <unistd.h>     // For close()
# include <fcntl.h>      // For fcntl()
# include <sys/socket.h> // For socket(), bind(), listen(), accept()
# include <netinet/in.h> // For sockaddr_in, sockaddr_in6
# include <stdexcept>    // For std::runtime_error
# include <string>       // For string in error messages
# include <cstring>      // For strerror()
#include <memory>

class Socket
{
	private:
		int _fd;
		//TODO: should we disable copy and assignment to prevent double-closing the same FD??
		// Socket(const Socket&);
		// Socket& operator=(const Socket&);

	public:
		Socket();
		// Constructor: Wraps an existing file descriptor (useful for accepted connections)
		explicit Socket(int fd);
		// Destructor: Ensures the socket is closed
		~Socket();

		int get_fd() const;
		// Set the socket to non-blocking mode
		void set_nonblocking();

		// void bind(int port);
		// void listen(int backlog);
		std::unique_ptr<Socket> accept() const; // Returns a new Socket object for the client (or unique_ptr)
		// ssize_t send(const void* buf, size_t len, int flags = 0);
		// ssize_t recv(void* buf, size_t len, int flags = 0);
};

#endif