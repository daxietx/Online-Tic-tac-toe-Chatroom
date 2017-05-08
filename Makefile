CFLAG=g++ -Wall -std=c++11 -pedantic
server: server4.cpp
	$(CFLAG) -lpthread server4.cpp -o server
client: client2.cpp
	$(CFLAG) client2.cpp -o client
demo: server client
clean:
	rm -f server client

	