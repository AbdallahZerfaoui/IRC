# IRC

This C++17 version assumes the same core requirements: build an IRC server, use standard IRC clients, handle multiple connections in a single thread using `poll` (or equivalent), non-blocking I/O, no forking, specific commands, etc. The difference is *how* you implement these requirements using C++17's capabilities.

---

**Overview: The C++17 Grand Design (ft_irc)**

The fundamental goal remains the same: build a robust, multi-client IRC server following the IRC protocol, driven by a single-threaded I/O multiplexing loop (`poll` or equivalent) and non-blocking sockets.

The key difference in the C++17 design lies in **leveraging RAII (Resource Acquisition Is Initialization) extensively, primarily through smart pointers**, and utilizing **modern language features** for improved readability, expressiveness, and compile-time safety. Manual memory management is significantly reduced, and resource cleanup (especially for dynamic objects like clients and channels) becomes largely automatic.

**Key Learnings (C++17 Focus):**

*   **RAII and Smart Pointers (`std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`):** Managing object lifetimes and resource ownership safely and automatically.
*   **Modern C++ Syntax (`auto`, Range-based for loops, Lambdas, Structured Bindings):** Writing cleaner, more concise, and readable code.
*   **Standard Library Containers & Utilities (`std::unordered_map`, `std::optional`):** Choosing appropriate data structures for efficiency and clarity.
*   **Applying Modern C++ to System Programming:** Integrating modern C++ practices with lower-level C APIs for networking.

This C++17 approach demonstrates a mature understanding of modern C++ paradigms applied to system-level tasks, which is highly valuable in a professional context.

**Detailed Step-by-Step: The C++17 Build (Challenges)**

Let's translate the C++98 challenges into the C++17 landscape. The structure is similar, but the *implementation details* within each step change significantly.

---

**Challenge 0: Laying the Foundation (Environment & Makefile, C++17 Edition)**

*   **Goal:** Set up your project environment for C++17 and create a compliant Makefile.
*   **Why it's a Challenge:** Ensuring your build system is configured correctly for C++17 is the starting point. Good argument parsing remains essential.
*   **Steps:**
    1.  Create your project directory.
    2.  Ensure your compiler (g++ or clang++) supports C++17.
    3.  Create your `Makefile`.
    4.  Implement the required rules: `$(NAME)` (builds `ircserv`), `all`, `clean`, `fclean`, `re`.
    5.  Add the specified compiler flags (`-Wall`, `-Wextra`, `-Werror`) and **replace `-std=c++98` with `-std=c++17`**.
    6.  Create placeholder files (`main.cpp`, `Server.hpp`, `Server.cpp`, `Client.hpp`, `Channel.hpp`, etc.).
    7.  Add basic command-line argument parsing for `<port>` and `<password>` in `main()`. Leverage C++17 features like `std::stoi` with proper error handling (e.g., `try-catch std::invalid_argument, std::out_of_range`) for parsing the port.
    8.  Test the Makefile (`make`, `make clean`, `make fclean`, `make re`, `make all`).
*   **Portfolio Tip:** Using `-std=c++17` is the first signal. Correctly handling argument parsing with C++17's standard library features shows a preference for type-safe and standard approaches over C-style `atoi` and error checks.

---

**Challenge 1: Opening the Gates (Socket Setup, C++17 Edition)**

*   **Goal:** Create the main server socket, bind it, start listening, and make it non-blocking.
*   **Why it's a Challenge:** This still involves C system calls, so the core logic (`socket`, `setsockopt`, `bind`, `listen`) is similar. The C++17 difference comes in how you might wrap the socket file descriptor.
*   **Steps:**
    1.  Inside your `Server` class, implement the logic using `socket()`, `setsockopt()`, `bind()`, `listen()`.
    2.  Use `fcntl()` to make the *listening socket* non-blocking immediately.
    3.  **C++17 Enhancement:** Consider creating a simple RAII wrapper class for file descriptors (or specifically, sockets). This class would hold the `int fd` and call `close(fd)` in its destructor. Your `Server` class could then store `SocketRAII listening_socket;` instead of a raw `int`. While simple, it's a small demonstration of RAII even for C resources.
    4.  Implement robust error handling for all system calls. Throw C++ exceptions or return `std::optional` (less common for constructor failures) if initialization fails.
    5.  Test: Compile and run. Check `netstat`.

*   **Portfolio Tip:** Even simple RAII wrappers for C resources show an understanding of the pattern and a desire for resource safety beyond manual `close()` calls scattered in the code.

