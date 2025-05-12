# IRC

**Overview: The Grand Design (ft_irc)**

Your mission is to build a server application (`ircserv`) that speaks the Internet Relay Chat protocol using C++98. This server will listen on a specific port, accept connections from standard IRC clients (like HexChat, irssi, mIRC, etc. - you'll choose one as your reference), manage multiple client connections *simultaneously*, and handle core IRC commands like joining channels, sending messages, changing nicknames, and enforcing channel operator privileges.

The core technical challenge lies in handling multiple clients *without* using traditional blocking I/O or heavyweight processes/threads (forking is forbidden). Instead, you'll use *non-blocking sockets* and a single *I/O multiplexing* call (`poll` or equivalent) to manage *all* client interactions and the listening socket within a single event loop.

This isn't just about sending bytes; it's about understanding state (client registration, channel membership, modes), parsing a text-based protocol, and managing resources (sockets, client data structures) in a concurrent environment.

**Key Learnings & Portfolio Value:**

*   **Network Programming Fundamentals:** Sockets, TCP/IP, bind, listen, accept, send, recv.
*   **Non-Blocking I/O:** Understanding why and how to use it.
*   **I/O Multiplexing (`poll`/`select`/`kqueue`/`epoll`):** The core technique for handling many connections efficiently in a single thread.
*   **Event Loop Design:** Structuring your server around waiting for and processing network events.
*   **Protocol Parsing:** Turning raw byte streams into structured commands.
*   **State Management:** Tracking clients, channels, and their states.
*   **Concurrency (Event-Driven):** Handling multiple interactions that happen "at the same time" from the server's perspective.
*   **Robustness:** Handling errors, disconnections, partial data reads, and malformed input gracefully.
*   **C++ System Programming:** Using C++ features to manage low-level system resources (file descriptors/sockets).
*   **Adherence to Standards:** Implementing a protocol according to its specification (IRC RFCs are your friends!).
*   **Code Structure & Organization:** Designing classes for clients, channels, the server itself.

This project clearly demonstrates your ability to build complex, performance-sensitive network applications from the ground up.

**Detailed Step-by-Step: The Build (Challenges)**

Let's break down the project into manageable challenges. Remember to **test thoroughly** after each significant step!

---

**Challenge 0: Laying the Foundation (Environment & Makefile)**

*   **Goal:** Set up your project environment and a compliant Makefile.
*   **Why it's a Challenge:** Getting the build system and basic structure right from the start saves headaches later. Adhering to specific compiler flags and C++98 standard is crucial.
*   **Steps:**
    1.  Create your project directory.
    2.  Choose your C++ development environment and ensure you can compile with a C++98 compliant compiler (like g++ or clang++).
    3.  Create your `Makefile`.
    4.  Implement the required rules: `$(NAME)` (which should build `ircserv`), `all` (alias for `$(NAME)`), `clean`, `fclean`, `re`.
    5.  Ensure the `$(NAME)` rule *does not relink* if only source files change; it should only recompile changed files and then link.
    6.  Add the specified compiler flags (`-Wall`, `-Wextra`, `-Werror`, `-std=c++98`) to your compilation rules.
    7.  Create placeholder `.cpp` and `.h`/`.hpp` files (e.g., `main.cpp`, `Server.hpp`, `Server.cpp`) to test the Makefile.
    8.  Add basic command-line argument parsing for `<port>` and `<password>` in `main()`. Handle missing arguments and invalid port numbers (e.g., not a number, outside valid range).
    9.  Test the Makefile (`make`, `make clean`, `make fclean`, `make re`, `make all`).
*   **Portfolio Tip:** A clean, correct, and efficient Makefile demonstrates attention to detail and understanding of the build process. Good error handling for command-line arguments is a basic mark of robustness.

---

**Challenge 1: Opening the Gates (Socket Setup)**

*   **Goal:** Create the main server socket, bind it to the specified port, and start listening for incoming connections.
*   **Why it's a Challenge:** This is the absolute entry point for clients. Getting the socket setup correct is non-negotiable. You also need to immediately make this socket non-blocking.
*   **Steps:**
    1.  Inside your `Server` class (or equivalent), implement the logic to create a socket using `socket()`. Choose the address family (AF_INET for IPv4, AF_INET6 for IPv6, or AF_UNSPEC for either) and socket type (SOCK_STREAM for TCP).
    2.  Use `setsockopt()` with `SO_REUSEADDR` (and `SO_REUSEPORT` if available and appropriate) to allow restarting the server quickly after a crash or shutdown without waiting for sockets to time out. This is best practice.
    3.  Prepare the server address structure (`sockaddr_in` or `sockaddr_in6`). Bind the socket to the port using `bind()`.
    4.  Start listening for incoming connections using `listen()`. Choose a reasonable backlog value (e.g., 10-20).
    5.  **Crucially:** Make the *listening socket* non-blocking *immediately* after creation using `fcntl(fd, F_SETFL, O_NONBLOCK)`. Handle potential errors from `fcntl`.
    6.  Store the listening socket file descriptor.
    7.  Implement error handling for all these system calls. If any fail, the server cannot start.
    8.  Test: Compile and run. It won't do anything visible yet, but it shouldn't crash, and you can use `netstat -tulnp` (Linux) or equivalent to see if your port is being listened on.
*   **Portfolio Tip:** Demonstrates core understanding of TCP/IP server setup. Using `SO_REUSEADDR` shows awareness of common development issues. Correct non-blocking setup is foundational for the rest of the project.

---

**Challenge 2: The Watchtower (Implementing the `poll` Loop)**

*   **Goal:** Create the central event loop using `poll()` to monitor the listening socket for new connections.
*   **Why it's a Challenge:** This is the single point of control for all I/O. Understanding how `pollfd` works and integrating the listening socket is the first step in building the event-driven architecture.
*   **Steps:**
    1.  In your `Server` class, create a loop that will be the heart of your server (e.g., a `run()` method).
    2.  Inside the loop, you'll manage a `std::vector<pollfd>` (or similar C++98 container) to hold the file descriptors you are monitoring.
    3.  Add the listening socket's file descriptor to this vector. Set the `events` field for the listening socket's `pollfd` to `POLLIN` (interested in read events - meaning new connections).
    4.  Call `poll()`, passing the `pollfd` vector, its size, and a timeout (e.g., -1 for infinite, or a small positive value for periodic tasks, though infinite is fine for now). Handle potential interruptions (`errno == EINTR`).
    5.  After `poll()` returns, iterate through the `pollfd` vector. Check the `revents` field for each entry.
    6.  If the listening socket's `revents` includes `POLLIN`, it means there's an incoming connection ready to be accepted. You will handle this in the next challenge.
    7.  Implement error handling for `poll()`.
    8.  Test: Run the server. It should enter the poll loop and wait. You can try connecting with `nc 127.0.0.1 <port>` (it will hang because the server isn't *accepting* yet, but `poll` should report an event). The server should not consume 100% CPU (it should be blocked in `poll`).
*   **Portfolio Tip:** Mastery of `poll` (or equivalent) is a major signal of network programming skill. This is the engine of your concurrent server.

---

**Challenge 3: Greeting Guests (Accepting Connections)**

*   **Goal:** Accept incoming connections detected by the `poll` loop and integrate the new client sockets into the loop.
*   **Why it's a Challenge:** You need to accept connections *efficiently* when `poll` signals they are ready, *and* make the new client sockets non-blocking immediately.
*   **Steps:**
    1.  In the `poll` loop, when you detect `POLLIN` on the listening socket, call `accept()`. Since the listening socket is non-blocking, `accept` might return `EAGAIN` or `EWOULDBLOCK` if the event was spurious or another thread/process (not applicable here, but good habit) accepted it first. You should probably loop on `accept` until it returns -1 with `EAGAIN`/`EWOULDBLOCK`, accepting all pending connections.
    2.  `accept()` returns a *new* file descriptor for the connected client socket. Handle errors from `accept()`.
    3.  **Crucially:** Make this *new client socket* non-blocking using `fcntl(client_fd, F_SETFL, O_NONBLOCK)`. Handle errors.
    4.  Create a data structure (e.g., a `Client` class instance) to represent this connected client. Store the client's socket file descriptor in it. You'll add more state later (nickname, buffers, etc.). You'll need a way to map file descriptors to `Client` objects (e.g., `std::map<int, Client>`).
    5.  Add the new client's file descriptor to your `pollfd` vector. Set its initial `events` to `POLLIN` (interested in reading data from the client).
    6.  Optionally, send a basic message to the client right away (e.g., a simple "Welcome!" or the start of the IRC registration sequence like server name/version, although the full sequence comes later). Use `send()` and handle its non-blocking nature (see Challenge 4).
    7.  Test: Run the server. Use an IRC client or `nc` to connect. The server should not crash. The `pollfd` vector should grow, and you should see log messages indicating accepted connections.
*   **Portfolio Tip:** Shows ability to handle dynamic resources (new connections) and correctly apply non-blocking properties to them.

---

**Challenge 4: The Conversation (Reading and Writing Data)**

*   **Goal:** Implement reading data from clients and writing data back, handling the complexities of non-blocking I/O and partial reads/writes.
*   **Why it's a Challenge:** `recv` might give you only *part* of a command, and `send` might not send your whole message at once or might fail if the buffer is full. You *must* handle this buffering correctly. This is where many non-blocking projects stumble.
*   **Steps:**
    1.  Modify your `Client` class to include a receive buffer (e.g., `std::string` or `std::vector<char>`) and a send queue/buffer (e.g., `std::vector<std::string>` or a single string for the current outgoing message).
    2.  In the `poll` loop, when you detect `POLLIN` on a *client* socket:
        *   Call `recv()` on the socket. Use a temporary buffer to read data into.
        *   Handle the return value:
            *   `> 0`: Data received. Append it to the client's receive buffer.
            *   `0`: Client disconnected gracefully. Remove the client from your list, close the socket (`close()`), and remove its `pollfd` from the vector.
            *   `-1` with `errno` being `EAGAIN` or `EWOULDBLOCK`: No data available *right now*. This is *normal* for non-blocking sockets; just continue.
            *   `-1` with any other `errno`: An actual error occurred. Treat as disconnection: remove, close, remove `pollfd`.
    3.  After appending data to the receive buffer, check if it contains a full IRC command (terminated by `\r\n`). You might have multiple commands or just part of one. Implement a method to extract complete commands from the buffer (removing them as they are processed).
    4.  Implement a function `sendMessage(Client* client, const std::string& message)` which appends `message + "\r\n"` to the client's send queue.
    5.  In the `poll` loop, after processing events, iterate through all clients. If a client's send queue is *not* empty, ensure its `pollfd` has `POLLOUT` in its `events` field. If the send queue *is* empty, ensure `POLLOUT` is *not* in its `events` field (to avoid unnecessary `poll` wakeups).
    6.  When you detect `POLLOUT` on a client socket:
        *   Attempt to send data from the *front* of the client's send queue using `send()`. `send` might send less than requested.
        *   Handle the return value:
            *   `> 0`: Data sent. Remove the sent portion from the send queue/buffer.
            *   `-1` with `errno` being `EAGAIN` or `EWOULDBLOCK`: Cannot send data right now. This is normal. `poll` will signal `POLLOUT` again later when the buffer is ready. Do *not* remove the data from the queue.
            *   `-1` with any other `errno`: Error. Treat as disconnection.
        *   If the send queue becomes empty after a successful `send`, update the client's `pollfd` to remove `POLLOUT`.
    7.  Add error handling for `recv` and `send`.
    8.  Test: Run the server. Connect with `nc`. Type some text and press enter. The server should receive it (log it for debugging). If you send a full line ending in `\r\n` (IRC clients usually handle this, but `nc` might need manual crafting or flags), it should be processed. Try sending a message *to* the client from the server's side (a hardcoded "hello" for testing). It should be sent and received by `nc`. Test sending a long message. Test sending multiple messages quickly.
*   **Portfolio Tip:** Successfully implementing receive buffering and send queuing for non-blocking sockets is a major achievement. It demonstrates a solid grasp of asynchronous I/O challenges. This is often the most complex part for beginners.

---

**Challenge 5: Breaking the Code (IRC Protocol Parsing)**

*   **Goal:** Parse the raw command strings received from clients into a structured format your server can understand.
*   **Why it's a Challenge:** IRC has a specific, albeit simple, text format (`[prefix] COMMAND params :[trailing]`). You need to reliably extract these parts.
*   **Steps:**
    1.  Refine the logic from Challenge 4 that extracts full commands from the receive buffer (`\r\n` terminated).
    2.  Create a `Command` class or struct. It should hold fields for: `prefix` (optional), `name` (the command string like "NICK", "USER", "JOIN"), and `parameters` (a list or vector of strings). The last parameter might be the "trailing" part starting with ':'.
    3.  Implement a function or method (e.g., `parseCommand(const std::string& raw_command)`) that takes a raw IRC command string and populates a `Command` object. Follow the IRC RFC 1459 or 2812 specification for message format. Pay attention to spaces, leading colons, and how parameters are delimited.
    4.  Modify your `recv` handling (Challenge 4) to call this parser for each extracted full command string.
    5.  Implement a basic command dispatcher (e.g., a function that takes a `Client*` and a `Command*`). For now, just print the parsed command details.
    6.  Test: Use `nc` to send various malformed and correctly formed IRC commands (e.g., `NICK guest`, `JOIN #test :password`, `:irc.example.com PRIVMSG #channel :Hello there!`). Your server should parse them correctly and print the components.
*   **Portfolio Tip:** Demonstrates ability to implement a parser for a real-world text-based protocol. Shows attention to detail by adhering to format specifications.

---

**Challenge 6: Knowing Your Guests (Client Registration & State)**

*   **Goal:** Implement the initial client registration flow (PASS, NICK, USER) and manage client state.
*   **Why it's a Challenge:** Clients aren't fully usable until they've identified themselves. You need to track their state and validate their input against server rules.
*   **Steps:**
    1.  Enhance your `Client` class: add fields for `nickname`, `username`, `realname`, `password_given` (bool), `nick_given` (bool), `user_given` (bool), `is_registered` (bool), etc.
    2.  Implement the command handler logic (the dispatcher from Challenge 5) for the `PASS`, `NICK`, and `USER` commands.
    3.  `PASS`: Check if the command parameter matches the server's password. Set `password_given`. If password is wrong, you can disconnect immediately (though RFCs allow delaying).
    4.  `NICK`: Validate the proposed nickname format. Check if it's already in use by another *registered* or *registering* client. If valid and unique, store it temporarily, set `nick_given`. Send appropriate error replies (ERR_NICKNAMEINUSE, ERR_ERRONEUSNICKNAME, etc.) or success (none explicitly required by RFC on success *before* registration).
    5.  `USER`: Parse and store the username and real name. Set `user_given`.
    6.  After processing *any* command, check if the client has now set `password_given`, `nick_given`, and `user_given`. If so, mark `is_registered` as true. This is the point of successful registration.
    7.  Upon successful registration:
        *   Send the welcome messages (RPL_WELCOME, RPL_YOURHOST, RPL_CREATED, RPL_MYINFO). Look up their format in the RFC.
        *   Add the client to a list of *registered* clients in your `Server` class.
    8.  Implement error handling for commands received *before* registration (most commands except PASS, NICK, USER should result in ERR_NOTREGISTERED).
    9.  Handle `QUIT`: Implement this command to gracefully disconnect a client at any time.
    10. Test: Use your reference IRC client. Connect, type the password, nickname, and username in the correct order. You should see the welcome messages in the client and be connected. Test invalid passwords, duplicate nicknames, invalid nicknames, sending commands before registering. Test typing `QUIT`.
*   **Portfolio Tip:** Demonstrates state machine implementation (the registration flow), validation logic, and adherence to protocol-specific replies (RPL/ERR messages). Managing collections of clients is fundamental.

---

**Challenge 7: Forming Tribes (Channel Management)**

*   **Goal:** Implement channel creation, joining, leaving, and messaging within channels.
*   **Why it's a Challenge:** You need to manage a list of channels, each with its own state (topic, members, operators, modes) and implement the logic for users interacting within them.
*   **Steps:**
    1.  Create a `Channel` class: fields for `name`, `topic`, `key`, `mode_flags` (e.g., bools or flags for `i`, `t`, `k`, `o`, `l`), lists of `clients` (registered in this channel), and `operators` (subset of clients).
    2.  In your `Server` class, maintain a list of active channels (e.g., `std::map<string, Channel>`).
    3.  Implement the `JOIN` command handler:
        *   Parse channel name(s) and key(s). Handle joining multiple channels in one command (`JOIN #chan1,#chan2`).
        *   Find or create the channel.
        *   Check channel modes (`+k` key required, `+i` invite only - handle the latter fully in Challenge 9/10).
        *   Add the client to the channel's list of members. If the channel was just created, make the joining client an operator.
        *   Send the `JOIN` message (`:nick!user@host JOIN #channel`) to *all* clients currently in the channel (including the one who just joined).
        *   Send the channel topic (RPL_TOPIC) and the list of users in the channel (RPL_NAMREPLY followed by RPL_ENDOFNAMES) to the joining client.
        *   Handle errors: ERR_BADCHANNELKEY, ERR_CHANNELISFULL, ERR_INVITEONLYCHAN, etc.
    4.  Implement the `PART` command handler: Remove client from channel, notify others in the channel (`:nick!user@host PART #channel`), handle ERR_NOTONCHANNEL. If the channel becomes empty, maybe delete it (optional, but cleans up resources).
    5.  Implement the `PRIVMSG` command handler for *channels*: If the target starts with '#' or '&', find the channel. If the client is in the channel, send the message (`:nick!user@host PRIVMSG #channel :message`) to *all other clients* in that channel. Handle ERR_NOSUCHCHANNEL, ERR_CANNOTSENDTOCHAN.
    6.  Modify the `QUIT` command handler: Before disconnecting a client, make them PART all channels they are in, notifying other channel members.
    7.  Test: Use multiple IRC clients. Have them connect, register, join the same channel. Send messages. They should all see each other's messages. Have clients join/part. Have clients quit. Test joining with/without a key on a channel that requires one (you'll add the key mode setting later, for now maybe hardcode one).
*   **Portfolio Tip:** This is where concurrent state management becomes more complex (multiple clients interacting with shared channel objects). Demonstrates managing collections of custom objects and implementing core IRC functionality. Broadcast messaging is a common network pattern.

---

**Challenge 8: Private Talks (Private Messages & Identity)**

*   **Goal:** Implement direct client-to-client messaging and handle nickname/username changes after registration.
*   **Why it's a Challenge:** You need to look up clients by nickname and correctly update identity information across the server.
*   **Steps:**
    1.  Implement the `PRIVMSG` command handler for *users*: If the target is a nickname (doesn't start with # or &), find the target client by their nickname in your list of registered clients. If found, send the message (`:sender_nick!user@host PRIVMSG target_nick :message`) only to that client. Handle ERR_NOSUCHNICK.
    2.  Implement the `NICK` command handler *after* registration:
        *   Parse the new nickname.
        *   Validate the format and check for uniqueness among *all* currently connected clients (registered or not, though typically only registered clients can change nick).
        *   If valid and unique, update the client's nickname.
        *   Send a `NICK` message (`:old_nick!user@host NICK :new_nick`) to *all* clients who share a channel with this client *and* to the client themselves (or just the client, depending on exact client behavior expectations - sending to shared channels is standard).
        *   Handle errors: ERR_NONICKNAMEGIVEN, ERR_ERRONEUSNICKNAME, ERR_NICKNAMEINUSE.
    3.  Implement the `USER` command handler *after* registration: According to RFCs, `USER` can only be sent once during registration. Receiving it again should usually result in ERR_ALREADYREGISTRED.
    4.  Test: Use two IRC clients. Connect and register them. Send a private message from one to the other. It should only appear in the target client's window. Have clients change their nicknames; this change should be reflected in channel user lists and future messages. Test invalid private message targets or invalid nicknames.
*   **Portfolio Tip:** Demonstrates indexing/searching through collections of objects (finding a client by nickname), implementing state changes that affect multiple parts of the system (nickname change), and handling commands based on client registration status.

---

**Challenge 9: The Channel Authorities (Operator Privileges & Commands)**

*   **Goal:** Introduce channel operator status and implement the KICK, INVITE, and TOPIC commands, respecting operator privileges.
*   **Why it's a Challenge:** You need to track which clients are operators in which channels and enforce permissions for specific actions.
*   **Steps:**
    1.  In your `Channel` class, ensure you have a way to designate and track operators (e.g., a `std::set<Client*>` or a flag on the `Client` within the channel's member list). The first client to join/create a channel is typically an operator.
    2.  Implement the `TOPIC` command handler:
        *   Parse channel name and the potential new topic string.
        *   Find the channel. Handle ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL.
        *   If no topic string is given, send the current topic (RPL_TOPIC) and who set it (RPL_TOPICWHOTIME - look up the format).
        *   If a topic string *is* given:
            *   Check if the client sending the command is a channel operator *OR* if the channel's 't' mode is *not* set (you'll implement 't' mode fully in Challenge 10, for now assume 't' is *on* by default meaning only operators can change topic). Handle ERR_CHANOPRIVSNEEDED.
            *   Update the channel's topic.
            *   Notify *all* clients in the channel about the new topic (`:nick!user@host TOPIC #channel :new topic`).
    3.  Implement the `INVITE` command handler:
        *   Parse the nickname of the user to invite and the channel name.
        *   Find the channel. Handle ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL.
        *   Check if the client sending the command is a channel operator *OR* if the channel's 'i' mode is *not* set (handle 'i' mode fully in Challenge 10, assume 'i' is *off* by default). Handle ERR_CHANOPRIVSNEEDED.
        *   Find the target user by nickname (they don't have to be on the channel). Handle ERR_NOSUCHNICK.
        *   Send an `INVITE` message (`:nick!user@host INVITE target_nick #channel`) to the target user.
        *   Send RPL_INVITING to the inviter.
    4.  Implement the `KICK` command handler:
        *   Parse the channel name, the nickname of the user to kick, and an optional kick message.
        *   Find the channel. Handle ERR_NOSUCHCHANNEL, ERR_NOTONCHANNEL.
        *   Check if the client sending the command is a channel operator. Handle ERR_CHANOPRIVSNEEDED.
        *   Find the target user *in that channel*. Handle ERR_USERNOTINCHANNEL.
        *   Send a `KICK` message (`:nick!user@host KICK #channel target_nick :[message]`) to *all* clients in the channel (including the kicker, excluding the kicked).
        *   Remove the target user from the channel's member list.
    5.  Test: Use multiple clients. Join a channel. Make one client an operator (initially, the creator is). Test changing the topic as a regular user (should fail) and as an operator (should work). Test inviting a user (they should receive the INVITE). Test kicking a user as an operator. Test trying to kick someone not in the channel, or as a non-operator.
*   **Portfolio Tip:** Adds an important layer of access control and privilege management. Demonstrates more complex command logic involving finding multiple entities (channel + target user).

---

**Challenge 10: Setting the Rules (MODE Command)**

*   **Goal:** Implement the complex `MODE` command for channels, allowing operators to change channel settings.
*   **Why it's a Challenge:** The MODE command is versatile, combining adding/removing flags and requiring parameters based on the mode. It's arguably the most complex command to parse and handle.
*   **Steps:**
    1.  In your `Channel` class, use flags or boolean members to track the state of modes: `invite_only`, `topic_restricted`, `has_key`, `key` (string), `limit_set`, `user_limit` (int).
    2.  Implement the `MODE` command handler:
        *   Parse the channel name and the mode string (`+t-k+i`, etc.). Handle errors: ERR_NOSUCHCHANNEL.
        *   If no mode string is given, send the current modes (RPL_CHANNELMODEIS) and maybe creation time/operator creation time (RPL_CREATIONTIME, RPL_CHANNEL_URL, etc. - simplify as needed, RPL_CHANNELMODEIS is mandatory).
        *   If a mode string *is* given:
            *   Check if the client sending the command is a channel operator. Handle ERR_CHANOPRIVSNEEDED.
            *   Parse the mode string: iterate through it, keeping track of whether you are adding (`+`) or removing (`-`) modes.
            *   For each mode character (`i`, `t`, `k`, `o`, `l`):
                *   Handle `i` (Invite-only): Set/unset `invite_only` flag.
                *   Handle `t` (Topic restricted): Set/unset `topic_restricted` flag.
                *   Handle `k` (Key): If adding (`+k`), parse the key parameter from the command parameters. Set `has_key` and store the `key`. If removing (`-k`), clear the `key` and unset `has_key`. Handle ERR_NEEDMOREPARAMS if parameter is missing for `+k`.
                *   Handle `o` (Operator): If adding (`+o`), parse the nickname parameter. Find the target user *in the channel*. Make them an operator. If removing (`-o`), parse the nickname and remove operator status. Handle ERR_NEEDMOREPARAMS, ERR_USERSDONTMATCH (user not in channel), ERR_NOSUCHNICK (parameter is not a valid user nickname at all). Be careful about removing the last operator or allowing an operator to de-op themselves incorrectly.
                *   Handle `l` (User Limit): If adding (`+l`), parse the limit parameter (must be a number). Set `limit_set` and `user_limit`. If removing (`-l`), unset `limit_set`. Handle ERR_NEEDMOREPARAMS, potential errors if limit is not a number.
            *   After processing a set of mode changes from one MODE command, send a `MODE` message (`:nick!user@host MODE #channel +modes -modes [parameters]`) to all clients in the channel reflecting the final state change.
    3.  Update your `JOIN` command handler (Challenge 7) to fully check the `invite_only` and `has_key`/`key` modes.
    4.  Update your `TOPIC` command handler (Challenge 9) to fully check the `topic_restricted` mode.
    5.  Test: As an operator, test setting/removing each mode (`+i`, `-i`, `+t`, `-t`, `+k password`, `-k password`, `+l 10`, `-l`, `+o user`, `-o user`). Test trying to set modes as a non-operator. Test joining a channel that requires a key (with/without key), is invite-only (without invite), is full. Test being kicked and then invited back, then joining the invite-only channel.
*   **Portfolio Tip:** This demonstrates advanced protocol parsing, managing complex state using flags and parameters, and implementing fine-grained access control based on state and roles. A well-implemented MODE command is a strong indicator of attention to detail.

---

**Challenge 11: The Final Polish (Robustness, Error Handling, Cleanup)**

*   **Goal:** Make your server resilient to errors, handle edge cases, and ensure proper resource management.
*   **Why it's a Challenge:** A server needs to run reliably for a long time, handling unexpected input and network conditions without crashing or leaking resources. C++98 requires manual memory management and careful resource cleanup.
*   **Steps:**
    1.  **Comprehensive Error Handling:** Review *every* system call (`socket`, `bind`, `listen`, `accept`, `fcntl`, `poll`, `recv`, `send`, `close`) and *every* standard library operation (allocations, container access). If they can fail, add checks. Don't just crash. Log the error (`perror`, `strerror`) and take appropriate action (disconnect client, shut down server, etc.).
    2.  **Resource Management (RAII):** Since you don't have `unique_ptr`/`shared_ptr` in C++98, implement basic RAII classes or patterns for critical resources like sockets (file descriptors). A simple `SocketRAII` class that closes the file descriptor in its destructor is invaluable. Use this pattern for any resource that needs guaranteed cleanup.
    3.  **Memory Management:** If you use `new`, ensure every `new` has a corresponding `delete`. Using containers like `std::vector` and `std::map` for objects or pointers to objects requires careful destruction to avoid leaks. If using pointers, ensure you iterate and `delete` objects before the container goes out of scope or is cleared. Storing objects directly in containers like `std::map<int, Client>` is often simpler as the container handles destruction, but be mindful of copying/moving semantics (less complex in C++98).
    4.  **Handle Client Disconnects:** Ensure that when `recv` returns 0 or an error, you correctly remove the client's file descriptor from the `pollfd` list, close the socket, and clean up the `Client` object and any references to it (in channels, the main client list).
    5.  **Handle Server Shutdown:** Implement a graceful shutdown mechanism (e.g., catching SIGINT/SIGTERM using `signal`/`sigaction` - these are allowed). When shutting down, close the listening socket and *all* client sockets. This will cause `recv` on the client side to return 0, prompting clients to disconnect cleanly. Clean up all allocated resources.
    6.  **Edge Case Testing:** What happens if a client sends data faster than you read it? (Your receive buffer handles this). What if they send data slower? (Your `poll` loop waits). What if they send huge messages? (Check buffer sizes). What if they send garbage? (Parser should handle it gracefully, maybe disconnect them if it's malicious). What if a client disconnects mid-command? What if `send` returns `EAGAIN`? (Your send queue and `POLLOUT` handling handles this).
    7.  **MacOS `fcntl` constraint:** Double-check that you *only* use `fcntl(fd, F_SETFL, O_NONBLOCK)`. No other flags like `O_SYNC`, `O_ASYNC`, etc.
    8.  **Code Style and Comments:** Ensure your code is clean, readable, and well-commented, especially complex parts like parsing, the poll loop, and resource management.
    9.  **Final Testing:** Use your reference client for extensive testing. Test multi-client scenarios heavily. Use tools like `lsof -i :<port>` to check for open sockets (resource leaks) after clients connect/disconnect and after server shutdown. Use valgrind (if available on your system) to check for memory leaks.
*   **Portfolio Tip:** This is the phase that separates good projects from great ones. A robust server that doesn't crash and cleans up after itself is a strong indicator of a responsible and skilled developer. Proper C++98 resource management is particularly impressive given the lack of modern smart pointers.

---

**Challenge 12: Showcase and Share (Documentation & Submission)**

*   **Goal:** Prepare your project for review and future use.
*   **Why it's a Challenge:** Even the best code needs documentation to be useful to others (and your evaluators!).
*   **Steps:**
    1.  Write a comprehensive `README.md`. Include:
        *   Project title and brief description.
        *   How to build the project (`make`).
        *   How to run the server (`./ircserv <port> <password>`).
        *   How to connect with an IRC client (mention your chosen reference client).
        *   List of implemented mandatory features.
        *   List of implemented bonus features (if any).
        *   Any known limitations or specific OS considerations (like the MacOS `fcntl`).
    2.  Ensure all required files are in your repository (`Makefile`, source files - `.h`, `.hpp`, `.cpp`, `.tpp`, `.ipp`).
    3.  Remove any unnecessary temporary files or build artifacts from your repository *before* pushing.
    4.  Review the submission instructions on your campus and ensure you comply with any specific naming conventions or repository structure.
*   **Portfolio Tip:** A clear and professional README demonstrates communication skills. Presenting your work well is as important as writing it.

---

**Bonus Challenges (If Mandatory is PERFECT):**

*   **Challenge B1: File Transfer:** Implement DCC SEND. This requires opening a *new* socket connection between the two clients involved, orchestrated by the server. This adds peer-to-peer elements and handling multiple socket types/roles.
*   **Challenge B2: The Bot:** Implement a simple IRC bot connected as a client. The bot could respond to specific commands in a channel (e.g., `!time`, `!weather`, `!fortune`). This adds another layer of interaction and demonstrates how clients interact with your server programmatically.

---

**Portfolio Impact Summary:**

By successfully completing these challenges, you will have built a functional, robust, and efficient network server using low-level system calls and C++98. You'll demonstrate mastery of:

*   Concurrent programming paradigms (event-driven with `poll`).
*   Network I/O handling (non-blocking sockets, buffering, `send`/`recv`).
*   Protocol implementation and parsing.
*   State management in a multi-client environment.
*   System programming in C++.
*   Robust error handling and resource management.

This project is a significant undertaking and a powerful addition to any developer's portfolio, showcasing foundational skills highly valued in various software development fields.

Good luck, and enjoy the process of building your own piece of the internet!
