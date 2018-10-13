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

//using namespace std;
void ChildProcess(int child_sock, addrinfo *res_c);
void ParentProcessBefore(std::string a_file_path, int child_sock, addrinfo *res_c);
void ParentProcessAfter(void);
std::string GetFourLetterRep(int bytes);

int main(int argc, char *argv[]) {
	std::cout << "ENTERING MAIN" << std::endl;
	
	if (argc != 2) {
		std::cout << "usage: ./Aprog <filepath>" << std::endl;
		exit(1);
	}
	
	std::string filepath = argv[1];
	
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
	
	std::condition_variable p_sending, p_recieving, c_sending, c_recieving;
	bool ready_rec = false;
	std::mutex lock;
	
	//int binding, connected, listening, accepting;
	//int limit = 10;
	
	////////////////////////////////////////////////////////////////////////////////////////
	
	memset(&hints_c, 0, sizeof(struct addrinfo));
	hints_c.ai_family = AF_UNSPEC;
	hints_c.ai_socktype = SOCK_STREAM;
	hints_c.ai_flags = AI_PASSIVE;
	
	if((status_c = getaddrinfo(NULL, "3490", &hints_c, &res_c)) != 0)
	{
		std::cout << "ERROR at getaddrinfo " << gai_strerror(status_c) << std::endl;
		exit(1);
	}
	
	memset(&hints_p, 0, sizeof(struct addrinfo));
	hints_p.ai_family = AF_UNSPEC;
	hints_p.ai_socktype = SOCK_STREAM;
	hints_p.ai_flags = AI_PASSIVE;
	
	if((status_p = getaddrinfo(NULL, "3491", &hints_p, &res_p)) != 0)
	{
		std::cout << "ERROR at getaddrinfo " << gai_strerror(status_p) << std::endl;
		exit(1);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	
	if((status_c = getaddrinfo(NULL, "3490", &hints_c, &res_c)) != 0)
	{
		std::cout << "ERROR at getaddrinfo " << gai_strerror(status_c) << std::endl;
		exit(1);
	}
	
	if((status_p = getaddrinfo(NULL, "3491", &hints_p, &res_p)) != 0)
	{
		std::cout << "ERROR at getaddrinfo " << gai_strerror(status_p) << std::endl;
		exit(1);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	
	child_sock = socket(res_c->ai_family,res_c->ai_socktype,res_c->ai_protocol);
	if(child_sock == -1)
	{
		std::cout << "ERROR at socket call " << errno << std::endl;
		exit(1);
	}
	
	parent_sock = socket(res_p->ai_family,res_p->ai_socktype,res_p->ai_protocol);
	if(parent_sock == -1)
	{
		std::cout << "ERROR at socket call " << errno << std::endl;
		exit(1);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	
	binding = bind(child_sock, res_c->ai_addr, res_c->ai_addrlen);
	if(binding == -1) {
		std::cout << "ERROR at bind call (c) " << errno << std::endl;
		exit(1);
	}
	
	binding = bind(parent_sock, res_p->ai_addr, res_p->ai_addrlen);
	if(binding == -1) {
		std::cout << "ERROR at bind call (p) " << errno << std::endl;
		exit(1);
	}
	
	////////////////////////////////////////////////////////////////////////////////////////
	
	listening = listen(child_sock, limit);
	if(listening == -1) {
		std::cout << "ERROR at listen call " << errno << std::endl;
		exit(1);
	}
	
	listening = listen(parent_sock, limit);
	if(listening == -1) {
		std::cout << "ERROR at listen call " << errno << std::endl;
		exit(1);
	}
	
	//TIME TO FORK!
	pid_t c_pid, p_pid;
	
	p_pid = getpid();
	c_pid = fork();
	
	//std::cout << "c_pid is " << c_pid << std::endl;
	
	if (c_pid == -1) {
		exit(1);//error forking
	}
	else if (c_pid == 0) {
		ChildProcess(child_sock, res_c);
	}
	else {
		int status2;
		waitpid(c_pid,&status2,0);
		ParentProcessBefore(filepath, child_sock, res_c);
		
		//waitpid(c_pid,&status2,0);
		//fork();
		ParentProcessAfter();
	}
	
	std::cout << "SUCCESSFUL EXIT OF MAIN" << std::endl;
	return 0;
}

void ChildProcess(int child_sock, addrinfo *res_c) {
	std::cout << "ENTER CHILD" << std::endl;
	
	/*for (int i = 0; i < 1500; ++i) {
		std::cout << "child at " << i << std::endl;
	}*/
	int newfd, recieved;
	struct sockaddr_storage incomming_addr;
	socklen_t addr_size;
	
	/*int binding = bind(child_sock, res_c->ai_addr, res_c->ai_addrlen);
	if(binding == -1) {
		std::cout << "ERROR at bind call " << errno << std::endl;
		exit(1);
	}
	
	listening = listen(child_sock, limit);
	if(listening == -1) {
		std::cout << "ERROR at listen call " << errno << std::endl;
		exit(1);
	}*/
	
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
	}*/
	
	std::cout << "EXIT CHILD" << std::endl;
}

void ParentProcessBefore(std::string filepath, int child_sock, addrinfo *res_c) {
	std::cout << "ENTER PARENT BEFORE" << std::endl;
	
	/*std::cout << "this should come sometime in the middle of child" << std::endl;
	for (int i = 0; i < 1500; ++i) {
		std::cout << "parent at " << i << std::endl;
	}*/
	//std::ifstream myFileHandler;
	//myFileHandler.open(filepath);
	
	//myFileHandler << "this shouldn't be added\n";
	std::string line;
	std::string lower_line;
	int connected;
	//int i = 0;
	int line_size = 0;
	//try to connect to the child_sock
	//while(connect(child_sock, res_c->ai_addr, res_c->ai_addrlen) == -1)
	//{
		//we're just going to sit and wait for the child process to be accepting signals
		//std::cout << "a" << std::endl;
	//}
	std::unique_lock<std::mutex> lk(lock);
	//std::cout << "waiting3" << std::endl;
	std::cout << "trying to connect" << std::endl;
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
}

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