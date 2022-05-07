Habitat : main.o
	c++ -o Habitat main.o -pthread -lsfml-system -lsfml-graphics -lsfml-window

main.o : main.cpp
	c++ -c main.cpp

clean :
	rm -f main.o Habitat
