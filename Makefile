C_FLAGS=-pedantic -Wall -Werror -g --std=c++0x

SOURCE=$(wildcard *.cpp )
OBJ=$(SOURCE:.cpp=.o)

all: $(OBJ)

clean: 
	rm -rf $(OBJ)
	rm -f fas

%.o: %.cpp
	g++ -c $(C_FLAGS) $< -o $@

test: fas
	ruby tests.rb

fas: $(OBJ)
	g++ -g -o fas permutations.o fas_tournament.o fas.o -lm
