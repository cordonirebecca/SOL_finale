# Makefile
CC	=  gcc 
CFLAGS	= -lm -Wall -pedantic -g 

.PHONY: clean all

objects = farm.o
objectsGeneraFile =generafile.o

farm: farm.o auxiliaryMW.o workers.o list.o collector.o 
	$(CC) -pthread -o $@ $^
	
generafile: $(objectsGeneraFile)
	$(CC) -o $@ $^
	
collector.o: collector.c collector.h
	$(CC) $(CFLAGS) -c $<

farm.o: farm.c auxiliaryMW.h workers.h list.h
	$(CC) $(CFLAGS) -c $< 
	
list.o: list.c list.h
	$(CC) $(CFLAGS) -c $<
	
auxiliaryMW.o: auxiliaryMW.c auxiliaryMW.h list.h
	$(CC) $(CFLAGS) -c $< 
	
workers.o: workers.c workers.h
	$(CC) $(CFLAGS) -c $< 
	
generafile.o:generafile.c generafile.h
	$(CC) $(CFLAGS) -c $< 
	
all:
	make farm
	make generafile
	
clean:
	-rm *.o
	-rm generafile
	-rm farm
	 
test:	
	make all
	./test.sh
	
