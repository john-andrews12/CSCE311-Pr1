#include "using_socket.h"

string socket_path = "\0hidden";

int main(int argc, char *argv[]) {
	cout << "Entering" << endl;
	
	struct sockaddr addr;
	char buf[100];
	int fd,rc;
	
	if (argc > 1) {
		socket_path = argv[1];
	}
	
	if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		cout << "Socket Error" << endl;
		exit(-1);
	}
	
	memset(&addr, 0, sizeof(addr));
	addr.sa_family = AF_UNIX;
	
	
	
	cout << "Exiting" << endl;
}