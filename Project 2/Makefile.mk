all: compile_sender compile_sequencer
compile_sender:
		gcc -o sender_test sender_test.c -pthread
compile_sequencer:
		gcc -o sequencer sequencer.c
clean:
		- rm *.o
