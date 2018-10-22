#include <iostream>
#include <fstream>
#include <vector>
#include <unistd.h>

std::string GetEightLetterRep(std::string input);
bool StringAContainsB(std::string a, std::string b);

#define FIRST_MESSAGE_LEN 8
#define ENGLISH_WORD_DELIM ' '

int main(int argc, char *argv[]) {
	//std::cout << "ENTERING MAIN" << std::endl;
	
	if (argc != 3) {
		std::cout << "usage: ./Aprog <filepath> <keyword>" << std::endl;
		exit(1);
	}
	
	std::string filepath = argv[1];
	std::string keyword = argv[2];
	
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
		int size_rec = 0;
		
		//handle the initial message, that is getting the total number of expected lines
		while (first_com) {
			recieved = read(pipe_P2C[0], buf, 8);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			//if we will get our first message
			if (recieved >= FIRST_MESSAGE_LEN || rec.size() + recieved >= FIRST_MESSAGE_LEN) {
				//the first thing sent should be the number of lines, 8 digits gives 99,999,999
				//lines of possibility and I'm assuming we aren't getting files that large
				first_com = false;//once we get those eight bytes we have received the first com
				
				size_rec = rec.size();//the size going in
				//stop condition is what is needed minus what we already have
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
				//accumulator and let the while(first_com) loop keep going
				for (int i = 0; i < recieved; ++i) {
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
		char const *first_message = GetEightLetterRep(std::to_string(all_lines.size())).c_str();
		
		bytes_sent = write(pipe_C2P[1], first_message, strlen(first_message));
		if (bytes_sent != FIRST_MESSAGE_LEN) {
			std::cout << "ERROR maybe at send (1) " << errno << std::endl;
			exit(1);
		}
		
		//now we send each and every line 
		std::string msg = "";
		for (int i = 0; i < all_lines.size(); ++i) {
			msg = all_lines.at(i) + '\0';
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
		char const *first_message = GetEightLetterRep(std::to_string(lines_total)).c_str();
		
		bytes_sent = write(pipe_P2C[1], first_message, strlen(first_message));
		if (bytes_sent != FIRST_MESSAGE_LEN) {
			std::cout << "ERROR maybe at send (1) " << errno << std::endl;
			exit(1);
		}
		
		//no we reopen the file to actually send each line of text
		myFileHandler.open(filepath);
		
		if (myFileHandler.is_open()) {
			while (getline(myFileHandler, line)) {
				msg = line + '\0';//mark the end of a line with a '\0' char
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
		std::cout << "Lines Containing key word alphabeticaly:" << std::endl;
		
		char buf[1024];
		bool first_com = true;
		
		std::string rec = "";
		std::vector<std::string> all_lines;
		
		int total_lines = 0;
		int size_rec = 0;
		
		//handle the initial message, that is getting the total number of expected lines
		while (first_com) {
			recieved = read(pipe_C2P[0], buf, 8);
			if (recieved == -1) {
				std::cout << "ERROR at recv call " << errno << std::endl;
				exit(1);
			}
			
			if (recieved >= FIRST_MESSAGE_LEN || rec.size() + recieved >= FIRST_MESSAGE_LEN) {
				//the first thing sent should be the number of lines, 8 digits gives 99,999,999
				//lines of possibility and I'm assuming we aren't getting files that large
				first_com = false;//once we get those eight bytes we have received the first com
				
				size_rec = rec.size();//the size going in
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
			recieved = read(pipe_C2P[0], buf, 1000);
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
		
		close(pipe_C2P[0]);//no longer need to read from child to parent
	}
	
	//std::cout << "SUCCESSFUL EXIT OF MAIN" << std::endl;
	return 0;
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
				split_string.push_back(curr_word);
				curr_word = "";//reset for the next word
			}
			//otherwise we know the word was empty so the string started with the
			//delimiter or there were multiple delimiters in a row, either way,
			//we don't need to do anything
		}
		else {
			curr_word += curr;
		}
	}
	
	//if the line does not end on the delimiter, the last word still needs to be added
	if (curr_word.size() > 0) {
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