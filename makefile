GPP = g++ -O3 -Wall -std=c++11

C = unix_dom_sock_sol.cc
S = sharedMemory.cpp
P = pipes_solution.cc

PipesSol: $P
	$(GPP) -o PipesSol $P

pipes_solution.o: pipes_solution.cc
	$(GPP) -c pipes_solution.cc

UnixDomSockSol: $C
	$(GPP) -o UnixDomSockSol $C

unix_dom_sock_sol.o: unix_dom_sock_sol.cc
	$(GPP) -c unix_dom_sock_sol.cc

SharedMemorySol: $S
	$(GPP) -o SharedMemorySol $S

sharedMemory.o: sharedMemory.cpp
	$(GPP) -c sharedMemory.cpp

clean:
	rm ./UnixDomSock ./unix_sock.* ./SharedMemorySol ./PipesSol

