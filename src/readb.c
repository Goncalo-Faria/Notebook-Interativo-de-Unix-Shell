#include "readb.h" /* header */
#include "common.h"
/*-- Módulo de tipo abstrato de dados ----------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
/*-- Da autoria de Gonçalo Rui Alves Faria                                                    --*/
/*-- https://github.com/Goncalo-Faria                                                         --*/
/*----------------------------------------------------------------------------------------------*/

/*-- ( ) Declaração da Estrutura ---------------------------------------------------------------*/

typedef struct buffer_t
{
    char *x;
    size_t nbyte;
    int filedes;
    int end;
    int st;
    
    int flag;
    // -> iniciated 0
    // -> with content 1

} * Buffer_t;

/*-- (1) Protótipos ----------------------------------------------------------------------------*/

/* Métodos públicos */
Buffer_t mkBuffer(int filedes, size_t nbyte);
int unmkBuffer(Buffer_t buffer);
ssize_t readln(Buffer_t buffer, void **buf);
int remkB(Buffer_t buffer);

/* Métodos privados */
static int isBufferConsumed(Buffer_t buffer);

/*-- (2) Implementação --------------------------------------------------------------------------*/

/*-- (2.0) Construtores -------------------------------------------------------------------------*/

/**
 * Cria uma instância do objeto Buffer com tamanho nbyte no ficheiro filedes.
 */

Buffer_t mkBuffer(int filedes, size_t nbyte)
{
    Buffer_t buffer = (Buffer_t)malloc(sizeof(struct buffer_t));

    if (filedes == -1)
    {
        //perror(" the file didn't work well. \n");
        return NULL;
    }

    buffer->x = malloc(nbyte);
    buffer->filedes = filedes;
    buffer->nbyte = nbyte;
    buffer->flag = 0;
    buffer->st = 0;
    buffer->end = 0;

    return buffer;
}

/**
 * Destrói uma instância do objeto Buffer.
 */

int unmkBuffer(Buffer_t buffer)
{
    free(buffer->x);
    free(buffer);
    return 1;
}

/*-- (2.1) Métodos privados ---------------------------------------------------------------------*/

/**
 * Atualiza o estado do objeto para indicar se o buffer está gasto.
 */

static int isBufferConsumed(Buffer_t buffer)
{
    if (buffer->st >= buffer->end)
    {
        buffer->flag = 0;
        return 1;
    }
    return 0;
}

/*-- (2.2) Métodos públicos ---------------------------------------------------------------------*/

/**
 * Recebe o endereço da variavel apontadora que vai receber a cópia.
 *  . não põe o caracter terminador (\0).
 *  . poderá possivelemente haver erros com encontrar o fim do 
 * buffer sem identificar o \n. 
 */

ssize_t readln(Buffer_t buffer, void **buf)
{

    int n, i = 0;
    char *out;

    isBufferConsumed(buffer);

    // flag === 0;
    // tem de ler
    if (!buffer->flag)
    {
        n = read(buffer->filedes, buffer->x, buffer->nbyte);
        // n contem o numero de bytes lidos.
        if (n > 0)
        {
            buffer->end = n;
            buffer->st = 0;
            buffer->flag = 1;
        }
        else{

            return -1;
        }
    }

    for (i = buffer->st; i < buffer->end; i++)
    {
        if (buffer->x[i] == '\n')
            break;
    }
    

    n = i - buffer->st+1; // numero de bytes até o /n inclusive
    out = malloc(n);
    if(out){
        memcpy(out,buffer->x + buffer->st, n );
    }

    buffer->st = i + 1;
    *buf = (void *)out;
    return n;
}

/**
 * Esta função começa faz o file descriotor começar o ficheiro de novo.
 * 
 *  . 1 caso corra tudo bem.
 *  . 0 caso contrário.
 */

int remkB(Buffer_t buffer)
{
    return !lseek( buffer->filedes , 0 ,SEEK_SET );
}
