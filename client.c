#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "common.h"

#define BUFSZ 1024
#define MAX_VIZ 11

viz vizinhos[MAX_VIZ];
int viz_qtd = 0;

//--------------------------------Erros-----------------------------------------------------------

void usage(int argc, char **argv) {
    printf("Usage: %s <server_ip> <port> -type <temperature|humidity|air_quality> -coords <x> <y>\n", argv[0]);
    exit(EXIT_FAILURE);
//--------------------------------------------------------------------------------------------------
}
// Compara se dois sensores estão na mesma posição
int sensor_compare(sensor_message *sensort, sensor_message *sensor){
    if (sensort->coords[0] == sensor->coords[0] && sensort->coords[1] == sensor->coords[1]){
        return 1;
    }else{
        return 0;
    }
}
// Calcula a distância euclidiana entre dois sensores
float distance(sensor_message *sensort, sensor_message *sensor){
    return sqrt(pow(sensor->coords[0] - sensort->coords[0], 2) + pow(sensor->coords[1] - sensort->coords[1], 2));
}

//------------------------ Função para calcular nova medicao-------------------------
float calculate_new_measurement(sensor_message *sensort, sensor_message *sensor ) { 

    
    float medi_atualizada = 0.1 * (1.0 / (distance(sensort, sensor) + 1.0)) * (sensort->measurement - sensor->measurement);
    float measurment_atual = sensor->measurement + medi_atualizada;
     switch (sensor->type[0]) { 
        case 't': 
            if (strcmp(sensor->type, "temperature") == 0) {
                if(sensor->measurement>40){
                    sensor->measurement=40;
                }else if(sensor->measurement<20){
                    sensor->measurement=20;
                }
            }
            break;
        case 'h': 
            if (strcmp(sensor->type, "humidity") == 0) {
                if(sensor->measurement>90){
                    sensor->measurement=90;
                }else if(sensor->measurement<10){
                    sensor->measurement=10;
                }
            }
            break;
        case 'a':
            if (strcmp(sensor->type, "air_quality") == 0) {
                if(sensor->measurement>30){
                    sensor->measurement=30;
                }else if(sensor->measurement<15){
                    sensor->measurement=15;
                }
            }
            break;
        default:
            break;
        }
    sensor->measurement = measurment_atual;

    // Retorna a variação aplicada 
    return medi_atualizada;
}

void random_number(sensor_message *sensor) {// Gera os valores
    srand(time(0));
    if (strcmp(sensor->type, "temperature") == 0) {
       float x =((float)rand() / RAND_MAX) * 20.0 + 20.0;  
        sensor->measurement = x;
    } else if (strcmp(sensor->type, "humidity") == 0) {
        float x = ((float)rand() / RAND_MAX) * 80.0 + 10.0;  
         sensor->measurement = x;
    } else if (strcmp(sensor->type, "air_quality") == 0) {
        float x = ((float)rand() / RAND_MAX) * 15.0 + 15.0;  
         sensor->measurement = x;
    } 
}

//------------------------------------------------------------------------------------------------

void add_viz(sensor_message *sensort, sensor_message *sensor){
    for(int i = 0; i<viz_qtd; i++){
        if(sensor_compare(sensort,&vizinhos[i].sensor)==1){
            vizinhos[i].sensor=*sensort;// Atualiza o vizinho
            vizinhos[i].distancia=distance(sensort,sensor);
            return;
        }
    }

    if(viz_qtd<MAX_VIZ){
        vizinhos[viz_qtd].sensor=*sensort;
        vizinhos[viz_qtd].distancia=distance(sensort,sensor);
        viz_qtd++;
    }

// Ordena os vizinhos pela distância
    for (int j = 0; j < viz_qtd - 1; j++) {
        for (int k = j + 1; k < viz_qtd; k++) {
            if (vizinhos[j].distancia > vizinhos[k].distancia) {
                viz temp = vizinhos[j];
                vizinhos[j] = vizinhos[k];
                vizinhos[k] = temp;
            }
        }
    }
}
// Remove um vizinho com base na sua coordenada
void rm_viz(sensor_message *sensort){
    for(int i =0; i<viz_qtd;i++){
        if(sensor_compare(sensort,&vizinhos[i].sensor)==1){
            for(int j = i;j<viz_qtd-1;j++){
                vizinhos[j]=vizinhos[j+1];
            }
            viz_qtd--;
            break;
        }
    }
}

