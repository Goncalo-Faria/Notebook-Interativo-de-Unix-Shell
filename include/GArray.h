#ifndef GArray_h
#define GArray_h

/*
    Este módulo implementa um Growing Array que está continguo em memória.
*/

#include <unistd.h>

typedef struct garry * GArray;

/*  
    Métodos construtores, responsáveis por criar ou terminal instâncias deste módulo. 
*/
GArray mkGArray(size_t mem);
void unmkGArray( GArray in,void (*freef)(void *));

/*
    Métodos de acesso aos atributos de uma instância deste módulo.
*/
void* getG( GArray in, int i );
void setG( GArray in ,int i , void* values);

/*
    Este método converte uma instância do GArray num Array convencional de C.
*/
void *cloneG(GArray in);

/*
    Consulta o numero de entradas que uma dada instância contem.
*/
int lengthG( GArray in);

#endif
