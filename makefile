all:
	g++ -std=c++17 ./src/client.cpp -o ./out/client
	g++ -std=c++17 ./src/server.cpp -o ./out/server

client:
	g++ -std=c++17 ./src/client.cpp -o ./out/client

server:
	g++ -std=c++17 ./src/server.cpp -o ./out/server