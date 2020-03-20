CC = g++
# g++ server.cpp ftpClient.cpp -o server -g
# g++ client.cpp ftpClient.cpp -o client -g
compile : server.o client.o ftpClient.o ftpServer.o multiServer.o
	$(CC) -g server.o ftpServer.o -o server -lpthread
	$(CC) -g client.o ftpClient.o -o client -lpthread
	$(CC) -g multiServer.o -o multiServer -lpthread


runcpdf : client
	./client get 127.0.0.1 file.pdf
runcmd  : client
	./client get 127.0.0.1 Lab2.md
runs : server
#	./server
	./multiServer


multiServer.o : multiServer.cpp ftpClient.h
	$(CC) -c -g multiServer.cpp -o multiServer.o
server.o : server.cpp ftpServer.h
	$(CC) -c -g server.cpp -o server.o
client.o : client.cpp ftpClient.h
	$(CC) -c -g client.cpp -o client.o
ftpClient.o : ftpClient.cpp ftpClient.h
	$(CC) -c -g ftpClient.cpp -o ftpClient.o
ftpServer.o : ftpServer.cpp ftpServer.h
	$(CC) -c -g ftpServer.cpp -o ftpServer.o 


clean : 
	rm *.o
	rm server client multiServer