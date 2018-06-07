#include "sshell.h" /* header file */
#include "common.h"
#include "mysys.h"  /* mysystem */
#include "readb.h"  /* mkBuffer unmkBuffer readln*/
#include <stdio.h>  /* perror */

/*-- Módulo de tipo abstrato de dados ----------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
/*-- Da autoria de Gonçalo Rui Alves Faria                                                    --*/
/*-- https://github.com/Goncalo-Faria                                                         --*/
/*----------------------------------------------------------------------------------------------*/
/*-- (1) Protótipos ----------------------------------------------------------------------------*/
/* Métodos públicos */
s_pipe s_open(s_pipe fin, char *command);
void s_close(s_pipe s);
s_pipe s_echo(char *str);
s_pipe s_fork(s_pipe fin, int fileds);

/*-- (2) Implementação --------------------------------------------------------------------------*/

/*-- (2.0) Construtores -------------------------------------------------------------------------*/

/*  Desc:
    . executa o comando com o stdin no ficheiro indicado e devolve o descritor com o output*/
/* warn: 
    . não fecha o fin 
    . gera um filho e não espera  */

s_pipe s_open(s_pipe fin, char *command)
{
    int pd[2], r;
    s_pipe ss;

    if (pipe(pd) == -1)
    {
        ss = S_ERR;
    }
    else
    {

        if ((r = fork()) == -1)
        {
            close(pd[0]);
            close(pd[1]);
            return S_ERR;
        }
        else if (!r)
        {
            if (fin)
            {
                dup2(fin, 0);
                s_close(fin);
            }
            dup2(pd[1], 1);

            close(pd[1]);
            close(pd[0]);

            mysystem(command);

            _exit(0);
        }

        close(pd[1]);
        wait(NULL);

        ss = pd[0];
    }

    return ss;
}

void s_close(s_pipe s)
{
    close(s);
}

/*-- (2.1) Métodos públicos ---------------------------------------------------------------------*/

s_pipe s_echo(char *str)
{

    int pd[2], r;
    s_pipe ss;

    if (pipe(pd) == -1)
    {
        ss = S_ERR;
    }
    else
    {
        if ((r = fork()) == -1)
        {
            close(pd[0]);
            close(pd[1]);
            return S_ERR;
        }
        else if (!r)
        {
            if (write(pd[1], str, strlen(str)) == -1)
                perror(" Erro ao escrever no pipe \n");
            close(pd[1]);
            _exit(1);
        }
        close(pd[1]);
        wait(NULL);
        ss = pd[0];
    }
    return ss;
}

char *s_script(s_pipe in)
{
    char *send = NULL, *huge[20000];
    Buffer_t stc;
    int n, i = 0;
    long sum = 0;

    if (in != S_NONE && in != S_ERR)
    { /* se está tudo direito com o pipe. */
        stc = mkBuffer(in, 4096);
        //printf("I'm here atleast\n");
        while ((n = (int)readln(stc, (void *)&send)) > 0)
        {
            //printf(" n :: %d\n",n);
            send[n - 1] = '\0';
            //printf("-> %s \n",send);
            huge[i++] = send; /* adiciona a uma lista */
            sum += n;         /*conta os todos os bytes lidos */
            if (i == 20000)
                perror("Capacity execeded _ ::script:: _\n");
        }
        //printf(" CataList whent fine \n");
        huge[i] = NULL;
        send = malloc(sizeof(char) * (sum + 1));
        *send = '\0';
        i = 0;
        while (huge[i])
        {
            strcat(send, huge[i]);
            strcat(send, "\n");
            free(huge[i]);
            i++;
        }
        unmkBuffer(stc);
    }
    return send;
}

/*  Desc:
    . escreve o conteudo do descritor recebido num ficheiro e para um pipe 
    . devolve o pd[0] desse pipe (fecha pd[1]). */
/* warn: 
    . não fecha o pEnd 
    . gera um filho e não espera. */

s_pipe s_fork(s_pipe fin, int fileds)
{
    Buffer_t stcm;
    int pd[2], n, r, erro = 0;
    char *text;
    s_pipe ss;

    if (pipe(pd) == -1)
    {
        ss = S_ERR;
    }
    else
    {
        if ((r = fork()) == -1)
        {
            close(pd[0]);
            close(pd[1]);
            return S_ERR;
        }
        else if (!r)
        {
            stcm = mkBuffer(fin, 1024);
            close(pd[0]);
            while ((n = readln(stcm, (void *)&text)) > 0)
            { //escrever para o pipe interno assim como ficheiro
                erro = MONAD(erro, write(pd[1], text, n));
                erro = MONAD(erro, write(fileds, text, n));

                free(text);
            }
            if (erro == -1)
                perror("Erro ao escrever para ficheiro\n");
            unmkBuffer(stcm);
            close(pd[1]);
            _exit(1);
        }

        close(pd[1]);
        wait(NULL);
        ss = pd[0];
    }

    return ss;
}
