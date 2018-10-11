GPP = g++ -O3 -Wall -std=c++11

A = main.cc

Aprog: $A
	$(GPP) -o Aprog $A

main.o: main.h main.cc
	$(GPP) -c main.cc

clean:
	rm ./Aprog