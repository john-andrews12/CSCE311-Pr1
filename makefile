GPP = g++ -O3 -Wall -std=c++11

C = completed_sol.cc
S = sharedMemory.cpp

UnixDomSock: $C
	$(GPP) -o UnixDomSock $C

completed_sol.o: completed_sol.cc
	$(GPP) -c completed_sol.cc

shm: $S
	$(GPP) -pthread -o shm $S

clean:
	rm ./UnixDomSock ./unix_sock.* ./shm