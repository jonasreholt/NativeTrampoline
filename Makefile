FLAGS=-Wextra -Wall -pedantic
COMPILER=g++

Trampoline:
	$(COMPILER) $(FLAGS) -o Trampoline trampoline.cpp

TrampolineDEBUG:
	$(COMPILER) $(FLAGS) -g -o Trampoline trampoline.cpp

Clean:
	rm -f Trampoline