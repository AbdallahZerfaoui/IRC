# Colors
RED = \033[31m
GREEN = \033[32m
RESET = \033[0m

# Compiler
CXX = c++
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
all: art $(NAME)

# Rule to build the executable
$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule: remove object files
clean:
	@echo "${RED}Cleaning up...${RESET}"
	rm -f $(OBJS)

# Fclean rule: remove object files and the executable
fclean: clean
	rm -f $(NAME)

# Re rule: fclean and then build all
re: fclean all

success_message:
	@echo "${RED}	------------------***༺ (${RED}${GREEN}IRC Compiled!${})༻***------------------\n\033[0m"

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
.PHONY: all clean fclean re