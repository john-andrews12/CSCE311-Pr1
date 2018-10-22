#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <vector>
#include <pthread.h>
#include <map>
#include <set>
#include <unistd.h>
#include <sys/wait.h> 

using namespace std;
typedef map<string, set<string>> MapData;

#define NUM_THREADS 4

//Struct used as a parameter for the threads.
struct line_data
{
	set<string> lines;
    MapData my_map;
};

//Re-using John's function
string RemoveStartEndSymbols(string input) 
{
    string input_lowercase = "";
	string to_add = "";
        for (int i = 0; i < input.size(); ++i)
        {
            input_lowercase += tolower(input.at(i), locale());
        }
	
	for (int i = 0; i < input_lowercase.size(); ++i) 
    {
		//loop until we find the first instance of a valid character
		if (isalnum(input_lowercase.at(i))) 
        {
			to_add = input_lowercase.substr(i);
			break;
		}
	}
	
	//note we are going backwards through the string now
	for (int i = to_add.size()-1; i >= 0; --i) 
    {
		//loop until we find a valid character
		if (isalnum(to_add.at(i))) 
        {
			to_add = to_add.substr(0,i+1);
			break;
		}
	}
	
	return to_add;
}

//This function, called by each of the four threads,
//maps all of the words in each section of the input text such that
//the keys represent every word in the file and the values represent all
//of the lines where the key appears.
void *Mapper(void *threadarg)
{
	
    struct line_data *my_data;
    my_data = (struct line_data *) threadarg;
    MapData ret_val;
    string word;
    string current_line;
    set<string> s_temp;

	//Iterates line by line, then word by word to generate the map.
    set<string>::iterator it;
    for (it = my_data->lines.begin(); it != my_data->lines.end(); ++it)
    {
        current_line = *it;
        istringstream iss(current_line);
        while (iss >> word)
        {
            //We only care about tokens that are actually words, so they
            //must be a sequence of alphabetical characters, potentially with
            //an apostrophe or a hyphen. Also, convert all letters to lowercase
            //so there's no distinction between strings like "Your" and "your".
            /*
            if (isalpha(word.at(0)))
            {
            
                string temp = "";
                temp += tolower(word.at(0), locale());
                for (unsigned int i=1; i < word.length(); i++)
                {
                    if (isalpha(word.at(i)))
                    {
                        temp += tolower(word.at(i), locale());
                    }
                    //Note that if the final character is not alphabetical,
                    //we ignore it so that strings like "your" and "your," will
                    //both be added to the map as just "your"
                    else if ((word.at(i) == '\'' || word.at(i) == '-')
                              && i != (word.length() - 1))
                    {
                        temp += word.at(i);
                    }
                }
            */
            string temp = RemoveStartEndSymbols(word);
                //Once we have a token representing a word, check if it's
                //already in the map. If it is, add the current line to the
                //set associated with that word.
                if (ret_val.count(temp) == 0)
                {
                    s_temp.insert(current_line);
                    ret_val.insert(pair<string, set<string>>(temp, s_temp));
                    s_temp.clear();
                }
                else
                {
                    MapData::iterator mit = ret_val.find(temp);
                    mit->second.insert(current_line);
                }
            //}
        }
    }

	//Assign the newly generated map to the map associated with the thread.
    my_data->my_map = ret_val;
    pthread_exit(0);
	
}

//This function splits the input from the text file into equal sections,
//one for each thread. The idea is that each thread will map its own section
//of the file, and then the main thread will reduce those maps into one map.
//The reason I have this function returning a set is because the strings will
//automatically be sorted alphabetically.
set<string> Splitter(int numOfLines, vector<string> v, int section)
{
	set<string> ret_val;
	
	//The value of i and j depends on which threa is currently making its way 
	//through this function. Thread number zero results in i=0 and so on.
	int i = (section* (numOfLines / NUM_THREADS));
	int j = ((section+1) * (numOfLines / NUM_THREADS));
	
	//If the number of lines in the text file isn't divisible by four, add
	//the remaining lines to the j value for the final thread.
	if (section == (NUM_THREADS - 1) && (numOfLines % NUM_THREADS != 0))
	{
		j += (numOfLines % NUM_THREADS);
	}
	
	for (i; i < j; i++)
	{
		ret_val.insert(v.at(i));
	}
	
	return ret_val;
}

