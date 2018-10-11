#include "main.h"
//file stream: fstream

vector<string> split_string(string line, char del) {
	vector<string> ret;
	string curr_word = "";
	int len = line.size();
	
	for (int i = 0; i < len; ++i) {
		char curr = line.at(i);
		
		if (curr == del && curr_word.size() == 0) {
			//the string has started with the delimiter or there are multiple delimiters in a row
			continue;
		}
		
		if (curr == del) {
			//we know the current word has value so push it onto the return vector
			ret.push_back(curr_word);
			curr_word = "";//reset for the next word
		}
		else {
			curr_word += curr;
		}
	}
	
	//if the line does not end on the delimiter, the last word still needs to be added
	if (curr_word.size() > 0) {
		ret.push_back(curr_word);
	}
	
	return ret;
}

string to_lower(string input) {
	string ret = "";
	
	for (auto iter = input.begin(); iter!= input.end(); ++iter)
	{
		//iterate over the characters in the string and make them lower case
		ret += tolower((*iter));
	}
	
	return ret;
}

bool contains_string(string container, string containee) {
	vector<string> container_split = split_string(container, ' ');
	
	containee = to_lower(containee);
	//for (int i = 0; i < container_split.size(); ++i) {
	//	cout << container_split.at(i) << endl;
	//}
	
	for (int i = 0; i < container_split.size(); ++i) {
		if (container_split.at(i) == containee) {
			return true;
		}
	}
	
	return false;
}

int main() {
	cout << "Starting Execution" << endl; 
	
	//ifstream for reading in (In) from a file 
	//ofstream for writing out (Out) to a file
	ifstream myFileHandler;
	myFileHandler.open("test.txt");
	
	//myFileHandler << "this shouldn't be added\n";
	string line;
	string lower_line;
	int i = 0;
	
	if (myFileHandler.is_open()) {
		while (getline(myFileHandler, line)) {
			lower_line = to_lower(line);
			
			cout << i << ": ";
			if (contains_string(lower_line, "John")) {
				cout << line;
			}
			cout << endl;
			
			i++;
		}
		myFileHandler.close();
	}
	
	//myFileHandler.close();
	
	/*string testing = "   This    is    a test string.   ";
	vector<string> mysplit = split_string(testing,' ');
	
	for (int i = 0; i < mysplit.size(); ++i) {
		cout << i << ": " << mysplit.at(i) << endl;
	}*/
	
	cout << "Finishing" << endl;
	
	return 0;
}
