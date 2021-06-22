##
## V8, C Binding, 2021
## Makefile
##

SRC         =   $(shell find src -name *.cc)

OBJ         =   $(SRC:.cc=.o)

NAME        =   libv8c.so

CXXFLAGS    =   -Wall                   \
                -W                      \
                -Wno-unused-parameter   \
                -Iinclude               \
                -fPIC                   \
                -DV8_COMPRESS_POINTERS  \
                -g

LDFLAGS     =   -shared                                                     \
                -Llibs                                                      \
                -lstdc++                                                    \
                -lpthread                                                   \
                -lv8                                                        \
                -ldl                                                        \
                -lm

all:    $(NAME)

$(NAME):$(OBJ)
	gcc -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re:     fclean all
