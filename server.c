#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "common.h"

#define Number_Total_Clients 12
#define BUFSZ 1024


//--------------------------------Erros-----------------------------------------------------------

void usage_server(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}
//-------------------------------------------------------------------------------------------------

int sensor_qtd = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int clients[Number_Total_Clients];  

struct clientes {
    int socket;
    struct sockaddr_storage client_storage;
};

// Função que distribui os dados do sensor recebido para todos os clientes conectados
void distribute_sensor_data(sensor_message msg) {
    for (int i = 0; i < sensor_qtd; i++) {
        if (send(clients[i], &msg, sizeof(msg) + 1, MSG_NOSIGNAL) != sizeof(msg) + 1) {
            //printf("Failed to send data to client %d. Removing...\n", clients[i]);
           
            for (int j = i; j < sensor_qtd - 1; j++) {
                clients[j] = clients[j + 1];
            }
            sensor_qtd--;
            i--; 
        }
    }
}
// Função executada por cada thread que gerencia a comunicação com um cliente
void * thread_c(void *data) {
    struct clientes *val = (struct clientes *)data;
    struct sockaddr *client_sockaddr = (struct sockaddr *)(&val->client_storage);
    char client_address[BUFSZ];
    convert_addr_to_str(client_sockaddr, client_address, BUFSZ);

    sensor_message sensor;
    //printf("Client %d connected. Total clients: %d\n", val->socket, sensor_qtd);
    while (1) { // Recebe os dados do cliente
        ssize_t Data_result = recv(val->socket, &sensor, sizeof(sensor) + 1, 0);
        if (Data_result <= 0) {
            if (Data_result == 0) {
                //printf("Client %d disconnected. Total clients: %d\n", val->socket, sensor_qtd);
            } else {
                perror("recv");
            }
            pthread_mutex_lock(&mutex);
            for (int i = 0; i < sensor_qtd; i++) {
                if (clients[i] == val->socket) {
                    // Remove o cliente da lista
                    for (int j = i; j < sensor_qtd - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    sensor_qtd--;
                    break;
                }
            }
            sensor.measurement=-1.0;
            printf("log:\n");
            printf("%s sensor in\n", sensor.type);
            printf("(%d,%d)\n", sensor.coords[0], sensor.coords[1]);
            printf("measurement: %.4f\n\n", sensor.measurement);
            for (int i = 0; i < sensor_qtd; i++) {
                if (send(clients[i], &sensor, sizeof(sensor) + 1, MSG_NOSIGNAL) != sizeof(sensor) + 1) {
                    //printf("Failed to send data to client %d.\n", clients[i]);
                }
            }

            pthread_mutex_unlock(&mutex);
            break; 

        }
    
            pthread_mutex_lock(&mutex);

            printf("log:\n");
            printf("%s sensor in\n", sensor.type);
            printf("(%d,%d)\n", sensor.coords[0], sensor.coords[1]);
            printf("measurement: %.4f\n\n", sensor.measurement);
            distribute_sensor_data(sensor);
            pthread_mutex_unlock(&mutex);
        
    }  

   
    close(val->socket);  
    free(val);          
    pthread_exit(NULL); 

}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage_server(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage_server(argc, argv);
    }

    int sock;
    sock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sock == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(sock, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(sock, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    convert_addr_to_str(addr, addrstr, BUFSZ);

    while(1) {

        struct sockaddr_storage client_address_storage;
        struct sockaddr *client_sockaddr = (struct sockaddr *)(&client_address_storage);
        socklen_t client_sockaddrlen = sizeof(client_address_storage);

        int socket = accept(sock, client_sockaddr, &client_sockaddrlen);

        if (socket == -1) {
            logexit("accept");
        }

      
        pthread_mutex_lock(&mutex);// Bloqueia o mutex
        if (sensor_qtd < Number_Total_Clients) {
            clients[sensor_qtd] = socket;
            sensor_qtd = sensor_qtd + 1;
        } else {
            close(socket);
        }   
        pthread_mutex_unlock(&mutex);// Libera o mutex
        struct clientes *val = malloc(sizeof(*val));
        if (!val) {
            logexit("malloc");
        }
        val->socket = socket;
        memcpy(&(val->client_storage), &client_address_storage, sizeof(client_address_storage));
        pthread_t n_thread;// Cria uma nova thread para lidar com o cliente
        pthread_create(&n_thread, NULL, thread_c, val);
        pthread_detach(n_thread);
    }

    exit(EXIT_SUCCESS);
}