---

**Challenge 2: The Watchtower (Implementing the `poll` Loop, C++17 Edition)**

*   **Goal:** Create the central event loop using `poll()` to monitor the listening socket.
*   **Why it's a Challenge:** The `poll` system call itself is C and remains the same. The C++17 benefits are primarily QoL features for managing the `pollfd` vector and iterating results.
*   **Steps:**
    1.  Create your main server loop (e.g., `Server::run()`).
    2.  Manage a `std::vector<pollfd>`.
    3.  Add the listening socket's file descriptor to the vector with `POLLIN`.
    4.  Call `poll()`. Handle `EINTR`.
    5.  **C++17 Enhancement:** Use **range-based for loops** to iterate through the `pollfd` vector after `poll` returns. Use `auto&` for convenience.
    6.  Check `revents`. If `POLLIN` on the listening socket, prepare to accept.
    7.  Implement error handling for `poll()`.
    8.  Test: Run the server, check CPU usage.

*   **Portfolio Tip:** Range-based for loops are a simple but clear sign of using modern C++ iteration patterns.

---

**Challenge 3: Greeting Guests (Accepting Connections, C++17 Edition)**

*   **Goal:** Accept incoming connections and integrate new client sockets into the loop, managing them with smart pointers.
*   **Why it's a Challenge:** Accepting the socket is C. Making it non-blocking is C. The *major* C++17 change is how you store the client object.
*   **Steps:**
    1.  When `POLLIN` on the listening socket, loop on `accept()` until `EAGAIN`/`EWOULDBLOCK`.
    2.  Handle `accept()` errors.
    3.  Make the *new client socket* non-blocking using `fcntl()`. Handle errors.
    4.  **C++17 Major Change:** Create a `Client` class instance. Store your clients in a container using **`std::unique_ptr`**. For example, `std::map<int, std::unique_ptr<Client>> clients;` where the key is the client's socket file descriptor. Use `std::make_unique<Client>(new_socket_fd)` to create the client object and its smart pointer.
    5.  Store the `unique_ptr` in your `clients` map.
    6.  Add the new client's file descriptor to your `pollfd` vector with `POLLIN`.
    7.  Test: Connect with an IRC client or `nc`. Server should not crash. `clients` map should grow.
*   **Portfolio Tip:** Using `std::unique_ptr` for client objects in a container is the hallmark of C++17 resource management here. This shows a deep understanding of object ownership and leveraging RAII for automatic cleanup when clients disconnect (removing the `unique_ptr` from the map will delete the `Client` object).

---

**Challenge 4: The Conversation (Reading and Writing Data, C++17 Edition)**

*   **Goal:** Implement reading and writing, handling non-blocking I/O and buffering, leveraging C++17 features.
*   **Why it's a Challenge:** `recv`/`send` are C. Buffering logic is conceptually similar. C++17 helps with managing the data within buffers and the send queue.
*   **Steps:**
    1.  `Client` class includes a receive buffer (`std::string` or `std::vector<char>`) and a send queue (`std::vector<std::string>`).
    2.  In the `poll` loop, when `POLLIN` on a client socket:
        *   Get the `Client*` from the `std::unique_ptr` in your map (e.g., `clients[fd].get()`).
        *   Call `recv()`, append data to receive buffer.
        *   Handle `recv` return values (`>0`, `0`, `-1` with `EAGAIN`/`EWOULDBLOCK` or other errors).
        *   If client disconnected (0 or error), remove the client's entry from the `std::map<int, std::unique_ptr<Client>>`. The `unique_ptr` destructor will automatically delete the `Client` object. Close the socket manually or via an RAII wrapper within `Client`. Remove the `pollfd`.
    3.  Extract complete commands (`\r\n` terminated) from the receive buffer. **C++17 Enhancement:** Use `std::string::find`, `substr`, and potentially `std::string_view` when processing buffer contents without copying.
    4.  Implement `sendMessage(Client* client, const std::string& message)`: add `message + "\r\n"` to the client's send queue. Use **`std::move(message)`** if the original message object is temporary or you won't use it afterward, potentially improving efficiency.
    5.  In the `poll` loop, manage `POLLOUT` events based on the send queue. Use range-based for loops to iterate through clients.
    6.  When `POLLOUT` on a client socket:
        *   Call `send()`, sending from the front of the send queue.
        *   Handle `send` return values (`>0`, `-1` with `EAGAIN`/`EWOULDBLOCK` or other errors).
        *   If data was sent, remove the sent portion from the send queue. Use string manipulation.
        *   Update `pollfd` `events` based on send queue status.
    7.  Test: Send/receive data using `nc`. Verify buffering and message boundaries.
