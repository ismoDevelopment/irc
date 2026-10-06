NAME = irc_server
TMP_DIR = tmp
SRC_DIR = src
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++17 -MMD -Iincl
SRCS = main.cpp Client.cpp Commands.cpp Channel.cpp utils.cpp Server.cpp
OBJS = $(addprefix $(TMP_DIR)/,$(SRCS:.cpp=.o))
DEPS = $(OBJS:.o=.d)

all: $(NAME)

$(TMP_DIR):
	mkdir $(TMP_DIR)

$(TMP_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(NAME): $(TMP_DIR) $(OBJS)
	$(CXX) -o $(NAME) $(OBJS)

clean:
	$(RM) $(OBJS)
	$(RM) $(DEPS)
	$(RM) -r $(TMP_DIR)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DEPS)
