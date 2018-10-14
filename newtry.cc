#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <fcntl.h>

//using namespace std;

int main() {
	std::cout << "ENTERING" << std::endl;
	
	int status;
	struct addrinfo hints;
	struct addrinfo *res;
	
	struct sockaddr_storage incomming_addr;
	socklen_t addr_size;
	
	int client_sock, newfd;
	
	int binding, connected, listening, accepting, recieved;
	
	int limit = 10;
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	if((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0)
	{
		std::cout << "ERROR at getaddrinfo " << gai_strerror(status) << std::endl;
		exit(1);
	}
	
	//do things
	//client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
	client_sock = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
	if(client_sock == -1)
	{
		std::cout << "ERROR at socket call " << errno << std::endl;
		exit(1);
	}
	
	/*int flags = fcntl(client_sock, F_GETFL);
	int make_non_blocking = fcntl(client_sock, F_SETFL, flags | O_NONBLOCK); // fcntl(client_sock, F_GETFL, 0), O_NONBLOCK);
	if(make_non_blocking == -1){
		std::cout << "ERROR at fcntl call " << errno << std::endl;
	}*/
	
	//fcntl(client_sock, F_SETFL, O_NONBLOCK);
	
	binding = bind(client_sock, res->ai_addr, res->ai_addrlen);
	if(binding == -1) {
		std::cout << "ERROR at bind call " << errno << std::endl;
		exit(1);
	}
	
	/*connected = connect(client_sock, res->ai_addr, res->ai_addrlen);
	if(connected == -1) {
		std::cout << "ERROR at connect call " << errno << std::endl;
		exit(1);
	}*/
	
	listening = listen(client_sock, limit);
	if(listening == -1) {
		std::cout << "ERROR at listen call " << errno << std::endl;
		exit(1);
	}
	
	addr_size = sizeof(incomming_addr);
	
	std::cout << "a" << std::endl;
	
	newfd = accept(client_sock, (struct sockaddr *) &incomming_addr, &addr_size);
	if(newfd == -1) {
		std::cout << "ERROR at accept call " << errno << std::endl;
		exit(1);
	}
	
	std::cout << "b" << std::endl;
	
	//int other_id;
	
	char *buf = new char[1024];
	int len;
	bool incomming = true;
	std::string rec = "";
	
	while(incomming) {
		recieved = recv(newfd, buf, sizeof(buf), 0);
		if (recieved == -1) {
			std::cout << "ERROR at recv call " << errno << std::endl;
			incomming = false;
			exit(1);
		}
		else if (recieved == 0)
		{
			//the sender is done sending
			incomming = false;
			
			std::cout << "recieved string: " << rec << std::endl;
			rec = "";
		}
		else {
			//other_id = getpeername(newfd, (struct sockaddr *) &incomming_addr, &addr_size);
			//std::cout << "Other ID is: " << other_id << std::endl;
			incomming = true;
			std::cout << "recieved bytes: " << recieved << std::endl;
			//char const * rec = buf[0]
			for (int i = 0; i < recieved; ++i) {
				rec += buf[i];
			}
		}
	}
	
	
	
	
	
	
	freeaddrinfo(res);
	//close(client_sock);
	
	std::cout << "SUCCESS" << std::endl;
	return 0;
}