01234567890123456789012345678901234567890123456789012345678901234567890123456789
README for project 1, CSCE 311-001, due Oct-23-2018

GROUP MEMBERS:
John Andrews, jha2@email.sc.edu
Edmond Klaric, eklaric@email.sc.edu

PREREQUISITES:
This program should be run on one of the lab machine in the Linux labs in
Swearingen Engineering Center.

RUNNING THE PROGRAM:
There are three programs that can be run with the included files. These programs
demonstrate the implementation of using Unix Domain Sockets, Pipes, and Shared
Memory for IPC. (The following instructions assume that you have directed 
yourself within the terminal into the directory holding all of the given code
files) To complile the program for demonstrating the Unix Domain Socket method,
enter 'make UnixDomSockSol' into the terminal to create the executable 
(named UnixDomSockSol). To compile the program for demonstrating the Pipe 
method, enter 'make PipesSol' into the terminal to create the executable (named 
PipesSol). To compile the program for demonstrating the Shared Memory solution
enter 'make SharedMemorySol' into the terminal to create the executable (named 
SharedMemorySol). All of the executables have form: 

	./<executable-name> ./<filename> <keyword> 

Here, executable-name is the name of the executable you are trying to run. The
filename is the name of a text file located IN THE SAME DIRECTORY as the 
executable that you wish to open to search for the keyword. And thus, the 
keyword is a word that you want to find in the file depicted by filename.

INTERPRETING THE OUTPUT
For the UnixDomSockSol and the PipesSol executables, the output has form:

	Lines containing keyword alphabetically:
	<line1>
	<line2>
	...

where the <line#>s are the lines containing the keyword in alphabetical order.
If there are no lines under the 'lines Containing keyword alphabetically:' then
the keyword was not found in the file. For the SharedMemorySol executable, the
output has form:

	Data written in memory:
	 <line1>
	<line2>
	....

where the <line#>s are the lines containing the keyword in alphabetical order. 
If the keyword is not found in the file, then instead of having <line#>s, it
will simply say 'keyword not found'.

EXAMPLE TEST
If I wanted to test the execution of the Pipe implementation for IPC, I would:
1. Navigate within the terminal to the directory with the source code.
2. Run 'make PipesSol'
3. Ensure that whatever text file I wanted to search is in the same directory 
   as the executable (for these purposes lets call this file 'dummy.txt').
4. Execute the program by entering './PipesSol ./dummy.txt yesterday' (here we
   are search for the word 'yesterday').
5. View the results. 

CONTRIBUTIONS BY GROUP MEMBER
-pipes_solution.cc :: written and tested by John Andrews
-unix_dom_sock_sol.cc :: written and tested by John Andrews
-sharedMemory.cpp :: written and tested by Edmond Klaric
-Report question 1 :: John Andrews
-Report question 2 :: Edmond Klaric 
-Report question 3 :: Edmond Klaric 
-Report question 4 :: Edmond Klaric
-README :: John Andrews
-makefile :: Edmond Klaric