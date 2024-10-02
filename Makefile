NAME		= ircserv

ifeq ($(shell uname), Linux)
COMPILE	= clang++
else
COMPILE 	= c++
endif

CFLAGS		= -Wall -Wextra -Werror -std=c++98 -pedantic -MMD

SRC_DIR 	= src/
OBJ_DIR 	= obj/

INCS		= -Iinc/

SRC			= main.cpp \
			  Server.cpp \
			  Channel.cpp \
			  User.cpp \
			  Parser.cpp \
			  Log.cpp

OBJ			= $(SRC:.cpp=.o)

SRCS		= $(addprefix $(SRC_DIR), $(SRC))
OBJS		= $(addprefix $(OBJ_DIR), $(OBJ))
DEPS		= $(OBJS:.o=.d)

all:$(NAME)

-include $(DEPS)

$(OBJ_DIR)%.o:$(SRC_DIR)%.cpp
	mkdir -p $(OBJ_DIR)
	$(COMPILE) $(CFLAGS) $(INCS) -o $@ -c $<

$(NAME):$(OBJS)
	$(COMPILE) $(CFLAGS) $(INCS) -o $@ $(OBJS)

clean:
	rm -rf $(OBJ_DIR)

fclean:clean
	rm -rf $(NAME)

re:fclean all

.PHONY:all clean fclean re
