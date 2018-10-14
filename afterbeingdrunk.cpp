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
	
	//map shred memory for the semaphores
	//struct semaphores *sems = (struct semaphores *)mmap(0, sizeof(*sems), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	
	//sem_t sem_p, sem_c;
	
	//int ret, ret2;
	//ret = sem_init(&sem_p, 1, 0);
	//ret2 = sem_init(&sem_c, 1, 0);
	//std::cout << "ret2 is: " << ret2 << std::endl;
	
	//sem_init(&sems->parent_done, 1, 0);
	//sem_init(&sems->child_done, 1, 0);
	
	//create a listening socket
	//create the child, it starts accepting
	//the parent should send out the data, also start listening for input on a different port and then wait
	//the child should read in the input, and then pass back the input to the parent
	
	//Let 3490 be the port that the child reads from and the parent sends through
	//Let 3491 be the port that the parent reads from and the child sends through
	
	//DATA THAT IS MUTUALLY USEFUL FOR BOTH PROCESSES
	int status_c,status_p;//
	struct addrinfo hints_c, hints_p;//
	struct addrinfo *res_c, *res_p;//
	
	int child_sock, parent_sock, binding, listening;//, newfd;
	
	int limit = 10;
	
	//TIME TO FORK!
	pid_t c_pid, p_pid;
	
	p_pid = getpid();
	c_pid = fork();
	
	if (c_pid == -1) {
		exit(1);//error forking
	}
	else if (c_pid == 0) {
		//ChildProcess1(child_sock, res_c);
		std::cout << "ENTER CHILD" << std::endl;
	
		int newfd, recieved;
		struct sockaddr_storage incomming_addr;
		socklen_t addr_size;
		
		int limit = 10;
		
		memset(&hints_c, 0, sizeof(struct addrinfo));
		hints_c.ai_family = AF_UNSPEC;
		hints_c.ai_socktype = SOCK_STREAM;
		hints_c.ai_flags = AI_PASSIVE;
		
		if((status_c = getaddrinfo(NULL, "3490", &hints_c, &res_c)) != 0)
		{
			std::cout << "ERROR at getaddrinfo " << gai_strerror(status_c) << std::endl;
			exit(1);
		}
		
		if((status_c = getaddrinfo(NULL, "3490", &hints_c, &res_c)) != 0)
		{
			std::cout << "ERROR at getaddrinfo " << gai_strerror(status_c) << std::endl;
			exit(1);
		}
		
		////////////////////////////////////////////////////////////////////////////////////////
		
		child_sock = socket(res_c->ai_family,res_c->ai_socktype,res_c->ai_protocol);
		if(child_sock == -1)
		{
			std::cout << "ERROR at socket call " << errno << std::endl;
			exit(1);
		}
		
		///////////////////////////////////////////////////////////////////////////
		std::cout << "child sock: " << child_sock << std::endl;
		int binding = bind(child_sock, res_c->ai_addr, res_c->ai_addrlen);
		if(binding == -1) {
			std::cout << "ERROR at bind call (c) " << errno << std::endl;
			//exit(1);
		}
		
		int listening = listen(child_sock, limit);
		if(listening == -1) {
			std::cout << "ERROR at listen call " << errno << std::endl;
			exit(1);
		}
		///////////////////////////////////////////////////////////////////////////
		
		
		//sem_wait(&sems->parent_done);
		
		//std::cout << "post child done start" << std::endl;
		//sem_post(&sems->child_done);
		//std::cout << "post child done done" << std::endl;
		
		addr_size = sizeof(incomming_addr);
		std::cout << "waiting2" << std::endl;
		newfd = accept(child_sock, (struct sockaddr *) &incomming_addr, &addr_size);
		if(newfd == -1) {
			std::cout << "ERROR at accept call " << errno << std::endl;
			exit(1);
		}
		
		char *buf = new char[1024];
		recieved = recv(newfd, buf, sizeof(buf), 0);
		if (recieved == -1) {
			std::cout << "ERROR at recv call " << errno << std::endl;
			//incomming = false;
			exit(1);
		}
		//EXIT CHILD
	}
	else {
		std::cout << "ENTER PARENT BEFORE" << std::endl;
		
		//std::ifstream myFileHandler;
		//myFileHandler.open(filepath);
		
		std::string line;
		std::string lower_line;
		int connected;
		//int i = 0;
		int line_size = 0;
		
		memset(&hints_c, 0, sizeof(struct addrinfo));
		hints_c.ai_family = AF_UNSPEC;
		hints_c.ai_socktype = SOCK_STREAM;
		hints_c.ai_flags = AI_PASSIVE;
		
		if((status_c = getaddrinfo(NULL, "3490", &hints_c, &res_c)) != 0)
		{
			std::cout << "ERROR at getaddrinfo " << gai_strerror(status_c) << std::endl;
			exit(1);
		}
		
		if((status_c = getaddrinfo(NULL, "3490", &hints_c, &res_c)) != 0)
		{
			std::cout << "ERROR at getaddrinfo " << gai_strerror(status_c) << std::endl;
			exit(1);
		}
		
		////////////////////////////////////////////////////////////////////////////////////////
		
		child_sock = socket(res_c->ai_family,res_c->ai_socktype,res_c->ai_protocol);
		if(child_sock == -1)
		{
			std::cout << "ERROR at socket call " << errno << std::endl;
			exit(1);
		}
		
		std::cout << "trying to connect" << std::endl;
		sleep(8);
		connected = connect(child_sock, res_c->ai_addr, res_c->ai_addrlen);
		if(connected == -1) {
			std::cout << "ERROR at connect call " << errno << std::endl;
			exit(1);
		}
		
		std::cout << "connected successfully" << std::endl;
		
		char const *msg = "Hello World please work";
		int len, bytes_sent;
		len = strlen(msg);
		bytes_sent = send(child_sock, msg, len, 0);
		if(bytes_sent != len)
		{
			std::cout << "ERROR maybe at send " << errno << std::endl;
			exit(1);
		}
		else
		{
			std::cout << "Bytes Sent: " << bytes_sent << std::endl;
		}
		/*
		if (myFileHandler.is_open()) {
			
			while (getline(myFileHandler, line)) {
				//lower_line = to_lower(line);
				line_size = sizeof(line);
				line = GetFourLetterRep(line_size) + line;
				
				
			}
			myFileHandler.close();
		}*/
		
		
		//continuously try to send the content of the file line by line once it is 
	
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

/*void ChildProcess1(int & child_sock, addrinfo *res_c) {
	std::cout << "ENTER CHILD" << std::endl;
	
	int newfd, recieved;
	struct sockaddr_storage incomming_addr;
	socklen_t addr_size;
	
	int limit = 10;
	
	///////////////////////////////////////////////////////////////////////////
	int binding = bind(child_sock, res_c->ai_addr, res_c->ai_addrlen);
	if(binding == -1) {
		std::cout << "ERROR at bind call " << errno << std::endl;
		exit(1);
	}
	
	int listening = listen(child_sock, limit);
	if(listening == -1) {
		std::cout << "ERROR at listen call " << errno << std::endl;
		exit(1);
	}
	///////////////////////////////////////////////////////////////////////////
	
	addr_size = sizeof(incomming_addr);
	std::cout << "waiting2" << std::endl;
	newfd = accept(child_sock, (struct sockaddr *) &incomming_addr, &addr_size);
	if(newfd == -1) {
		std::cout << "ERROR at accept call " << errno << std::endl;
		exit(1);
	}
	
	char *buf = new char[1024];
	recieved = recv(newfd, buf, sizeof(buf), 0);
	if (recieved == -1) {
		std::cout << "ERROR at recv call " << errno << std::endl;
		//incomming = false;
		exit(1);
	}
	
	/*char *buf = new char[1024];
	int len;
	bool incomming = true;
	std::string rec = "";
	
	while(incomming) {
		std::cout << "waiting1" << std::endl;
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
	
	std::cout << "EXIT CHILD" << std::endl;
}*/

void ParentProcessAfter(void) {
	std::cout << "ENTER PARENT AFTER" << std::endl;
	
	std::cout << "this should come at the end of child" << std::endl;
	
	std::cout << "EXIT PARENT AFTER" << std::endl;
}

std::string GetFourLetterRep(int bytes) {
	int num_not_zero = 0;
	
	for (int i = bytes; i > 0; i /= 10) {
		num_not_zero++;
	}
	
	std::string ret = "";
	
	ret = std::to_string(num_not_zero);
	
	int size_ret = ret.size();
	
	for (int i = size_ret; i < 4; ++i) {
		ret = "0"+ret;
	}
	
	return ret;
}