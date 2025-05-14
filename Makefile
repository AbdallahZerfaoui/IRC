# Compiler
CXX = g++
# CXX = clang++ # Uncomment this line and comment the above if you prefer clang

# Compiler flags
# For C++17
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g
# For C++98 (as per project, but you asked for C++17 for this example)
# CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -g

# Executable name
NAME = ircserv

# Source files
# For Block 1, we have these:
SRCS = main.cpp src/Server.cpp src/Socket.cpp

# Object files (derived from SRCS)
OBJS = $(SRCS:.cpp=.o)

# Default rule: make all
all: $(NAME)

# Rule to build the executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule: remove object files
clean:
	rm -f $(OBJS)

# Fclean rule: remove object files and the executable
fclean: clean
	rm -f $(NAME)

# Re rule: fclean and then build all
re: fclean all

# Phony targets (targets that don't represent files)
.PHONY: all clean fclean re