*   **Portfolio Tip:** Demonstrates managing buffers and send queues. Using `std::string_view` (if applicable) and `std::move` are indicators of performance awareness in modern C++. Crucially, correctly handling client disconnection by erasing from the `unique_ptr` map showcases RAII cleanup.

---

**Challenge 5: Breaking the Code (IRC Protocol Parsing, C++17 Edition)**

*   **Goal:** Parse raw command strings into a structured format.
*   **Why it's a Challenge:** Parsing rules are IRC-specific. C++17 QoL features make the parsing code cleaner.
*   **Steps:**
    1.  Refine command extraction from the receive buffer (Challenge 4).
    2.  Create a `Command` struct/class (`prefix`, `name`, `parameters`).
    3.  Implement `parseCommand(const std::string& raw_command)`:
        *   Parse the string according to RFC rules.
        *   **C++17 Enhancement:** Use `std::string_view` for parts of the raw command string during parsing to avoid copies where possible. Use range-based for loops, `auto`, and structured bindings (e.g., if iterating a map to find a delimiter and getting both iterator and value) where appropriate.
    4.  Modify `recv` handling to call the parser.
    5.  Basic dispatcher printing parsed command details.
    6.  Test: Send various IRC commands with `nc`, verify correct parsing output.

*   **Portfolio Tip:** Clean, modern C++ parsing logic using features like `string_view` and structured bindings (if applicable) is a good sign.

---

**Challenge 6: Knowing Your Guests (Client Registration & State, C++17 Edition)**

*   **Goal:** Implement client registration (PASS, NICK, USER) and state management using modern features.
*   **Why it's a Challenge:** State tracking logic is the same. C++17 helps with storing and looking up clients efficiently.
*   **Steps:**
    1.  Enhance `Client` class: `nickname`, `username`, flags (`is_registered`, etc.).
    2.  Implement `PASS`, `NICK`, `USER` handlers.
    3.  **C++17 Enhancement:** Maintain a way to look up clients by nickname. While iterating the `std::map<int, std::unique_ptr<Client>>` is an option, a faster approach is a secondary index. Use `std::unordered_map<std::string, Client*>` to map nicknames to raw pointers of the `Client` objects stored in your primary `unique_ptr` map. Update this map whenever a nickname changes or a client disconnects/registers.
    4.  Implement registration logic: check `PASS`, `NICK`, `USER` status.
    5.  Upon registration: send RPL messages. Add the client to a collection of *registered* clients (perhaps just rely on the `is_registered` flag and the main client map/unordered_map).
    6.  Handle errors like ERR_NOTREGISTERED.
    7.  Implement `QUIT`. When a client quits, remove them from all relevant structures (`pollfd` vector, main `clients` map, nickname lookup map). The `unique_ptr` handles the object deletion.
    8.  Test: Register clients with your reference client. Test invalid registration attempts. Test quitting.
*   **Portfolio Tip:** Using `std::unordered_map` for fast lookups by nickname demonstrates choosing an efficient data structure. Correctly managing the lifetime of objects owned by `unique_ptr` when using raw pointers (`Client*`) in secondary indices is important.

---

**Challenge 7: Forming Tribes (Channel Management, C++17 Edition)**

