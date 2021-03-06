C_FLAGS=-pedantic --std=c99 -Wall -Werror -pg -O3 -Wextra -fpic

SOURCE=$(wildcard *.c **/*.c)
OBJ=$(SOURCE:.c=.o)

all: $(OBJ)

clean: 
	rm -rf *.o
	rm -f fas

%.o: %.c
	gcc -c $(C_FLAGS) $< -o $@

venv:
	virtualenv venv
	venv/bin/pip install colorama

test: fas.so venv
	venv/bin/python tests.py

fas: $(OBJ)
	gcc -g -o fas permutations.o fas_tournament.o fas.o optimisation_table.o population.o -lm -O3

fas.so: $(OBJ)
	gcc -g --shared -o fas.so permutations.o fas_tournament.o optimisation_table.o population.o -lm -O3
