FLAGS= -g -o 
CFLAGS= -g -c -DUSER_MODE 
CC=gcc
CCP=g++
SYSFUNCOBJ = sysfunc.o 

main: test_main ../lib/libsysfunc.a
test_main:  test_main.c ../lib/libsysfunc.a  
	$(CC) $(FLAGS) test_main test_main.c -L../lib -lsysfunc -lcrypto_func 
../lib/libsysfunc.a: $(SYSFUNCOBJ) 	
	ar rs libsysfunc.a $(SYSFUNCOBJ)
	mv libsysfunc.a ../lib
sysfunc.o:  sysfunc.c ../include/sysfunc.h
	$(CC) $(CFLAGS) sysfunc.c 
#load_message.o:  load_message.c message_box.h
#	$(CC) $(CFLAGS) load_message.c 
clean:
	rm -f test_main *~ *.o *.a
