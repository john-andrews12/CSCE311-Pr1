GPP = g++ -O3 -Wall -std=c++11

A = newtry.cc
B = newtrysending.cc
C = alltogethere.cc

Aprog: $C
	$(GPP) -o Aprog $C

alltogethere.o: alltogethere.cc
	$(GPP) -c alltogethere.cc

Aprog2: $A
	$(GPP) -o Aprog2 $A

newtry.o: newtry.cc
	$(GPP) -c newtry.cc
	
Aprog1: $B
	$(GPP) -o Aprog1 $B

newtrysending.o: newtrysending.cc
	$(GPP) -c newtrysending.cc

clean:
	rm ./Aprog*