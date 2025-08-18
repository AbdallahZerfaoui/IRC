# Colors
RED = \033[31m
GREEN = \033[32m
RESET = \033[0m

# Parameters
DEFAULT_PORT = 6667
DEFAULT_PASSWORD = my_password

# Compiler
CXX = c++
# CXX = clang++ # Uncomment this line and comment the above if you prefer clang

HEADER_DIR = ./includes
SRCS_DIR = ./src
OBJSDIR = ./objs

# Compiler flags
# For C++17
CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -g # -Wpedantic -fdiagnostics-color=always -fdiagnostics-show-option -fno-diagnostics-show-caret
# For C++98 (as per project, but you asked for C++17 for this example)
# CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -g

# Executable name
NAME = ircserv

# Source files
# For Block 1, we have these:
# SRCS = main.cpp src/Server.cpp src/Socket.cpp src/Client.cpp

# SRCS = Server.cpp Socket.cpp Client.cpp Channel.cpp ParsedMessage.cpp
# SRCS := main.cpp $(addprefix $(SRCS_DIR)/, $(SRCS))

# SRCS = Server.cpp Socket.cpp Client.cpp Channel.cpp ParsedMessage.cpp main.cpp
# SRCS := $(addprefix $(SRCS_DIR)/, $(SRCS))
SRCS := src/main.cpp \
		src/utils/CaseMap.cpp src/utils/Reply.cpp src/utils/Split.cpp src/utils/Validate.cpp \
		src/net/Socket.cpp src/net/connection/Accept.cpp src/net/connection/Data.cpp src/net/connection/Disconnect.cpp \
		src/model/Channel.cpp src/model/Client.cpp \
		src/core/Dispatch.cpp src/core/ParsedMessage.cpp src/core/Server.cpp \
		src/command/auth/Nick.cpp src/command/auth/Pass.cpp src/command/auth/User.cpp \
		src/command/channel/Join.cpp src/command/channel/Part.cpp src/command/channel/Topic.cpp src/command/channel/Kick.cpp src/command/channel/Invite.cpp \
		src/command/messaging/Privmsg.cpp src/command/messaging/Ping.cpp \
		src/command/utility/Quit.cpp \
		src/command/registry.cpp
# SRCS := $(shell find $(SRCS_DIR) -type f -name '*.cpp') #TODO: check if we are allowed to use find


# Object files (derived from SRCS)
# OBJS = $(SRCS:.cpp=.o)
OBJS = $(SRCS:%.cpp=$(OBJSDIR)/%.o)

# Default rule: make all
all: art $(NAME) success_message

# Rule to build the executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -I$(HEADER_DIR) -o $(NAME)

# Rule to compile .cpp files into .o files
$(OBJSDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(HEADER_DIR) -c $< -o $@

# Clean rule: remove object files
clean:
	@echo "${RED}Cleaning up...${RESET}"
	rm -f $(OBJS)
	rm -rf $(OBJSDIR)

# Fclean rule: remove object files and the executable
fclean: clean
	rm -f $(NAME)

# Re rule: fclean and then build all
re: fclean all

success_message:
	@echo "${RED}	------------------***༺ (${RED}${GREEN}IRC Compiled Successfully!${RED})༻***------------------\n\033[0m"

start_server: re
	@echo "${GREEN}Starting server...${RESET}"
	./$(NAME) ${DEFAULT_PORT} ${DEFAULT_PASSWORD}

val:
	@echo "${GREEN}Running Valgrind...${RESET}"
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes --trace-children=yes -s --log-file=valgrind.log --verbose ./$(NAME) 6669 a

art:
	@echo "${GREEN}IIIIIIIIIIRRRRRRRRRRRRRRRRR           CCCCCCCCCCCCC             SSSSSSSSSSSSSSS EEEEEEEEEEEEEEEEEEEEEERRRRRRRRRRRRRRRRR   VVVVVVVV           VVVVVVVVEEEEEEEEEEEEEEEEEEEEEERRRRRRRRRRRRRRRRR   "
	@echo "I::::::::IR::::::::::::::::R       CCC::::::::::::C           SS:::::::::::::::SE::::::::::::::::::::ER::::::::::::::::R  V::::::V           V::::::VE::::::::::::::::::::ER::::::::::::::::R  "
	@echo "I::::::::IR::::::RRRRRR:::::R    CC:::::::::::::::C          S:::::SSSSSS::::::SE::::::::::::::::::::ER::::::RRRRRR:::::R V::::::V           V::::::VE::::::::::::::::::::ER::::::RRRRRR:::::R "
	@echo "II::::::IIRR:::::R     R:::::R  C:::::CCCCCCCC::::C          S:::::S     SSSSSSSEE::::::EEEEEEEEE::::ERR:::::R     R:::::RV::::::V           V::::::VEE::::::EEEEEEEEE::::ERR:::::R     R:::::R"
	@echo "  I::::I    R::::R     R:::::R C:::::C       CCCCCC          S:::::S              E:::::E       EEEEEE  R::::R     R:::::R V:::::V           V:::::V   E:::::E       EEEEEE  R::::R     R:::::R"
	@echo "  I::::I    R::::R     R:::::RC:::::C                        S:::::S              E:::::E               R::::R     R:::::R  V:::::V         V:::::V    E:::::E               R::::R     R:::::R"
	@echo "  I::::I    R::::RRRRRR:::::R C:::::C                         S::::SSSS           E::::::EEEEEEEEEE     R::::RRRRRR:::::R    V:::::V       V:::::V     E::::::EEEEEEEEEE     R::::RRRRRR:::::R "
	@echo "  I::::I    R:::::::::::::RR  C:::::C                          SS::::::SSSSS      E:::::::::::::::E     R:::::::::::::RR      V:::::V     V:::::V      E:::::::::::::::E     R:::::::::::::RR  ${RESET}"
	@echo "  I::::I    R::::RRRRRR:::::R C:::::C                            SSS::::::::SS    E:::::::::::::::E     R::::RRRRRR:::::R      V:::::V   V:::::V       E:::::::::::::::E     R::::RRRRRR:::::R "
	@echo "  I::::I    R::::R     R:::::RC:::::C                               SSSSSS::::S   E::::::EEEEEEEEEE     R::::R     R:::::R      V:::::V V:::::V        E::::::EEEEEEEEEE     R::::R     R:::::R"
	@echo "  I::::I    R::::R     R:::::RC:::::C                                    S:::::S  E:::::E               R::::R     R:::::R       V:::::V:::::V         E:::::E               R::::R     R:::::R"
	@echo "  I::::I    R::::R     R:::::R C:::::C       CCCCCC                      S:::::S  E:::::E       EEEEEE  R::::R     R:::::R        V:::::::::V          E:::::E       EEEEEE  R::::R     R:::::R"
	@echo "II::::::IIRR:::::R     R:::::R  C:::::CCCCCCCC::::C          SSSSSSS     S:::::SEE::::::EEEEEEEE:::::ERR:::::R     R:::::R         V:::::::V         EE::::::EEEEEEEE:::::ERR:::::R     R:::::R"
	@echo "I::::::::IR::::::R     R:::::R   CC:::::::::::::::C          S::::::SSSSSS:::::SE::::::::::::::::::::ER::::::R     R:::::R          V:::::V          E::::::::::::::::::::ER::::::R     R:::::R"
	@echo "I::::::::IR::::::R     R:::::R     CCC::::::::::::C          S:::::::::::::::SS E::::::::::::::::::::ER::::::R     R:::::R           V:::V           E::::::::::::::::::::ER::::::R     R:::::R"
	@echo "IIIIIIIIIIRRRRRRRR     RRRRRRR        CCCCCCCCCCCCC           SSSSSSSSSSSSSSS   EEEEEEEEEEEEEEEEEEEEEERRRRRRRR     RRRRRRR            VVV            EEEEEEEEEEEEEEEEEEEEEERRRRRRRR     RRRRRRR"
	@echo "${RED}                                                                                                             by The Greatest Team Ever (2025)                                                  ${RESET}"

# Phony targets (targets that don't represent files)
.PHONY: all clean fclean re success_message art start_server
