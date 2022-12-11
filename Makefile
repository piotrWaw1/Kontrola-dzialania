all:
	gcc client.c -o client
	gcc server.c -o server

init:
	sudo apt-get update
	sudo apt-get install gcc