*   **Goal:** Implement channel creation, joining, leaving, and messaging within channels, leveraging C++17 for channel/member management.
*   **Why it's a Challenge:** Managing collections of channels and client memberships requires careful design.
*   **Steps:**
    1.  Create a `Channel` class: `name`, `topic`, modes, lists of `clients` (members), `operators`.
    2.  **C++17 Enhancement:** Store channels in `std::map<std::string, std::unique_ptr<Channel>>` or `std::unordered_map<std::string, std::unique_ptr<Channel>>`.
    3.  Inside the `Channel` class, store members and operators using **raw pointers (`Client*`)** or **`std::weak_ptr<Client>`**. *Do not* use `unique_ptr` or `shared_ptr` here, as the `Client` objects are owned elsewhere (by the server's main client map). Using raw pointers is simpler if you guarantee that channel lists are cleaned up *before* the `Client` objects are deleted from the server's main list (e.g., on client `QUIT` or disconnection, remove them from all channels first). `weak_ptr` is safer but requires clients to be stored in `shared_ptr`s initially, which might be overkill if `unique_ptr` works for server ownership. Let's go with `Client*` for simplicity mirroring C++98, but note the safety implications vs. `weak_ptr`. Use `std::vector<Client*>` or `std::unordered_set<Client*>` for members/operators.
    4.  Implement `JOIN`, `PART`, `PRIVMSG` (to channel) handlers.
    5.  **C++17 Enhancement:** Use range-based for loops (`auto* client_ptr : channel.get_members()`) for iterating through channel members to send messages.
    6.  Update `QUIT` to remove client from all channels *before* the client object is destroyed.
    7.  Test: Multi-client joining/parting channels, sending channel messages.
*   **Portfolio Tip:** Correctly handling the pointer types and ownership model when referencing `Client` objects (owned by `unique_ptr` in the server) from `Channel` objects (owned by `unique_ptr` in the server) is crucial. Using `unordered_map` for channel lookup shows efficiency.

---

**Challenge 8: Private Talks (Private Messages & Identity, C++17 Edition)**

*   **Goal:** Implement direct messaging and identity changes (NICK).
*   **Why it's a Challenge:** Finding clients by nickname and updating state across the server.
*   **Steps:**
    1.  Implement `PRIVMSG` (to user) handler. Use your nickname-to-`Client*` `unordered_map` for efficient lookup. Handle ERR_NOSUCHNICK gracefully (perhaps using `std::optional` in the lookup function's return type, though a raw pointer or `nullptr` is also common).
    2.  Implement `NICK` handler (after registration):
        *   Validate new nick, check for uniqueness using the nickname `unordered_map`.
        *   If valid, update the client's nickname field.
        *   **Crucially:** Update your nickname `unordered_map` (remove old entry, add new entry).
        *   Send `NICK` message to relevant clients.
        *   Handle errors.
    3.  Handle `USER` after registration -> ERR_ALREADYREGISTRED.
    4.  Test: Send private messages between clients. Change nicknames and observe updates.
*   **Portfolio Tip:** Efficient lookup using `unordered_map` and correctly managing its state when nicknames change are key C++17 improvements.

---

**Challenge 9: The Channel Authorities (Operator Privileges & Commands, C++17 Edition)**

*   **Goal:** Introduce operator status and implement KICK, INVITE, TOPIC, respecting privileges.
*   **Why it's a Challenge:** Tracking operators and enforcing permissions.
*   **Steps:**
    1.  In `Channel`, use `std::unordered_set<Client*>` for operators.
    2.  Implement `TOPIC`, `INVITE`, `KICK` handlers.
    3.  Check if the sender is in the channel's operator set (`operators.count(client_ptr)` or `operators.find(client_ptr) != operators.end()`).
    4.  **C++17 Enhancement:** Use `auto` and range-based for loops when iterating through command parameters or channel members.
    5.  Test: Test operator-only commands as both operator and regular user.

*   **Portfolio Tip:** Using `unordered_set` for operator tracking provides efficient checks (average O(1)).

---

**Challenge 10: Setting the Rules (MODE Command, C++17 Edition)**

*   **Goal:** Implement the complex `MODE` command for channels.
*   **Why it's a Challenge:** Parsing the mode string and its parameters is intricate.
*   **Steps:**
    1.  In `Channel`, track modes (`invite_only`, `topic_restricted`, `key`, `user_limit`). Use `std::string` for the key.
    2.  **C++17 Enhancement:** Use `std::optional<int>` for `user_limit` if you want to explicitly represent "no limit set" vs. a limit of 0. Use `std::optional<std::string>` for the key if it's not always set.
    3.  Implement `MODE` handler:
        *   Parse the channel name and mode string.
        *   Check operator privilege.
        *   Parse the mode string carefully (+/- flags, mode characters, parameters). **C++17 Enhancement:** Use a state machine approach or loop through the mode string using `auto` and process character by character. Parsing parameters might involve structured bindings if iterating parameter lists.
        *   Implement logic for `i`, `t`, `k`, `o`, `l` modes.
        *   For `+k` and `+l`, parse the parameter string. Use `std::stoi` with error handling for the limit number.
        *   For `+o` and `-o`, find the target user in the channel (using `Client*`). Handle errors like user not found or not in channel.
        *   Send `MODE` message to channel members reflecting changes.
    4.  Update `JOIN` and `TOPIC` to respect the implemented modes.
    5.  Test: Extensive testing of setting and removing various modes, including those with parameters, and verifying the behavior change (joining with/without key/invite, changing topic as non-operator).
*   **Portfolio Tip:** Handling this complex command parsing and state management using clean C++17 code (parsing loops, `optional` if used, clear state updates) is impressive.

---

**Challenge 11: The Final Polish (Robustness, Error Handling, Cleanup, C++17 Edition)**

*   **Goal:** Make your server resilient and ensure proper resource management *primarily* through RAII.
*   **Why it's a Challenge:** Even with smart pointers, system resources (file descriptors) still need care, and logical errors need handling.
*   **Steps:**
    1.  **Comprehensive Error Handling:** Continue to check system calls. For logical errors (invalid commands, permissions), send appropriate IRC error replies. Leverage C++ exception handling for exceptional, unrecoverable errors (e.g., socket creation failure), but maybe stick to return codes/status objects for common command errors as is typical in server loops.
    2.  **Resource Management (RAII is Key):** Your primary resources (`Client`, `Channel` objects) are managed by `std::unique_ptr`. When you erase a client from the `std::map<int, std::unique_ptr<Client>>`, the `Client` object is automatically deleted. Ensure the `Client` object's destructor calls `close()` on its socket file descriptor (or wrap the socket fd in a custom RAII class *within* `Client`). Ensure `Channel` objects are removed from their map when empty (if desired) or on server shutdown, and their `unique_ptr` destructor handles cleanup.
    3.  **Memory Management:** Focus on ensuring *all* dynamic allocations go through smart pointers (`std::make_unique`, `std::make_shared`) or are managed by standard containers holding values directly. Raw pointers (`Client*` in channels or nickname map) should *only* be non-owning views into objects owned elsewhere by smart pointers.
    4.  **Handle Client Disconnects:** The logic of removing the `pollfd` and erasing the entry from the `clients` `unique_ptr` map is robust and handles memory cleanup automatically.
    5.  **Handle Server Shutdown:** Catch signals (`sigaction`). In the signal handler (or a function called by it), gracefully shut down: close the listening socket. Iterate through the `clients` map and close each client socket (this will trigger their disconnect handling in the main loop). The `std::map` containing `unique_ptr`s will automatically delete all `Client` objects when the map itself is destroyed (e.g., when the `Server` object goes out of scope in `main`). Channels will also be cleaned up by their owning map.
    6.  **Edge Case Testing:** Same as C++98, but trust that `std::string`, `std::vector`, `std::map`, `std::unordered_map`, `std::unique_ptr` handle their internal memory management correctly. Focus testing on your logic, protocol parsing, and state transitions.
    7.  **Code Style:** Adhere to consistent C++17 style. Use `auto`, range-based for loops, etc. where they improve clarity.
    8.  **Final Testing:** Use reference client, multi-client scenarios, `lsof` (to verify socket closure), and memory leak checkers (valgrind, if compatible with your C++17 setup, though smart pointers greatly reduce leak potential).
*   **Portfolio Tip:** This is where the C++17 benefits are most visible in code stability and safety. A server using smart pointers correctly for object lifetime management is significantly more robust against memory errors than a raw-pointer C++98 version. This demonstrates modern, safe C++ practices.

---

**Bonus Challenges (C++17 Edition):**

*   **Challenge B1: File Transfer (DCC SEND):** Requires creating new sockets. You'd use RAII wrappers or smart pointers for these new client-to-client sockets as well. Managing these connections within your `poll` loop adds complexity.
*   **Challenge B2: The Bot:** Implementing a bot as a separate client process/thread would interact with your server just like any other client, sending IRC commands.

---

**C++17 Portfolio Impact Summary:**

By building this project with C++17, you move from demonstrating manual low-level control (C++98) to showcasing **modern C++ design patterns for safety, efficiency, and expressiveness**. You'd highlight:

*   **Automatic Resource Management:** Mastery of RAII and smart pointers for object lifetimes.
*   **Clean and Expressive Code:** Utilizing C++17 language features for readability.
*   **Efficient Data Structures:** Choosing appropriate standard library containers like `unordered_map`.
*   **Integration of Modern C++ with C APIs:** Successfully building on system-level networking calls using modern language constructs.
*   **Increased Robustness:** Building a server less prone to memory errors compared to a raw-pointer implementation.

This C++17 version allows you to demonstrate a higher level of C++ language proficiency and awareness of modern best practices, complementing the fundamental network programming concepts learned from the project's core requirements.

Again, remember this is a theoretical exercise for understanding. The actual 42 project demands C++98. Good luck with the challenging (but rewarding!) C++98 implementation!