long GetFileSize(string fileName)
{
    FILE *p_file = NULL;
    p_file = fopen(fileName.c_str(),"rb");
    fseek(p_file,0,SEEK_END);
    long size = ftell(p_file);
    fclose(p_file);
    return size;
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "usage: ./Aprog <filepath> <keyword>" << endl;
		exit(1);
	}
	
    string fileName = argv[1];
    string keyword = argv[2];
    ifstream myFile(fileName.c_str());
    const long SHM_SIZE = GetFileSize(fileName);	
    //Pointer to shared memory.
    char *segptr;
	
    string line;
    string inpL = "";
    int numOfLines = 0;
	
    //Open the input file and put all the text into one string.
    if (myFile.is_open())
    {
        while (getline(myFile,line))
        {
            numOfLines++;
            inpL += line;
            inpL += "\n";
        }
    }
    
    else
    { 
        cout << "Unable to open file." << endl;
        exit(1);
	}
	
	myFile.close();
	
	//ftok system call generates a unique key for the shared memory.
	key_t shmkey = ftok(fileName.c_str(),15);
	if (shmkey == -1)
	{
		perror("ftok");
		exit(1);
	}
	
	//shmget creates a portion of shared memory and returns an identifier.
	int shmid = shmget(shmkey, SHM_SIZE, IPC_CREAT | 0666);
	if (shmid == -1)
	{
		perror("shmget");
		exit(1);
	}
	
	//shmat attaches this shared memory to a pointer.
	segptr = (char*) shmat(shmid,(void*)0,0);

	if(segptr == (char*) -1)
	{
		perror("shmat");
		exit(1);
	}
	
	//Puts the text from the input file in shared memory.
	//segptr = (char*) inpL.c_str();
	strncpy(segptr,inpL.c_str(),SHM_SIZE);
    
	//Creates the child process that will perform multithreading and MapReduce.
	pid_t pid = fork();
	if (pid == -1)
	{
	    cout << "forking error" << endl;
	}
	else if (pid == 0)
	{
		
        vector<string> v;
        string str(segptr);
        istringstream inpStream(str);
        while(!inpStream.eof())
        {
                getline(inpStream, line);
                v.push_back(line);
        }
		
        //Creates an array of threads and corresponding structs
        //to be passed to the Mapper function.
        pthread_t threads[NUM_THREADS];
        pthread_attr_t attr;
        struct line_data ld[NUM_THREADS];
    
        int rc;
        unsigned int k;
	    
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	    
        //Creates each thread and haves them all run the Mapper function
        //on their respective sections of text generated by Splitter.
        for (k=0; k < NUM_THREADS; k++)
        {
            ld[k].lines = Splitter(numOfLines, v, k);
            rc = pthread_create(&threads[k], &attr, Mapper, (void *)&ld[k]);

            if (rc)
            {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
	    
        vector<MapData> v_maps;
		
        //Frees the attribute and waits for the threads to finish.
        pthread_attr_destroy(&attr);
        for(k=0; k < NUM_THREADS; k++) 
        {
            rc = pthread_join(threads[k], NULL);
            v_maps.push_back(ld[k].my_map);
            if (rc) 
            {
                printf("ERROR; return code from pthread_join() is %d\n", rc);
                exit(-1);
            }
        }
        
        //Performs the Reduce step. Starts by making a copy of the first map,
        //then inserts every new key from every other map and merges any keys
        //that are shared between maps.
        MapData reduced_map = v_maps.at(0);
        for (k=1; k < v_maps.size(); k++)
        {
            MapData temp = v_maps.at(k);
            MapData::iterator mit;
            for (mit = temp.begin(); mit != temp.end(); ++mit)
            {
                string key = mit->first;
                set<string> value = mit->second;
				
                MapData::iterator rit = reduced_map.find(key);
                if (rit != reduced_map.end())
                {
                    set<string> reduced_value = rit->second;
                    reduced_value.insert(value.begin(), value.end());
                    rit->second = reduced_value;
                }
				
                reduced_map.insert(pair<string, set<string>>(key, value));
            }
        }
        
        //Since we're not concerned with case-sensitivity, convert
        //the keyword to lowercase just like the map keys.
        string keyword_lowercase = "";
        for (k=0; k < keyword.length(); k++)
        {
            keyword_lowercase += tolower(keyword.at(k), locale());
        }
        
        //Search for the keyword in the reduced map, and if it exists,
        //add all of the lines in its corresponding set to a string.
        MapData::iterator test = reduced_map.find(keyword_lowercase);
        string s = "";
        if (test != reduced_map.end())
        {
            set<string> s_temp = test->second;
            set<string>::iterator sit;
            for (sit = s_temp.begin(); sit != s_temp.end(); ++sit)
            {
                s += *sit;
                s += "\n";
            }
        }
        else
        {
            string not_found = "keyword not found";
            strncpy(segptr, not_found.c_str(), SHM_SIZE);
            exit(1);
        }
	    
        //Change the string in shared memory from the input file's text
        //to the final specified lines so that the parent process can
        //print them.
        strncpy(segptr,s.c_str(),SHM_SIZE);
		
        //shmdt detaches this process from shared memory.
        if (shmdt(segptr) == -1)
        {
            perror("shmdt");
            exit(1);
        }
        exit(EXIT_SUCCESS);
    }

    else
    {
        //Wait for the child process to exit successfully.
        wait(NULL);
        
        //This correctly outputs the lines where the keyword appears.
        printf("Data written in memory:\n %s",segptr);
        if (shmdt(segptr) == -1)
        {
            perror("shmdt");
            exit(1);
        }
	    
        //shmctl destroys the shared memory. 
        shmctl(shmid,IPC_RMID,NULL);
    }
    
    return 0;
}



