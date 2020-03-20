#include "ftpClient.h"

int main(int argc, char const *argv[])
{
	ftpClient client;

	client.establish_socket_client();
	client.establish_connection(argv[2], argv[3]); 
	client.fileRecvProgram(argv[3]);
	client.close_connection();
	return 0;
}

