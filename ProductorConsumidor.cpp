#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <cstdio> 
#include <cstdlib>
#include <unistd.h>
#include <sstream> 
#include <string>   
#include <iomanip>
#define ROJO "\033[1;31m"
#define VERDE "\033[1;32m"
#define NORMAL "\033[1;0m"                               
using namespace std ;

//Declaración de variables globales 
sem_t semProductor;     
sem_t semConsumidor;    
sem_t mutex;
sem_t mutexPantalla;                    
int posicionVacia = 0;
bool terminanProducir;
bool terminanConsumir;
int numeroProductoras;
int numeroConsumidoras;
int tamanoVector;
int *buffer;


void imprimirRojo() {
    cout << "Buffer actual: ";
    for (int i = 0; i < tamanoVector; i++) {
        if (i == posicionVacia) {
            cout << "[" << ROJO << buffer[i] << NORMAL << "]";
        } else {
            cout << "[" << buffer[i] << "]";
        }
    } 
    cout << endl;
}

void imprimirVerde() {
    cout << "Buffer actual: ";
    for (int i = 0; i < tamanoVector; i++) {
        if (i == posicionVacia) {
            cout << "[" << VERDE << buffer[i] << NORMAL << "]";
        } else {
            cout << "[" << buffer[i] << "]";
        }
    } 
    cout << endl;
}

bool hayCeros(){
    bool encontrado = false;

    for (int i = 0 ; i < tamanoVector && encontrado == false; i++) {
        if (buffer[i] == 0) {;
            encontrado = true;
        }
    return encontrado;
    }
}

int buscaCeros() {
    int posicion = 0;
    bool encontrado = false;

    for (int i = 0 ; i < tamanoVector && encontrado == false; i++) {
        if (buffer[i] == 0) {
            posicion = i;
            encontrado = true;
        }
    }
    return posicion;
}

bool hayUnos(){
    bool encontrado = false;

    for (int i = 0 ; i < tamanoVector && encontrado == false; i++) {
        if (buffer[i] == 1) {
            encontrado = true;
        }
    return encontrado;
    }
}

int buscaUnos() {
    int posicion = 0;
    bool encontrado = false;

    for (int i = 0 ; i < tamanoVector && encontrado == false; i++) {
        if (buffer[i] == 1) {
            posicion = i;
            encontrado = true;
        }
    }
    return posicion;
}


void * productor(void* arg) {
    srand(time(NULL));
    int *numeroHebra; 
    numeroHebra = (int*) arg;

    if(terminanProducir == false){
        while(true){
            sem_wait(&semProductor);
            sem_wait(&mutex);
            posicionVacia = buscaCeros();
            buffer[posicionVacia] = 1;
            sem_wait(&mutexPantalla);
            cout << "La hebra productora número " << VERDE << " (" << *numeroHebra  << ") "<< NORMAL << " está produciendo en la posición " << posicionVacia << " del buffer (vector)." << endl;
            imprimirVerde();
            sem_post(&mutexPantalla); 
            sem_post(&mutex);
            sem_post(&semConsumidor);
            sleep(1); //dormimos la hebra para que se produzca cambio de contexto ligero entre hebras
        }
    }
    else{
        sem_wait(&semProductor);
        sem_wait(&mutex);
        bool hayceros = hayCeros();
        posicionVacia = buscaCeros();
        buffer[posicionVacia] = 1;

        sem_wait(&mutexPantalla);
        cout << "La hebra productora número " << VERDE << " (" << *numeroHebra  << ") " << NORMAL << " está produciendo en la posición " << posicionVacia << " del buffer (vector)." << endl;
        imprimirVerde();
        sem_post(&mutexPantalla); 
        
        sem_post(&mutex);
        sem_post(&semConsumidor);
        sleep(1); //dormimos la hebra para que se produzca cambio de contexto ligero entre hebras
    }   
    return NULL;
}

void* consumidor(void* arg) {
    srand(time(NULL));
    int *numeroHebra; 
    numeroHebra = (int *) arg;

    if(terminanConsumir == false){
        while(true){
            sem_wait(&semConsumidor);
            sem_wait(&mutex);

            posicionVacia = buscaUnos();
            buffer[posicionVacia] = 0;

            sem_wait(&mutexPantalla);
            cout << "La hebra consumidora número " << ROJO <<  " (" << *numeroHebra  << ") " << NORMAL << " está consumiendo en la posición " << posicionVacia << " del buffer (vector)." << endl;
            imprimirRojo();
            sem_post(&mutexPantalla); 
            sem_post(&mutex);
            sem_post(&semProductor);
            sleep(1); //dormimos la hebra para que se produzca cambio de contexto ligero entre hebras

        }
    }
    else{
        sem_wait(&semConsumidor);
        sem_wait(&mutex);
        posicionVacia = buscaUnos();
        buffer[posicionVacia] = 0;

        sem_wait(&mutexPantalla);
        cout << "La hebra consumidora número " << ROJO << " (" << *numeroHebra  << ") "<< NORMAL << " está consumiendo en la posición " << posicionVacia << " del buffer (vector)." << endl;
        imprimirRojo();
        sem_post(&mutexPantalla); 
        
        
        sleep(1); //dormimos la hebra para que se produzca cambio de contexto ligero entre hebras
        sem_post(&semProductor);
        sem_post(&mutex);
    }
    return NULL;
}

int main(int argc, char **argv) {                
    if (argc == 6){
        tamanoVector = atoi(argv[1]);
        numeroProductoras = atoi(argv[2]);
        std::istringstream(argv[3]) >> std::boolalpha >> terminanProducir;
        numeroConsumidoras = atoi(argv[4]);
        std::istringstream(argv[5]) >> std::boolalpha >> terminanConsumir; 
        buffer = new int [tamanoVector];
        

        pthread_t idHebraProductora[numeroProductoras];
        pthread_t idHebraConsumidora[numeroConsumidoras];

        //limpiamos vector
        for(int i = 0; i < tamanoVector; i++){
            buffer[i] = 0;
        }
        
        sem_init(&semProductor, 0,tamanoVector);
        sem_init(&semConsumidor,0,0);
        sem_init(&mutex,0,1);
        sem_init(&mutexPantalla,0,1);

        int idArrayProductoras[numeroProductoras];
        for(int i = 0; i < numeroProductoras; i++){
            idArrayProductoras[i] = i;
        }

        int idArrayConsumidoras[numeroConsumidoras];
        for(int i = 0; i < numeroConsumidoras; i++){
            idArrayConsumidoras[i] = i;
        }
        
        //Creamos todas las hebras que nos han introducido por pantalla
        for(unsigned i = 0; i <= numeroProductoras ; i++) {
            pthread_create( &(idHebraProductora[i]),NULL,productor, (void *)&idArrayProductoras[i]); 
        }

        for(unsigned i=0; i <= numeroConsumidoras; i++) {
            pthread_create( &(idHebraConsumidora[i]),NULL,consumidor, (void *)&idArrayConsumidoras[i]); 
        }


        for(unsigned i=0; i <= numeroProductoras; ++i) {
            pthread_join(idHebraProductora[i], NULL); //Liberación de los recursos utilizados por las de hebras productoras
        }

        for(unsigned i=0; i <= numeroConsumidoras; ++i) {
            pthread_join(idHebraConsumidora[i], NULL); //Liberación de los recursos utilizados por las de hebras consumidoras
        }


        sem_destroy(&semProductor);
        sem_destroy(&semConsumidor);
        sem_destroy(&mutex);
        sem_destroy(&mutexPantalla);
    }
    
    else{
        cout << "Introduzca un número correcto de parámetros." << endl; 
    }
}