int main(int argc, char **argv) {
	
//------------------------------------Erros------------------------------------------------------
 // Verifica os argumentos e valida
 
 if (argc != 8) {//lembrar de corrigir
        fprintf(stderr, "Error: Invalid number of arguments\n");
        usage(argc, argv);
    }

    if (strcmp(argv[3], "-type") != 0) {
        fprintf(stderr, "Error: Expected '-type' argument\n");
        usage(argc, argv);
    }

    const char *sensor_type = argv[4];
    if (strcmp(sensor_type, "temperature") != 0 && strcmp(sensor_type, "humidity") != 0 && strcmp(sensor_type, "air_quality") != 0) {
        fprintf(stderr, "Error: Invalid sensor type\n");
        usage(argc, argv);
    }

    if (strcmp(argv[5], "-coords") != 0) {
        fprintf(stderr, "Error: Expected '-coords' argument\n");
        usage(argc, argv);
    }

    int x = atoi(argv[6]);
    int y = atoi(argv[7]);
    if (x < 0 || x > 9 || y < 0 || y > 9) {
        fprintf(stderr, "Error: Coordinates must be in the range 0-9\n");
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (addrparse(argv[1], argv[2], &storage) != 0) {
        usage(argc, argv);
    }

    int sock = socket(storage.ss_family, SOCK_STREAM, 0);
    if (sock == -1) logexit("socket");

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (connect(sock, addr, sizeof(storage)) != 0) logexit("connect");

    //printf("Client connected to server!\n");
//-------------------------------------------------------------------------------------------------

    sensor_message sensor;   
    int value1;
    int value2;

	 value1 = atoi(argv[6]);value2 = atoi(argv[7]); 
     
    sensor.coords[0] = value1; sensor.coords[1] = value2;
   
    strcpy(sensor.type, argv[4]);

    random_number(&sensor);
    
	float measurment_atual;

    int delay = 0; // Inicializa o tempo de atraso padrão
    switch (sensor.type[0]) { 
    case 't':
        if (strcmp(sensor.type, "temperature") == 0) {
            delay = 5;
        }
        break;
    case 'h': 
        if (strcmp(sensor.type, "humidity") == 0) {
            delay = 7;
        }
        break;
    case 'a': 
        if (strcmp(sensor.type, "air_quality") == 0) {
            delay = 10;
        }
        break;
    default:
        delay = 1; 
        break;
    }

	int i;

    sensor_message rsensor;
	while(1) {
		
        i=0;
        
        while(i<delay){
            sleep(1);
            ssize_t Data_result = recv(sock, &rsensor, sizeof(rsensor) + 1, MSG_DONTWAIT);
            if(Data_result>0){
                if(strcmp(sensor.type,rsensor.type) == 0){
                
                    printf("%s sensor in\n",rsensor.type);
                	printf("(%d,%d)\n", rsensor.coords[0], rsensor.coords[1]);
                	printf("measurement: %.4f\n", rsensor.measurement);

                    if(rsensor.coords[0] == sensor.coords[0] && rsensor.coords[1] == sensor.coords[1]){
		            		printf("action: same location \n\n");
                    }else if(rsensor.measurement==-1){
                        rm_viz(&rsensor);
                        printf("action: removed\n\n");
                    }else{
                        add_viz(&rsensor, &sensor);
                        for(int j = 0; (j<viz_qtd && j<3); j++){

                            if(sensor_compare(&vizinhos[j].sensor, &rsensor)==1){
                                measurment_atual = calculate_new_measurement(&rsensor, &sensor);
                                printf("action: correction of %.4f\n\n", measurment_atual);
                                break;
                            }
                            if(j==2){
                                printf("action: not neighbor\n\n");
                            }
                        }
                        
                    }
	            }	
            }
            i++;
        }
		send(sock, &sensor, sizeof(sensor) + 1, 0);
		
	}

	close(sock);
	exit(EXIT_SUCCESS);
}
