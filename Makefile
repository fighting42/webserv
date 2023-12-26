NAME		=	webserv

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98

SRCS		=	srcs/main.cpp \
				srcs/Config.cpp \
				srcs/Request.cpp \
				srcs/Server.cpp \
				srcs/Webserv.cpp

OBJS		=	$(SRCS:.cpp=.o)

all		:	$(NAME)

$(NAME) :	$(OBJS)
			$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

clean   :	
			$(RM) $(OBJS)

fclean  :	clean
			$(RM) $(NAME)

re  	:	fclean all

.PHONY  :	all clean fclean re
