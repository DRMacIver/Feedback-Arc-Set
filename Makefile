C_FLAGS=-pedantic -Wall -Werror -g

SOURCE=$(wildcard *.cpp )
OBJ=$(SOURCE:.cpp=.o)

all: $(OBJ)

clean: 
	rm -rf $(OBJ)
	rm -f fas

%.o: %.cpp
	gcc -c $(C_FLAGS) $< -o $@

test: fas
	ruby tests.rb

fas: $(OBJ)
	gcc -g -o fas permutations.o fas_tournament.o fas.o -lm
