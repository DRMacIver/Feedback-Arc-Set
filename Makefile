C_FLAGS=-pedantic --std=c99 -Wall -Werror -g

SOURCE=$(wildcard *.c **/*.c)
OBJ=$(SOURCE:.c=.o)

all: $(OBJ)

clean: 
	rm -rf $(OBJ)
%.o: %.c
	gcc -c $(C_FLAGS) $< -o $@

test: $(OBJ)
	gcc -g -o test tournament.o permutations.o fas_tournament.o test.o

fas: $(OBJ)
	gcc -g -o fas tournament.o permutations.o fas_tournament.o fas.o
