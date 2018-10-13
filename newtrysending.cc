#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//using namespace std;

int main() {
	std::cout << "ENTERING" << std::endl;
	
	int status;
	struct addrinfo hints;
	struct addrinfo *res;
	
	struct sockaddr_storage incomming_addr;
	socklen_t addr_size;
	
	int client_sock, newfd;
	
	int binding, connected, listening, accepting;
	
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
	
	/*binding = bind(client_sock, res->ai_addr, res->ai_addrlen);
	if(binding == -1) {
		std::cout << "ERROR at bind call " << errno << std::endl;
		exit(1);
	}*/
	
	connected = connect(client_sock, res->ai_addr, res->ai_addrlen);
	if(connected == -1) {
		std::cout << "ERROR at connect call " << errno << std::endl;
		exit(1);
	}
	
	/*listening = listen(client_sock, limit);
	if(listening == -1) {
		std::cout << "ERROR at listen call " << errno << std::endl;
		exit(1);
	}
	
	addr_size = sizeof(incomming_addr);
	
	newfd = accept(client_sock, (struct sockaddr *) &incomming_addr, &addr_size);
	if(newfd == -1) {
		std::cout << "ERROR at accept call " << errno << std::endl;
		exit(1);
	}*/
	
	char const *msg = "Hello World please work";
	int len, bytes_sent;
	len = strlen(msg);
	bytes_sent = send(client_sock, msg, len, 0);
	if(bytes_sent != len)
	{
		std::cout << "ERROR maybe at send " << errno << std::endl;
		exit(1);
	}
	else
	{
		std::cout << "Bytes Sent: " << bytes_sent << std::endl;
	}
	
	/*msg = "Message 2";
	bytes_sent = send(client_sock, msg, len, 0);
	if(bytes_sent != len)
	{
		std::cout << "ERROR maybe at send " << errno << std::endl;
		exit(1);
	}
	else
	{
		std::cout << "Bytes Sent: " << bytes_sent << std::endl;
	}*/
	
	
	freeaddrinfo(res);
	
	std::cout << "SUCCESS" << std::endl;
	return 0;
}