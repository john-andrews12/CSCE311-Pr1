#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <signal.h>

bool StringAContainsB(std::string a, std::string b);
std::string RemoveStartEndSymbols(std::string input);
std::string ToLower(std::string input);

#define END_OF_LINE_CHAR '\0'
#define ENGLISH_WORD_DELIM ' '

int main(int argc, char *argv[]) {
	//std::cout << "ENTERING MAIN" << std::endl;
	
	if (argc != 3) {
		std::cout << "usage: ./Aprog <filepath> <keyword>" << std::endl;
		exit(1);
	}
	
	std::string filepath = argv[1];
	std::string keyword = argv[2];
	keyword = ToLower(keyword);//we search for the keyword in a non-case sensitive context
	
	//created pipes in shared address space
	int pipe_P2C[2];
	int pipe_C2P[2];
	int recieved, bytes_sent, len;
	
	if (pipe(pipe_P2C) == -1) {
		std::cout << "pipe error" << std::endl;
		exit(1);
	}
	
	if (pipe(pipe_C2P) == -1) {
		std::cout << "pipe error" << std::endl;
		exit(1);
	}
	
	pid_t c_pid;
	c_pid = fork();
	
	if (c_pid == -1) {
		exit(1);//error forking
	}
	else if (c_pid == 0) {
		//std::cout << "ENTER CHILD" << std::endl;
		close(pipe_P2C[1]);//the child doesn't write from the parent to itself
		close(pipe_C2P[0]);//the child doesn't read from itself to the parent
		
		//HANDLE INPUT FROM PARENT PROCESS
		
		char buf[1024];
		bool first_com = true;
		
		std::string rec = "";
		std::vector<std::string> all_lines;
		
		//read from buffer as one step
		//then we process the input (what was read) as another step
		
		int total_lines = 0;
		
		//handle the initial message, that is getting the total number of expected lines
		while (first_com) {
			recieved = read(pipe_P2C[0], buf, 10);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			for (int i = 0; i < recieved; i++) {
				if (buf[i] == END_OF_LINE_CHAR && first_com) {
					//we've just hit the end of the first communication which tells us how many lines to expect
					total_lines = stoi(rec);
					first_com = false;
					rec = "";
				}
				else if (buf[i] == END_OF_LINE_CHAR) {
					//no longer the first communication, so just process this like a normal line
					total_lines--;
					if (StringAContainsB(rec,keyword)) {
						all_lines.push_back(rec);
					}
					
					rec = "";
				}
				else {
					rec += buf[i];
				}
			}
		}
		
		//while there are still lines to process
		while (total_lines > 0) {
			//having the 1000 here helps read quickly from the buffer
			//ie if the parent process is already sending, this allows the child to read quickly
			recieved = read(pipe_P2C[0], buf, 1000);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			for (int i = 0; i < recieved; i++) {
				if (buf[i] == END_OF_LINE_CHAR) {
					//rec is currently holding a full line so process it and subtract from the total lines
					total_lines--;
					if (StringAContainsB(rec,keyword)) {
						all_lines.push_back(rec);
					}
					
					rec = "";
				}
				else {
					rec += buf[i];
				}
			}
		}
		
		close(pipe_P2C[0]);//we're done reading from parent to child
		
		//at this point, we have read in all lines and checked them for the key word, 
		//we have all lines that contained the key word in all_lines, 
		//now we just need to sort them and send them back
		
		//sorting::
		//implementing selection sort because I do not remember how to write quick sort off the top of my head
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
		//first we send how many lines it should be expecting
		char const *first_message = (std::to_string(all_lines.size()) + END_OF_LINE_CHAR).c_str();
		
		bytes_sent = write(pipe_C2P[1], first_message, strlen(first_message)+1);//plus one to send the end line char
		if (bytes_sent < 0) {
			std::cout << "ERROR maybe at send (1) " << errno << std::endl;
			exit(1);
		}
		
		//now we send each and every line 
		std::string msg = "";
		for (int i = 0; i < all_lines.size(); ++i) {
			msg = all_lines.at(i) + END_OF_LINE_CHAR;
			len = msg.size();
			char const *msgfinal = msg.c_str();
			
			bytes_sent = write(pipe_C2P[1], msgfinal, len);
			if (bytes_sent == -1) {
				std::cout << "ERROR maybe at send (3) " << errno << std::endl;
				exit(1);
			}
		}
		
		close(pipe_C2P[1]);
		//std::cout << "EXIT CHILD" << std::endl;
	}
	else {
		//std::cout << "ENTER PARENT BEFORE" << std::endl;
		signal(SIGCHLD,SIG_IGN);//we do not care about the exit status of the child
		close(pipe_P2C[0]);//the parent doesn't read from the itself to the child
		close(pipe_C2P[1]);//the parent doesn't write the child to itself
		
		//first thing to do is get how many lines of text there are
		int lines_total = 0;
		
		std::ifstream myFileHandler;
		myFileHandler.open(filepath);
		
		std::string line;
		std::string msg = "";
		
		if (myFileHandler.is_open()) {
			while (getline(myFileHandler, line)) {
				lines_total++;
			}
			myFileHandler.close();
		}
		
		//now we need to send the first message, that is, how many lines there are total
		char const *first_message = (std::to_string(lines_total) + END_OF_LINE_CHAR).c_str();
		
		bytes_sent = write(pipe_P2C[1], first_message, strlen(first_message)+1);//plus one to send the end line char
		if (bytes_sent < 0) {
			std::cout << "ERROR maybe at send (1) " << errno << std::endl;
			exit(1);
		}
		
		//no we reopen the file to actually send each line of text
		myFileHandler.open(filepath);
		
		if (myFileHandler.is_open()) {
			while (getline(myFileHandler, line)) {
				msg = line + END_OF_LINE_CHAR;
				len = msg.size();
				char const *msgfinal = msg.c_str();
				
				bytes_sent = write(pipe_P2C[1], msgfinal, len);
				if(bytes_sent != len)
				{
					std::cout << "ERROR maybe at send (2) " << errno << " : " << bytes_sent << std::endl;
					exit(1);
				}
			}
			myFileHandler.close();
		}
		
		close(pipe_P2C[1]);//we no longer need to send from parent to child
		//std::cout << "EXIT PARENT BEFORE" << std::endl;
		
		//HANDLE INPUT FROM CHILD PROCESS
		std::cout << "Lines containing keyword alphabetically:" << std::endl;
		
		char buf[1024];
		bool first_com = true;
		
		std::string rec = "";
		std::vector<std::string> all_lines;
		
		int total_lines = 0;
		
		//handle the initial message, that is getting the total number of expected lines
		while (first_com) {
			recieved = read(pipe_C2P[0], buf, 10);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			for (int i = 0; i < recieved; ++i) {
				if (buf[i] == END_OF_LINE_CHAR && first_com) {
					//we have gotten the entire first communication which is the number of lines to expect
					total_lines = stoi(rec);
					first_com = false;
					rec = "";
				}
				else if (buf[i] == END_OF_LINE_CHAR) {
					//this is no longer the first communication so process the line normally
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
		
		//while there are still lines to process
		while (total_lines > 0) {
			recieved = read(pipe_C2P[0], buf, 1000);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			for (int i = 0; i < recieved; ++i) {
				if (buf[i] == END_OF_LINE_CHAR) {
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
		
		close(pipe_C2P[0]);//no longer need to read from child to parent
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
			//delimiter or there were multiple delimiters in a row, either way,
			//we don't need to do anything
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

std::string ToLower(std::string input) {
	std::string ret = "";
	
	for (int i = 0; i < input.size(); ++i)
	{
		ret += tolower(input.at(i));
	}
	
	return ret;
}
