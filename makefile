all:  test

test:	        sisctrl.o
		g++ sisctrl.o -lyaml-cpp -L. -pthread -o test 

sisctrl.o:	sisctrl.cpp
		g++ -c -Wall -I. -g sisctrl.cpp 
		
 
