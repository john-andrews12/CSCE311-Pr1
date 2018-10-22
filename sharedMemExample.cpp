#include <string.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h> 

//Modified code from Beej's shared memory example
using namespace std;

#define SHM_SIZE 1024  /* make it a 1K shared memory segment */

int main(int argc, char *argv[])
{
    key_t key;
    int shmid;
    char *data;
    string initial = "initial value";
    data = (char*) initial.c_str();
    cout << data << endl;
    string finale = "modified value";
    /* make the key: */
    if ((key = ftok("shT.cpp", 'R')) == -1) {
        perror("ftok");
        exit(1);
    }

    /* connect to (and possibly create) the segment: */
    if ((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

   /* attach to the segment to get a pointer to it: */
    data = (char*) shmat(shmid, (void *)0, 0);
    if (data == (char *)(-1)) {
        perror("shmat");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1)
	cout << "forking error" << endl;
    else if (pid == 0) {
        cout << "child process pid is " << pid << endl;
        printf("writing to segment: \"%s\"\n", finale.c_str());
        strncpy(data, finale.c_str(), SHM_SIZE);
	//data = (char*) finale.c_str();
    } else{
        wait(NULL);
        cout << "parent process pid is " << pid << endl;
	printf("segment contains: \"%s\"\n", data);
	}
    /* detach from the segment: */
    if (shmdt(data) == -1) {
        perror("shmdt");
        exit(1);
    }
    shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
