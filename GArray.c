#include "GArray.h" /* header file*/
#include <string.h> /* memcpy */
#include <stdlib.h> /* realloc malloc free */

/*-- Módulo de tipo abstrato de dados ----------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
/*-- Da autoria de Gonçalo Rui Alves Faria                                                    --*/
/*-- https://github.com/Goncalo-Faria                                                         --*/
/*----------------------------------------------------------------------------------------------*/

/*-- ( ) Declaração da Estrutura ---------------------------------------------------------------*/

typedef struct garry
{
    char *v;
    int use;
    int cap;
    size_t mem;
} * GArray;

/*-- (1) Protótipos ----------------------------------------------------------------------------*/

/* Métodos públicos */
GArray mkGArray(size_t mem);
void unmkGArray(GArray in, void (*freef)(void *));
void *getG(GArray in, int i);
void *cloneG(GArray in);
void setG(GArray in, int i, void *el);
int lengthG(GArray in);

/*-- (2) Implementação --------------------------------------------------------------------------*/

/*-- (2.0) Construtores -------------------------------------------------------------------------*/

/**
 * Cria uma instância do módulo GArray, que é uma implementação  
 *  de um growing array.
 * Recebe como argumento o tamanho de cada elemento do GArray.
 */

GArray mkGArray(size_t mem)
{
    GArray out = malloc(sizeof(struct garry));
    out->v = malloc(20 * mem);
    out->use = 0;
    out->cap = 20;
    out->mem = mem;

    return out;
}

/**
 * Destroi uma instância do módulo GArray.
 */

void unmkGArray(GArray in, void (*freef)(void *))
{
    int i;
    if (freef)
    {
        for (i = 0; i < in->use; i++)
        {
            freef(in->v + i * in->mem);
        }
    }
    free(in->v);
    free(in);
}

/*-- (2.1) Métodos públicos ---------------------------------------------------------------------*/

/**
 * Apontador para o elemento no array com o indice indicado. 
 */

void *getG(GArray in, int i)
{
    if(i < in->use)
        return (void *)(in->v + i * in->mem);
    else
        return NULL;
}

/**
 * Converte o GArray para um array normal.
 */

void *cloneG(GArray in)
{
    return memcpy(malloc(in->use * in->mem), in->v, in->use * in->mem);
}

/**
 * Substitui o elemento do indice indicado pelo recebido valor.
 */

void setG(GArray in, int i, void *el)
{
    if (i >= in->cap)
    {
        in->cap *= 2;
        in->v = (char *)realloc(in->v, sizeof(in->mem) * in->cap);
        setG(in, i, el);
    }
    else if( i< in->use )
    {
        memcpy(in->v + i * in->mem, el, in->mem);
    }
    else {
        in->use++;
        memcpy(in->v + i * in->mem, el, in->mem);
    }
}

/**
 * Devolve o numero de elementos no GArray (entradas preenchidas).
 */

int lengthG(GArray in)
{
    return in->use;
}

