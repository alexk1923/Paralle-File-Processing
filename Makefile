build:
	g++ -Wall -Wextra main.cpp -o tema1 -g -lpthread
	g++ -Wall -Wextra andreea.cpp -o andreea -g -lpthread
clean:
	rm tema1
