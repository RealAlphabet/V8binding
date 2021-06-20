##
## V8, C Binding, 2021
## Makefile
##

SRC         =   src/v8.cc

OBJ         =   $(SRC:.cc=.o)

NAME        =   libv8c.so

CXXFLAGS    =   -Wall                   \
                -W                      \
                -Iinclude               \
                -Wno-unused-parameter   \
                -fPIC                   \
                -DV8_COMPRESS_POINTERS -O3

LDFLAGS     =   -shared                                                     \
                -lstdc++                                                    \
                -lpthread                                                   \
                -Wl,--whole-archive libs/libv8.a -Wl,--no-whole-archive     \
                -Wl,--exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all

all:    $(NAME)

$(NAME):$(OBJ)
	gcc -o $(NAME) $(OBJ) $(LDFLAGS)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re:     fclean all
