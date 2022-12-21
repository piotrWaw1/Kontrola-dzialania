all:
	gcc client.c -o client -lsqlite3
	gcc server.c -o server

init:
	sudo apt-get update
	sudo apt-get install gcc
	sudo apt-get install sqlite3
	sudo apt-get install libsqlite3-dev