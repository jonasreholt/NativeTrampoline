FLAGS=-Wextra -Wall -pedantic
COMPILER=g++

Example: Clean
	$(COMPILER) $(FLAGS) -o example.exe example.cpp trampoline.h codeBuffer.h


Clean:
	rm -f *.exe