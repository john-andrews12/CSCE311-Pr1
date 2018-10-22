#include <iostream>
#include <fstream>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>

std::string GetEightLetterRep(std::string input);
bool StringAContainsB(std::string a, std::string b);
std::string RemoveStartEndSymbols(std::string input);
std::string ToLower(std::string input);

#define PARENT_PATH "unix_sock.parent"
#define CHILD_PATH "unix_sock.child"
#define FIRST_MESSAGE_LEN 8
#define ENGLISH_WORD_DELIM ' '
#define ERRNO_NO_SUCH_FILE 2
#define ERRNO_NO_DATA_AVAILABLE 61

int main(int argc, char *argv[]) {
	//std::cout << "ENTERING MAIN" << std::endl;
	
	if (argc != 3) {
		std::cout << "usage: ./Aprog <filepath> <keyword>" << std::endl;
		exit(1);
	}
	
	std::string filepath = argv[1];
	std::string keyword = argv[2];
	keyword = ToLower(keyword);//we search for the keyword in a non-case sensitive context
	
	pid_t c_pid;
	c_pid = fork();
	
	if (c_pid == -1) {
		exit(1);//error forking
	}
	else if (c_pid == 0) {
		//std::cout << "ENTER CHILD" << std::endl;
		
		//here we are creating the socket that the child will read from and the parent sends to
		struct sockaddr_storage incomming_addr;
		socklen_t addr_size;
		
		int child_sock2, parent_sock2, len2, binding, connected, listening, recieved, newfd;
		int limit = 10;
		
		struct sockaddr_un parent_sockaddr; 
		struct sockaddr_un child_sockaddr; 
		
		memset(&child_sockaddr, 0, sizeof(struct sockaddr_un));
		
		child_sock2 = socket(AF_UNIX, SOCK_STREAM, 0);
		if (child_sock2 == -1) {
			std::cout << "SOCKET ERROR" << errno << std::endl;
			exit(1);
		}
		
		child_sockaddr.sun_family = AF_UNIX;   
		strcpy(child_sockaddr.sun_path, CHILD_PATH); 
		len2 = sizeof(child_sockaddr);

		unlink(CHILD_PATH);
		binding = bind(child_sock2, (struct sockaddr *) &child_sockaddr, len2);
		if (binding == -1){
			std::cout << "BIND ERROR"<< errno << std::endl;
			close(child_sock2);
			exit(1);
		}
		
		//now the socket should be created and bound, next thing to do is listen for someone connecting
		listening = listen(child_sock2, limit);
		if(listening == -1) {
			std::cout << "ERROR at listen call " << errno << std::endl;
			exit(1);
		}
		
		addr_size = sizeof(incomming_addr);
		
		newfd = accept(child_sock2, (struct sockaddr *) &incomming_addr, &addr_size);
		if(newfd == -1) {
			std::cout << "ERROR at accept call " << errno << std::endl;
			exit(1);
		}
		
		//HANDLE INPUT FROM PARENT PROCESS
		
		char *buf = new char[1024];
		bool first_com = true;
		
		std::string rec = "";
		std::vector<std::string> all_lines;
		
		//read from buffer as one step
		//then we process the input (what was read) as another step
		
		int total_lines = 0;
		int size_rec = 0;
		//handle the initial message, that is getting the total number of expected lines
		while (first_com) {
			recieved = recv(newfd, buf, sizeof(buf), 0);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			if (recieved >= FIRST_MESSAGE_LEN || rec.size() + recieved >= FIRST_MESSAGE_LEN) {
				//the first thing sent should be the number of lines, 8 digits gives 99,999,999
				//lines of possibility and I'm assuming we aren't getting files that large
				first_com = false;//once we get those eight bytes we have received the first com
				
				size_rec = rec.size();
				//stop condition here is what we need minus what we already have
				for (int i = 0; i < FIRST_MESSAGE_LEN - size_rec; ++i) {
					rec += buf[i];
					recieved--;
				}
				
				total_lines = stoi(rec);
				rec = "";
				
				//if there are still characters to process - note the stop condition
				for (int i = FIRST_MESSAGE_LEN; recieved > 0; ++i) {
					if (buf[i] == '\0') {
						if (StringAContainsB(rec,keyword)) {
							all_lines.push_back(rec);
						}
						total_lines--;
						rec = "";
					}
					else
					{
						rec += buf[i];
					}
					recieved--;
				}
			}
			
			if (first_com && recieved > 0) {
				//we haven't read a total of FIRST_MESSAGE_LEN yet so just add whats there to the
				//accumulator and let the while(first_com) keep going
				for (int i = 0; i < recieved; ++i) {
					rec += buf[i];
				}
			}
		}
		
		//while there are still lines to process
		while (total_lines > 0) {
			//read from the socket
			recieved = recv(newfd, buf, sizeof(buf), 0);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			for (int i = 0; i < recieved; i++) {
				if (buf[i] == '\0') {
					//rec is currently holding a full line so process it and subtract from the total lines
					total_lines--;
					if (StringAContainsB(rec,keyword)) {
						all_lines.push_back(rec);
					}
					
					rec = "";
				}
				else
				{
					//if this isn't the end of a line, just add the next char onto the accumulator
					rec += buf[i];
				}
			}
		}
		
		//at this point, we have read in all lines and checked them for the key word, 
		//we have all lines that contained the key word in all_lines, 
		//now we just need to sort them and send them back
		
		//sorting::
		//implementing selection sort because I do not remember how to write quick sort immediately
		std::string temp;//i say "shortest", but I mean smallest alphabetically
		int shortest;
		if(all_lines.size() > 0)
		{
			for (int i = 0; i < all_lines.size()-1; ++i) {
				shortest = i;//assume it to be the string already there
				for (int j = i+1; j < all_lines.size(); ++j) {
					if (all_lines.at(shortest).compare(all_lines.at(j)) > 0) {
						shortest = j;
					}
				}
				temp = all_lines.at(shortest);
				all_lines.at(shortest) = all_lines.at(i);
				all_lines.at(i) = temp;
			}
		}
		
		//give this data back to the parent process
		
		//first we need to try to connect to the parent
		bool connected_to_parent = false;
		while (!connected_to_parent) {
			//go through the process of making a socket and try to connect to it
			memset(&parent_sockaddr, 0, sizeof(struct sockaddr_un));
			
			parent_sock2 = socket(AF_UNIX, SOCK_STREAM, 0);
			if (parent_sock2 == -1) {
				std::cout << "SOCKET ERROR" << errno << std::endl;
				exit(1);
			}
			
			parent_sockaddr.sun_family = AF_UNIX;   
			strcpy(parent_sockaddr.sun_path, PARENT_PATH); 
			len2 = sizeof(parent_sockaddr);
			
			connected = connect(parent_sock2, (struct sockaddr *) &parent_sockaddr, len2);
			if (connected == -1) {
				if (errno == ERRNO_NO_SUCH_FILE || errno == ERRNO_NO_DATA_AVAILABLE) {
					//the socket simply hasn't been created yet by the parent or it isn't recieving yet
					close(parent_sock2);
				}
				else {
					//there was some other real error
					std::cout << "ERROR at connect call (2) " << errno << std::endl;
					exit(1);
				}
			}
			else {
				connected_to_parent = true;
			}
		}
		
		//first we send how many lines it should be expecting
		char const *first_message = GetEightLetterRep(std::to_string(all_lines.size())).c_str();
		
		int bytes_sent = send(parent_sock2, first_message, strlen(first_message), 0);
		if (bytes_sent != FIRST_MESSAGE_LEN) {
			std::cout << "ERROR maybe at send (4) " << errno << std::endl;
			exit(1);
		}
		
		//now we send each and every line 
		std::string msg = "";
		int leng;
		for (int i = 0; i < all_lines.size(); ++i) {
			msg = all_lines.at(i) + '\0';
			leng = msg.size();
			char const *msgfinal = msg.c_str();
			
			bytes_sent = send(parent_sock2, msgfinal, leng, 0);
			if (bytes_sent == -1) {
				std::cout << "ERROR maybe at send (3) " << errno << std::endl;
				exit(1);
			}
		}
		
		close(parent_sock2);
		close(child_sock2);
		//std::cout << "EXIT CHILD" << std::endl;
	}
	else {
		//std::cout << "ENTER PARENT BEFORE" << std::endl;
		signal(SIGCHLD,SIG_IGN);//we do not care about the exit status of the child
		
		struct sockaddr_storage incomming_addr;
		socklen_t addr_size;
		
		int child_sock2, parent_sock2, len2, binding, connected, listening, newfd;
		int limit = 10;
		
		struct sockaddr_un parent_sockaddr; 
		struct sockaddr_un child_sockaddr; 
		
		//try to establish a connection to the child socket 
		bool connected_to_child = false;
		while (!connected_to_child) {
			//go through the process of making a socket and try to connect to it
			memset(&child_sockaddr, 0, sizeof(struct sockaddr_un));
			
			child_sock2 = socket(AF_UNIX, SOCK_STREAM, 0);
			if (child_sock2 == -1) {
				std::cout << "SOCKET ERROR" << errno << std::endl;
				exit(1);
			}
			
			child_sockaddr.sun_family = AF_UNIX;   
			strcpy(child_sockaddr.sun_path, CHILD_PATH); 
			len2 = sizeof(child_sockaddr);
			
			connected = connect(child_sock2, (struct sockaddr *) &child_sockaddr, len2);
			if (connected == -1) {
				if (errno == ERRNO_NO_SUCH_FILE || errno == ERRNO_NO_DATA_AVAILABLE) {
					//the socket simply hasn't been created yet by the child or it isn't recieving yet
					close(child_sock2);
				}
				else {
					//there was some other real error
					std::cout << "ERROR at connect call (1) " << errno << std::endl;
					exit(1);
				}
			}
			else {
				connected_to_child = true;
			}
		}
		
		//so now we open up the file of text
		
		//first thing to do is get how many lines of text there are
		int lines_total = 0;
		
		std::ifstream myFileHandler;
		myFileHandler.open(filepath);
		
		std::string line;
		std::string msg = "";
		int len, bytes_sent;
		
		if (myFileHandler.is_open()) {
			while (getline(myFileHandler, line)) {
				lines_total++;
			}
			myFileHandler.close();
		}
		
		//now we need to send the first message, that is, how many lines there are total
		char const *first_message = GetEightLetterRep(std::to_string(lines_total)).c_str();
		
		bytes_sent = send(child_sock2, first_message, strlen(first_message), 0);
		if (bytes_sent != FIRST_MESSAGE_LEN) {
			std::cout << "ERROR maybe at send " << errno << std::endl;
			exit(1);
		}
		
		//no we reopen the file to actually send each line of text
		myFileHandler.open(filepath);
		
		if (myFileHandler.is_open()) {
			while (getline(myFileHandler, line)) {
				msg = line + '\0';//mark the end of a line with a '\0' char
				len = msg.length();
				char const *msgfinal = msg.c_str();
				
				bytes_sent = send(child_sock2, msgfinal, len, 0);
				if(bytes_sent != len)
				{
					std::cout << "ERROR maybe at send " << errno << std::endl;
					exit(1);
				}
			}
			myFileHandler.close();
		}
		
		//std::cout << "EXIT PARENT BEFORE" << std::endl;
		
		//open up the parent port to recieve the child data
		memset(&parent_sockaddr, 0, sizeof(struct sockaddr_un));
		
		parent_sock2 = socket(AF_UNIX, SOCK_STREAM, 0);
		if (parent_sock2 == -1) {
			std::cout << "SOCKET ERROR" << errno << std::endl;
			exit(1);
		}
		
		parent_sockaddr.sun_family = AF_UNIX;   
		strcpy(parent_sockaddr.sun_path, PARENT_PATH); 
		len2 = sizeof(parent_sockaddr);

		unlink(PARENT_PATH);
		binding = bind(parent_sock2, (struct sockaddr *) &parent_sockaddr, len2);
		if (binding == -1){
			std::cout << "BIND ERROR"<< errno << std::endl;
			close(parent_sock2);
			exit(1);
		}
		
		listening = listen(parent_sock2, limit);
		if(listening == -1) {
			std::cout << "ERROR at listen call " << errno << std::endl;
			exit(1);
		}
		
		addr_size = sizeof(incomming_addr);
		
		newfd = accept(parent_sock2, (struct sockaddr *) &incomming_addr, &addr_size);
		if(newfd == -1) {
			std::cout << "ERROR at accept call " << errno << std::endl;
			exit(1);
		}
		
		//HANDLE INPUT FROM CHILD PROCESS
		std::cout << "Lines Containing key word alphabeticaly:" << std::endl;
		
		char *buf = new char[1024];
		bool first_com = true;
		
		std::string rec = "";
		
		int recieved;//the number of bytes we read in on every cycle
		
		std::vector<std::string> all_lines;
		
		//read from buffer as one step
		//then we process the input (what was read) as another step
		
		int total_lines = 0;
		int size_rec = 0;
		//handle the initial message, that is getting the total number of expected lines
		while (first_com) {
			recieved = recv(newfd, buf, sizeof(buf), 0);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			if (recieved >= FIRST_MESSAGE_LEN || rec.size() + recieved >= FIRST_MESSAGE_LEN) {
				//note: we have to process 'recieved' characters. We only need the first 8 for the length, all past that 
				//still need to be handled, just separtely 
				
				//the first thing sent should be the number of lines, 8 digits gives 99,999,999
				//lines of possibility and I'm assuming we aren't getting files that large
				first_com = false;//once we get those eight bytes we have received the first com
				
				size_rec = rec.size();
				for (int i = 0; i < FIRST_MESSAGE_LEN - size_rec; ++i) {
					rec += buf[i];
					recieved--;
				}
				
				total_lines = stoi(rec);
				rec = "";
				
				//if there is still characters to process, note the stop condition
				for (int i = FIRST_MESSAGE_LEN; recieved > 0; ++i) {
					if (buf[i] == '\0') {
						std::cout << rec << std::endl;
						total_lines--;
						rec = "";
					}
					else
					{
						rec += buf[i];
					}
					recieved--;
				}
			}
			
			if (first_com && recieved > 0) {
				//we haven't read a total of FIRST_MESSAGE_LEN yet so just add whats there to the
				//accumulator and let the while(first_com) keep going
				for (int i = 0; i < recieved; ++i) {
					rec += buf[i];
				}
			}
		}
		
		//while there are still lines to process
		while (total_lines > 0) {
			recieved = recv(newfd, buf, sizeof(buf), 0);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			for (int i = 0; i < recieved; ++i) {
				if (buf[i] == '\0') {
					//this is how output is given to the user
					std::cout << rec << std::endl;
					total_lines--;
					rec = "";
				}
				else
				{
					rec += buf[i];
				}
			}
		}
		
		close(parent_sock2);
		close(child_sock2);
	}
	
	//std::cout << "SUCCESSFUL EXIT OF MAIN" << std::endl;
	return 0;
}

