FLAGS=-Wextra -Wall -pedantic
COMPILER=g++

Trampoline:
	$(COMPILER) $(FLAGS) -o Trampoline trampoline.cpp

Clean:
	rm -f Trampoline