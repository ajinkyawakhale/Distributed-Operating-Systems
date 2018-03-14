all: compile_server compile_client start_server	
compile_server:
		gcc -o server server.c -pthread
compile_client:
		gcc -o client client.c
start_server:
		./server 7867
start_client:
		./client localhost 7867 0.1
clean:
		rm server client
