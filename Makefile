ahs_bash: ahs_bash.o ahs_parser.o ahs_netbash.o
	gcc -o ahs_bash ahs_bash.o ahs_parser.o ahs_netbash.o -Wall

ahs_bash.o: ahs_bash.c ahs_bash.h ahs_parser.h
	gcc -o ahs_bash.o ahs_bash.c -c -Wall

ahs_netbash.o: ahs_netbash.c ahs_netbash.h ahs_bash.h ahs_parser.h
	gcc -o ahs_netbash.o ahs_netbash.c -c -Wall

ahs_parser.o: ahs_parser.c ahs_parser.h
	gcc -o ahs_parser.o ahs_parser.c -c -Wall

clean:
	rm *.o -f
