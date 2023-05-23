// Student: Lior Erenreich

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

char *IP_address;
int servers_port;
char *filepath;
int client;
FILE *file;
char *message;
int client_fd;
int current_character;


void handle_errors(int int_of_exit, int other_error_message){
    if ((errno == ETIMEDOUT) || (errno == ECONNRESET) || (errno == EPIPE)) {
        perror("Error occurred (not fatal). ");
        close(client);
        if (int_of_exit!=-1){
            exit(int_of_exit);
        }
    } 
    else if ((errno == SIGINT) || (errno == EINTR)) {
    } 
    else {
        if (other_error_message == 0){
            perror("Failed to send message size. ");
        }
        else if (other_error_message == 1){
            perror("Failed to send message. ");
        }
        else{
            perror("Failed to receive message. ");
        }
        exit(1);
    }
}

int client_socket_constractor() {
    if (0 == (client_fd = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Failed to start socket. ");
        exit(1);
    }

    return client_fd;

}
void check_and_load_input_variables(int argc, char *ip, char *serverPort, char *path){
    /**
     * from the assignment description: "You should validate that the correct number of command line arguments is passed."
     */
    if (argc != 4) {
        fprintf(stderr, "Incorrect number of command line arguments.\n");
        exit(1);
    }
    IP_address = ip;
    servers_port = atoi(serverPort);
    filepath = path;
}


int main(int argc, char *argv[]) {
    struct sockaddr_in server;
    struct in_addr server_ip;
    check_and_load_input_variables(argc, argv[1], argv[2], argv[3]);
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    if (!inet_aton(IP_address, &server_ip)){
        fprintf(stderr, "Failed to convert Internet dot address to network address. ");
        exit(1);
    }
    client = client_socket_constractor();
    server.sin_port = servers_port;

    if ((file = fopen(filepath, "rw")) == NULL) {
        perror("Failed to open file. ");
        exit(1);
    }


    if (connect(client,(struct sockaddr *) &server,sizeof(server)) < 0) {
        perror("Connection failed on the client's side. ");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    uint32_t size_host = ftell(file);
    uint32_t size_network = htonl(size_host);

    /**
     * from the assignment description: "Transfer the contents of the file to the server over the TCP connection and decieve
     * the count of printable characters compted by the server."
     */

    /**
     * from the assignment description: "The client sends the server N, the number of bytes that will be transferred (i.e., the file size).
     * The value N is a 32-bit unsigned integer in network byte order."
     */
    if (send(client, &size_network, sizeof(size_network), 0) < 0) {
        handle_errors(0, 0);
        if ((errno == ETIMEDOUT) || (errno == ECONNRESET) || (errno == EPIPE)) {
            perror("Error occurred on the client side (not fatal). ");
            close(client);
            exit(0);
        } 
        else if ((errno == SIGINT) || (errno == EINTR)) {
        } 
        else {
            perror("Failed to send message size. ");
            exit(1);
        }
    }

    rewind(file);
    message = malloc(size_host);
    memset(message, '\0', size_host);

    /**
     * now I need to load the file's content in order to send it to the server:
     */
    unsigned int i = 0U;
    while ( (current_character = fgetc(file)) ) {

        if (current_character == EOF) {
            message[i] = '\0';
            break;
        }
        else if (current_character == '\n') {
            message[i] = '\0';
            i = 0U;
            continue;
        }
        else
            message[i++] = (char)current_character;
    }

    /**
     * from the assignment description: "The client sends the server N bytes (the file's content)."
     */
    if (send(client, message, strlen(message), 0) < 0) {
        handle_errors(-1, 1);
    }

    fclose(file);

    uint32_t count_of_printable;
    if (recv(client, &count_of_printable, sizeof(count_of_printable), 0) < 0) {
        handle_errors(0, 2);
    }
    printf("# of printable characters: %u\n", count_of_printable);

    free(message);
    close(client);
    exit(0);
}