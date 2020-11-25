##
## LUMZ APP, 2020
## Makefile
##

SRC		=	src/server/server.c		\
			src/smtp/utils.c		\
			src/smtp/globals.c		\
			src/smtp/connection.c	\
			src/smtp/packets.c		\
			src/main.c

OBJ		=	$(SRC:.c=.o)

NAME	=	lightsmtp

CFLAGS	=	-Wall											\
			-W												\
			-Wno-unused-parameter							\
			-I include										\
			$$(pkg-config --libs --cflags libmongoc-1.0)	\
			-g

all:	$(NAME)

$(NAME):$(OBJ)
	gcc -o $(NAME) $(OBJ) $(CFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re:		fclean all
