# Printable Characters Counting Server - Operating Systems TAU

This project is an implementation of a client-server architecture for counting printable characters in a file. It consists of a client program (`pcc_client.c`) and a server program (`pcc_server.c`). The project was developed as part of the Operating Systems course (0368-2162) at Tel Aviv University in semester 1/2021, taught by Prof. Adam Morrison.


## Contents

- `Assignment - pcc.pdf`: The assignment document that provides instructions and guidelines for the project.
- `pcc_client.c`: The source code for the client program.
- `pcc_server.c`: The source code for the server program.
- `README.md`: This file, providing an overview of the project.

## Introduction

The Printable Characters Counting Server project aims to count the number of printable characters in a given file. It employs a client-server architecture, where the client sends the file's contents to the server, and the server counts the printable characters before returning the count to the client.

## Project Structure

The project consists of two main components: the client and the server.

### Client

The client program, `pcc_client.c`, is responsible for establishing a connection with the server, sending the file's contents, and receiving the count of printable characters from the server.

The client program follows these steps:

1. Command line argument validation: The client checks if the correct number of command line arguments is passed.
2. Initialization: The client initializes the necessary variables, such as the IP address, server port, and file path.
3. Socket creation: The client creates a socket using the `socket` system call.
4. File opening: The specified file is opened for reading and writing.
5. Connection: The client establishes a connection with the server using the `connect` system call.
6. File size transmission: The client sends the size of the file (in network byte order) to the server using the `send` system call.
7. File content transmission: The client reads the file's contents, stores them in a buffer, and sends the buffer to the server using the `send` system call.
8. File closing: The client closes the file.
9. Result retrieval: The client receives the count of printable characters from the server using the `recv` system call.
10. Output: The client prints the count of printable characters to the console.
11. Cleanup: The client frees memory, closes the socket, and exits.

### Server

The server program, `pcc_server.c`, is responsible for receiving the file's contents from the client, counting the printable characters, and sending the count back to the client.

The server program follows these steps:

1. Initialization: The server initializes the necessary variables, such as the server port and file buffers.
2. Socket creation: The server creates a socket using the `socket` system call.
3. Binding: The server binds the socket to a specific port using the `bind` system call.
4. Listening: The server listens for incoming connections using the `listen` system call.
5. Accepting connections: The server accepts incoming connections from clients using the `accept` system call.
6. File size retrieval: The server receives the size of the file from the client using the `recv` system call.
7. File content retrieval: The server receives the file's contents from the client using the `recv` system call.
8. Printable characters counting: The server counts the number of printable characters in the received file contents.
9. Result transmission: The server sends the count of printable characters back to the client using the `send` system call.
10. Cleanup: The server closes the client connection, closes the socket, and exits.

## Usage

1. Compile the client and server programs using a C compiler. For example:
   ```
   gcc pcc_client.c -o pcc_client
   gcc pcc_server.c -o pcc_server
   ```
2. Run the server program on the desired machine:
   ```
   ./pcc_server
   ```
3. Run the client program on a different machine, providing the required command line arguments:
   ```
   ./pcc_client <server_ip> <server_port> <file_path>
   ```
   Replace `<server_ip>` with the IP address of the machine running the server program, `<server_port>` with the port number used by the server, and `<file_path>` with the path to the file you want to count the printable characters in.
