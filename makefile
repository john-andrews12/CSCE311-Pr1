GPP = g++ -O3 -Wall -std=c++11

C = completed_sol.cc
S = sharedMemory.cpp
P = pipes_solution.cc

PipesSol: $P
	$(GPP) -o PipesSol $P

pipes_solution.o: pipes_solution.cc
	$(GPP) -c pipes_solution.cc

UnixDomSock: $C
	$(GPP) -o UnixDomSock $C

completed_sol.o: completed_sol.cc
	$(GPP) -c completed_sol.cc

shm: $S
	$(GPP) -pthread -o shm $S

clean:
	rm ./UnixDomSock ./unix_sock.* ./shm ./PipesSol

