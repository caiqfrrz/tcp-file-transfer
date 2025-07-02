all:
	g++ -std=c++17 ./src/client.cpp -o ./out/client -lssl -lcrypto
	g++ -std=c++17 ./src/server.cpp -o ./out/server -lssl -lcrypto

client:
	g++ -std=c++17 ./src/client.cpp -o ./out/client -lssl -lcrypto

server:
	g++ -std=c++17 ./src/server.cpp -o ./out/server -lssl -lcrypto