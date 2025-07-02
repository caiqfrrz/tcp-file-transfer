# TCP File Transfer and Chat Message Application

## Overview

This project is a simple TCP-based client-server application implemented in C++ that allows:

- **File transfer**: clients can download files from the server with SHA256 integrity check.
- **Chat messaging**: clients can send broadcast messages to all connected clients, and the server can also send messages to all clients.

It is designed as a learning tool for the Computer Networks class at UTFPR to understand how TCP sockets, basic concurrency (via threads) and simple protocols work.

---

## Features

- Multi-client support with server threads.
- Download files from the server with progress bar on client.
- SHA256 checksum verification after file download.
- Broadcast chat messages:
  - Client -> all other clients
  - Server -> all clients
- File listing on the server.

---

## Building

**Dependencies:**  
- C++17 compiler (e.g. g++ >= 7)  
- OpenSSL (for SHA256)
- Make

```bash
make
```

This will produce:

- `./out/server/` - the TCP server
- `./out/client/` - the client

```bash
./out/server
```

The server console allows you to type messages to broadcast to all connected clients.

### Start the client
Run the client by specifying the server IP:

```bash
./out/client <server_ip>
```

Example:

```bash
./out/client 192.168.0.10
```

---

## Usage (commands from client)

```plaintext
file <file-name>          -> download a file from the server
message <message-text>    -> send a message to all clients
list                      -> list files available on the server
help                      -> show help menu
quit                      -> disconnect from server
```

---

## SHA256 integrity
When downloading a file, the client computes a local SHA256 hash and compares it with the one sent by the server to ensure the file was transferred correctly.

---

## Notes
- The server binds to INADDR_ANY, so clients in the same LAN should be able to connect by providing the server's LAN IP.
- This code is **extremely** unsafe, vulnerable to path traversal and etc, take care and feel free to make it secure!


