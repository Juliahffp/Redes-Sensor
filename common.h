#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <arpa/inet.h>// Inclui funções para manipular endereços IP
#include <time.h>
#include <math.h>

#define Number_Total_Clients 12
//-----------------------------------------------------------------------------------------------
typedef struct sensor_message{
    char type[12];
    int coords[2];
    float measurement;
} sensor_message;



typedef struct {
    sensor_message sensor;// Dados do sensor
    float distancia;// Distância calculada
} viz;

//-------------------------------------------------------------------------------------------------
void logexit(const char *error_message);// Função para exibir uma mensagem de erro e finalizar o programa
// Função para validar os argumentos de entrada do sensor
void in_check(char *arguments, const char *sensor_type, const char *type_str, const char *coordinates, const char *x_coord, const char *y_coord);
// Função que converte um endereço de string
int addrparse(const char *address_str, const char *port_str, struct sockaddr_storage *addr_storage);

void convert_addr_to_str(const struct sockaddr *address, char *output_str, size_t output_size);// Função que converte um endereço IP e porta em formato binário
int server_sockaddr_init(const char *protocol, const char *port_str, struct sockaddr_storage *addr_storage);// Função para inicializar o endereço do servidor (IPv4 ou IPv6)


//-----------------------------------------------------------------------------------------------
