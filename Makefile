NAME		=	webserv

CXXFLAGS	=	-Wall -Wextra -Werror -std=c++98 #-g -fsanitize=address

SRCS		=	srcs/main.cpp \
				srcs/Client.cpp \
				srcs/Config.cpp \
				srcs/Event.cpp \
				srcs/EventMethod.cpp \
				srcs/EventUtils.cpp \
				srcs/Kqueue.cpp \
				srcs/Request.cpp \
				srcs/RequestUtils.cpp \
				srcs/Response.cpp \
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