std::string RemoveStartEndSymbols(std::string input) {
	std::string to_add = "";
	
	for (int i = 0; i < input.size(); ++i) {
		//loop until we find the first instance of a valid character
		if (isalnum(input.at(i))) {
			to_add = input.substr(i);
			break;
		}
	}
	
	//note we are going backwards through the string now
	for (int i = to_add.size()-1; i >= 0; --i) {
		//loop until we find a valid character
		if (isalnum(to_add.at(i))) {
			to_add = to_add.substr(0,i+1);
			break;
		}
	}
	
	return to_add;
}

bool StringAContainsB(std::string a, std::string b) {
	bool ret_val = false;
	char curr;
	
	std::vector<std::string> split_string;
	
	int len = a.size();
	std::string curr_word = "";
	
	for (int i = 0; i < len; ++i) {
		curr = a.at(i);
		
		if (curr == ENGLISH_WORD_DELIM) {
			if (curr_word.size() > 0) {
				//if the current word has size
				
				//remove non alpha-numeric entities from the begginning and the ending of the string
				curr_word = RemoveStartEndSymbols(curr_word);
				
				split_string.push_back(curr_word);
				curr_word = "";//reset for the next word
			}
			//otherwise we know the word was empty so the string started with the
			//delimiter or there were multiple delimiters in a row
		}
		else {
			curr_word += tolower(curr);//again, non case sensitive
		}
	}
	
	//if the line does not end on the delimiter, the last word still needs to be added
	if (curr_word.size() > 0) {
		curr_word = RemoveStartEndSymbols(curr_word);
		split_string.push_back(curr_word);
	}
	
	//check if the word contains the search string
	for (int i = 0; i < split_string.size(); ++i) {
		if (split_string.at(i) == b) {
			ret_val = true;
			break;//no need to keep looking
		}
	}
	
	return ret_val;
}

std::string GetEightLetterRep(std::string input) {
	int num_not_zero = input.size();
	
	if (num_not_zero > FIRST_MESSAGE_LEN) {
		//the file has too many lines
		exit(1);
	}
	
	std::string ret = input;
	
	for (int i = num_not_zero; i < FIRST_MESSAGE_LEN; ++i) {
		ret = "0"+ret;
	}
	
	return ret;
}

std::string ToLower(std::string input) {
	std::string ret = "";
	
	for (int i = 0; i < input.size(); ++i)
	{
		ret += tolower(input.at(i));
	}
	
	return ret;
}







