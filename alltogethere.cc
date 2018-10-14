#include <iostream>
#include <fstream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>

//using namespace std;
void ChildProcess1(int &child_sock, addrinfo *res_c);
void ParentProcessAfter(void);
std::string GetFourLetterRep(int bytes);
int FourLetterRepVal(std::string input);

struct semaphores {
	sem_t child_done;
	sem_t parent_done;
};

int main(int argc, char *argv[]) {
	std::cout << "ENTERING MAIN" << std::endl;
	
	if (argc != 2) {
		std::cout << "usage: ./Aprog <filepath>" << std::endl;
		exit(1);
	}
	
	std::string filepath = argv[1];
	
	
	pid_t c_pid, p_pid;
	
	p_pid = getpid();
	c_pid = fork();
	
	if (c_pid == -1) {
		exit(1);//error forking
	}
	else if (c_pid == 0) {
		std::cout << "ENTER CHILD" << std::endl;
		
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
		
		newfd = accept(client_sock, (struct sockaddr *) &incomming_addr, &addr_size);
		if(newfd == -1) {
			std::cout << "ERROR at accept call " << errno << std::endl;
			exit(1);
		}
		
		//int other_id;
		
		char *buf = new char[1024];
		int len;
		bool incomming = true;
		bool start = true;
		int string_size;
		std::string rec = "";
		
		while(incomming) {
			std::cout << "a" << std::endl;
			recieved = recv(newfd, buf, sizeof(buf), 0);
			std::cout << "b" << std::endl;
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
				int i = 0;
				if (start) {
					//assuming we read in at least 4
					for (int i = 0; i < 4; ++i) {
						rec += buf[i];
					}
					
					string_size = FourLetterRepVal(rec) + 4;
					//recieved -= 4;
					
					rec = "";
					start = false;
					i = 4;
				}
				
				incomming = true;
				std::cout << "recieved bytes: " << recieved << std::endl;
				//char const * rec = buf[0]
				for (i; i < recieved; ++i) {
					rec += buf[i];
				}
				
				string_size -= recieved;
				
				if(string_size <= 0) 
				{
					std::cout << "recieved string: " << rec << std::endl;
					break;
				}
			}
			std::cout << "rec so far: " << rec << std::endl;
		}
		
		
		freeaddrinfo(res);
		
		std::cout << "EXIT CHILD" << std::endl;
	}
	else {
		std::cout << "ENTER PARENT BEFORE" << std::endl;
		
		int status;
		struct addrinfo hints;
		struct addrinfo *res;
		
		struct sockaddr_storage incomming_addr;
		socklen_t addr_size;
		
		int client_sock, newfd;
		
		int binding, connected, listening, accepting;
		
		int limit = 10;
		
		//try to establish a connection to the child socket 
		bool connected_to_child = false;
		while (!connected_to_child) {
			//go through the process of making a socket and try to connect to it
			memset(&hints, 0, sizeof(struct addrinfo));
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;
			
			if((status = getaddrinfo(NULL, "3490", &hints, &res)) != 0)
			{
				std::cout << "ERROR at getaddrinfo " << gai_strerror(status) << std::endl;
				exit(1);
			}
			
			client_sock = socket(res->ai_family,res->ai_socktype,res->ai_protocol);
			if(client_sock == -1)
			{
				std::cout << "ERROR at socket call " << errno << std::endl;
				exit(1);
			}
			
			connected = connect(client_sock, res->ai_addr, res->ai_addrlen);
			if(connected == -1) {
				if(errno == 61)
				{
					//the socket simply isn't accepting yet
					close(client_sock);
				}
				else
				{
					//there was some other real error
					std::cout << "ERROR at connect call " << errno << std::endl;
					exit(1);
				}
			}
			else {
				connected_to_child = true;
			}
		}
		
		//char const *msg = "Hello World please work";
		std::string msg = "Hello World please work";
		int len, bytes_sent;
		len = msg.length();//strlen(msg);
		std::cout << "length found " << len << std::endl;
		msg = GetFourLetterRep(len) + msg;
		len += 4;//we just added four chars to the front of the string
		char const *msgfinal = msg.c_str();
		
		std::cout << "final message: " << msg << std::endl;
		
		//msg = GetFourLetterRep(len) + msg;
		bytes_sent = send(client_sock, msgfinal, len, 0);
		if(bytes_sent != len)
		{
			std::cout << "ERROR maybe at send " << errno << std::endl;
			exit(1);
		}
		else
		{
			std::cout << "Bytes Sent: " << bytes_sent << std::endl;
		}
		
		freeaddrinfo(res);
		
		std::cout << "EXIT PARENT BEFORE" << std::endl;
		
		
		int status2;
		waitpid(c_pid,&status2,0);
		//waitpid(c_pid,&status2,0);
		//fork();
		ParentProcessAfter();
	}
	
	std::cout << "SUCCESSFUL EXIT OF MAIN" << std::endl;
	return 0;
}

void ParentProcessAfter(void) {
	std::cout << "ENTER PARENT AFTER" << std::endl;
	
	std::cout << "this should come at the end of child" << std::endl;
	
	std::cout << "EXIT PARENT AFTER" << std::endl;
}

std::string GetFourLetterRep(int bytes) {
	std::cout << "input for 4 letter rep: " << bytes << std::endl;
	int num_not_zero = 0;
	
	for (int i = bytes; i > 0; i /= 10) {
		num_not_zero++;
	}
	
	std::cout << "num not zero is: " << num_not_zero << std::endl;
	
	std::string ret = "";
	
	ret = std::to_string(bytes);
	
	//int size_ret = ret.size();
	
	for (int i = num_not_zero; i < 4; ++i) {
		ret = "0"+ret;
	}
	std::cout << "return of 4 letter rep: " << ret << std::endl;
	return ret;
}

int FourLetterRepVal(std::string input) {
	std::cout << "input for conversion: " << input << std::endl;
	return stoi(input);
}