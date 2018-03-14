all: compile_back_server compile_client compile_front_server start_back_server1 start_back_server2 start_back_server3	
compile_back_server:
		gcc -o server1 server1.c -pthread
compile_client:
		gcc -o client client.c
compile_front_server:
		gcc -o front_server front_server.c -pthread			
clean:
		rm server1 client front_server
