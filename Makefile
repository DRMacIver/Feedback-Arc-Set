C_FLAGS=-pedantic --std=c99 -Wall -Werror -g

SOURCE=$(wildcard *.c **/*.c)
OBJ=$(SOURCE:.c=.o)

all: $(OBJ)

clean: 
	rm -rf $(OBJ)
%.o: %.c
	gcc -c $(C_FLAGS) $< -o $@

test: $(OBJ)
	gcc -o test $(OBJ)
