#ifndef readb_h
#define readb_h

/*
    Este módulo implementa um Buffer sobre um certo descritor de ficheiro.
*/
#include <unistd.h>

typedef struct buffer_t * Buffer_t; 

/* 
    Esta função cria uma instância de Buffer_t com o tamanho especificado. 
*/
Buffer_t mkBuffer(int filedes, size_t nbyte);

/* 
    Esta função destroy uma instância de Buffer_t.
*/
int unmkBuffer(Buffer_t buffer);

/* 
    Esta função lê uma linha e alocando memória dinâmica altera o valor da variavel
        apontadora recebida (buf).
    Caso buffer seja consumido é lida uma nova porção do ficheiro associado.
    Devolve o numero de caracters lidos.
*/
ssize_t readln(Buffer_t buffer, void **buf);

/*
    Esta função retorna a instâmcia do Buffer_t recebido para o inicio do ficheiro. 
*/
int remkB(Buffer_t buffer);

#endif