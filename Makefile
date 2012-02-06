C_FLAGS=-pedantic --std=c99 -Wall -Werror -g

SOURCE=$(wildcard *.c **/*.c)
OBJ=$(SOURCE:.c=.o)

all: $(OBJ)

clean: 
	rm -rf *.o
	rm -f fas

%.o: %.c
	gcc -c $(C_FLAGS) $< -o $@

test: fas
	ruby tests.rb

fas: $(OBJ)
	gcc -g -o fas permutations.o fas_tournament.o fas.o -lm
