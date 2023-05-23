// Student: Lior Erenreich

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>


static uint32_t pcc_total_tmp[95] = {0};
static uint32_t pcc_total[95] = {0};
static int sigint = 0;
static int is_client = 0;
int servers_port, server;

int server_socket_constractor();
uint32_t cnt_chars_of_message(char *message);
void load_data_pcc_total(int is_from_total_to_tmp);
void sigint_handler(int signal);
int receive_message(int server);
void pcc_total_print();
int handle_errors();


int server_socket_constractor() {
    int server_fd;
    if (0 == (server_fd = socket(AF_INET, SOCK_STREAM, 0))) {
        perror("Starting socket failed");
        exit(1);
    }
    return server_fd;
}

uint32_t cnt_chars_of_message(char *message_str) {
    uint32_t cnt = 0;
    int pcc_total_i;
    int val;
    for (int j = 0; j < strlen(message_str); j++) {
        val = (int) message_str[j];
        if ((val >= 32) && (val <= 126)) {
            pcc_total_i = (val-32);
            pcc_total_tmp[pcc_total_i]++;
            cnt++;
        }
    }

    return cnt;
}

void load_data_pcc_total(int is_from_total_to_tmp) {
    if (!is_from_total_to_tmp) {
        for (int i = 0; i < 95; i++) {
            pcc_total_tmp[i] = pcc_total[i];
        }
    } 
    else{
        for (int i = 0; i < 95; i++) {
            pcc_total[i] = pcc_total_tmp[i];
        }
    }
}

int handle_errors(int to_load, int connection, sigset_t sigset, sigset_t oldset, int error_message){
    if ((errno == ETIMEDOUT) || (errno == ECONNRESET) || (errno == EPIPE)) {
        perror("Error occurred on the server side (not fatal). "); //
        close(connection); //
        if (to_load){
            load_data_pcc_total(0);
        }
        is_client = 0;
        sigprocmask(SIG_UNBLOCK, &sigset, &oldset);
        return 0;
    } 
    else if ((errno == SIGINT) || (errno == EINTR)) {
        return 0;
    } 
    else {
        if (error_message==0){
            perror("Failed to receive message's size. ");
        }
        else if (error_message==1){
            perror("Failed to receive message. ");
        }
        else{
            perror("Failed to send message. ");
        }
        is_client = 0;
        sigprocmask(SIG_UNBLOCK, &sigset, &oldset);
        return 1;
    }

}

int receive_message(int server) {
    struct sockaddr_in client;
    char *message;
    int connection;
    int client_size = sizeof(client);

    while (!sigint) {

        is_client = 0;
        sigset_t sigset, oldset;
        sigemptyset(&sigset);
        sigaddset(&sigset, SIGINT);
    
        connection = accept(server,(struct sockaddr *) &client,(socklen_t *) &client_size);
        sigprocmask(SIG_BLOCK, &sigset, &oldset);

        if (sigint) { 
            break; 
        }
        is_client = 1;

        if (connection < 0) {
            perror("Connection failed. ");
            is_client = 0;
            sigprocmask(SIG_UNBLOCK, &sigset, &oldset);
            return 1;
        }

        int N_size, N_host;
        if (recv(connection,&N_size, 4, 0) < 0) {
            if (handle_errors(0, connection, sigset, oldset, 0)){
                return 1;
            }
        }
        N_host = ntohl(N_size);
        message = malloc(sizeof(char) * N_host);
        memset(message, '\0', sizeof(char) * N_host);

        //Receive the message from the client
        //Checking if receiving action went well
        if (recv(connection,message,sizeof(char) * N_host, 0) < 0) {
            if (handle_errors(0, connection, sigset, oldset, 1)){
                return 1;
            }
        }

        //Sending the number of printable characters to client
        uint32_t printable_chars_cnt = cnt_chars_of_message(message);
        if (send(connection, &printable_chars_cnt, sizeof(printable_chars_cnt), 0) < 0) {
            if (handle_errors(1, connection, sigset, oldset, 2)){
                return 1;
            }
        }

        //Close the connection of the current client
        close(connection);
        load_data_pcc_total(1);
        free(message);

        //Now SIGINT is unblocked and if a signal was received during the flow
        //we will exit the loop
        is_client = 0;
        sigprocmask(SIG_UNBLOCK, &sigset, &oldset);
    }

    return 0;
}

void sigint_handler(int signal) {
    if (!is_client) {
        sigint = 1;
    }
}

void pcc_total_print() {
    for (int i = 0; i < 95; i++) {
        printf("char '%c' : %u times\n", (i+32), pcc_total[i]);
    }
}

int main(int argc, char *argv[]) {
    struct sockaddr_in socket_address;

    /**
     * Validation that the correct number of command arguments is passed.
     */
    if (argc != 2) {
        fprintf(stderr, "Incorrect number of arguments. ");
        exit(1);
    }

    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server = server_socket_constractor();

    /**
     * From the assignment description: "Use the SO_REUSEADDR socket option to enable
     * reusing the port quickly after the server terminates."
     */

    if (1 == setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int) {1}, sizeof(int))) {
        perror("Failed to set socket options. ");
        exit(1);
    }

    servers_port = atoi(argv[1]);
    socket_address.sin_port = servers_port;

    if (0 != bind(server,(struct sockaddr *) &socket_address,sizeof(socket_address))) {
        perror("Failed to bind to server. ");
        exit(1);
    }

    if (0 != listen(server, 10)) {
        perror("Failed to listen to server. ");
        exit(1);
    }

    //Setting the handler for SIGINT
    struct sigaction sigaction_for_sigint = {
            .sa_handler = sigint_handler
    };

    if ((-1) == sigaction(SIGINT, &sigaction_for_sigint, NULL)) {
        fprintf(stderr, "SIGINT handling error occurred. ");
        if (!is_client) {
            sigint = 1;
        }
    }

    /**
     * From the assignment description: "For every printable character, print the number of times it 
     * was observed (possibly 0) to standard output.."
     */
    if (0 != receive_message(server)) {
        perror("Failed to receive message. ");
        exit(1);
    }

    /**
     * from the assignment description: "If the server is processing a client when SIGINT is delivered, finish handling this client
     * (including updating the pcc_total global data structure). For every printable character, print the number of times it 
     * was observed (possibly 0) to standard output.."
     */
    pcc_total_print();
    close(server);
    return 0;
}