#include "ftpServer.h"

int main(int argc, char const *argv[])
{
	ftpServer server;
	char filename[50];

	
	server.addr.sin_family = AF_INET;
	inet_pton(AF_INET, argv[3], &(server.addr.sin_addr));
	server.addr.sin_port = htons(atoi(argv[2]));
	
	server.establish_socket_server(atoi(argv[1])); // 从muticlientserver传过来的端口号  port

	server.accept(filename);

	server.fileSendProgram(filename);
	server.close_connection();
	
	return 0;
}
