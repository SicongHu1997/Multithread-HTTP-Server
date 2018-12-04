server:server.c server.h
	gcc server.c -o server -lpthread
clean:
	rm -f server