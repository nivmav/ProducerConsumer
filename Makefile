warmup2: warmup2.o my402list.o server.o arrivalt.o tokbucket.o queue.o token.o 
	gcc -o warmup2 -g warmup2.o my402list.o server.o arrivalt.o tokbucket.o queue.o token.o -lpthread -lm

warmup2.o: warmup2.c warmup2.h my402list.h server.h arrivalt.h tokbucket.h queue.h token.h
	gcc -g -c -Wall warmup2.c

my402list.o: my402list.c my402list.h
	gcc -g -c -Wall my402list.c

server.o: server.c server.h queue.h my402list.h warmup2.h
	gcc -g -c -Wall server.c

arrivalt.o: arrivalt.c arrivalt.h  queue.h token.h my402list.h warmup2.h
	gcc -g -c -Wall arrivalt.c

tokbucket.o: tokbucket.c tokbucket.h server.h queue.h token.h my402list.h warmup2.h
	gcc -g -c -Wall tokbucket.c

queue.o: queue.c queue.h my402list.h warmup2.h
	gcc -g -c -Wall queue.c

token.o: token.c token.h my402list.h warmup2.h
	gcc -g -c -Wall token.c
clean:
	rm -f *.o warmup2
