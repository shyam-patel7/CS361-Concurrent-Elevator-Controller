SCHEDULER=hw6.c
BINARIES=hw6
CCOPT=-g --std=gnu99
LIBS=-lpthread

all: $(BINARIES)

hw6: main.c hw6.h $(SCHEDULER)
	@gcc $(CCOPT) main.c $(SCHEDULER) -o hw6 $(LIBS)

burn-in: clean hw6.c
	gcc $(CCOPT) -DDELAY=1000 -DLOG_LEVEL=5 -DPASSENGERS=100 -DTRIPS_PER_PASSENGER=100 -DELEVATORS=10 -DFLOORS=40 main.c $(SCHEDULER) -o hw6 $(LIBS)
	./hw6

test1: clean $(SCHEDULER)
	@gcc -DDELAY=10000   -DPASSENGERS=10 -DELEVATORS=1 -DFLOORS=5 -g --std=c99 main.c $(SCHEDULER) -o hw6 $(LIBS) $(CCOPT)

test2: clean $(SCHEDULER)
	@gcc -DDELAY=10000   -DFLOORS=2 -g --std=c99 main.c $(SCHEDULER) -o hw6 $(LIBS) $(CCOPT)

test3: clean $(SCHEDULER)
	@gcc -DDELAY=10000   -DELEVATORS=10 -g --std=c99 main.c $(SCHEDULER) -o hw6 $(LIBS) $(CCOPT)

leaderboard: clean $(SCHEDULER)
	@gcc main.c $(SCHEDULER) -o hw6 $(LIBS)



clean:
	@rm -rf *~ *.dSYM $(BINARIES)

