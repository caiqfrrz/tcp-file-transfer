all:
	g++ ./src/client.cpp -o ./out/client
	g++ ./src/server.cpp -o ./out/server

client:
	g++ ./src/client.cpp -o ./out/client

server:
	g++ ./src/server.cpp -o ./out/server