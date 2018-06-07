#include <stdio.h>
#include "mysys.h"
#include <fcntl.h>
#include "common.h"
#include "Stplan.h"
#include <signal.h>
#include <sys/stat.h>
/*-- Módulo IO  --------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
/*-- Da autoria de Gonçalo Rui Alves Faria                                                    --*/
/*-- https://github.com/Goncalo-Faria                                                         --*/
/*----------------------------------------------------------------------------------------------*/

/*-- (1) Protótipos ----------------------------------------------------------------------------*/
static int generaluse(char *fdes, Structure uSt);
static int usebackup(char *fdes, Structure uSt, Structure bcSt);
static void safe_quit(int x);

/*-- (2) Implementação --------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    Structure uSt, bSt;
    int ufd, bfd, rfl;
    pid_t pid, tpid;
    char buffer[4000], *cur;
    if (argc > 1)
    {
        signal(SIGINT, safe_quit);
        pid = getpid();
        mkfifo("ffadmin.session", 0660);

        if ((tpid = fork()) == -1)
        {
            errorMessage(" Erro ao gerar processos\n");
        }
        else if (!tpid)
        {
            cur = buffer;
            rfl = open("ffadmin.session", O_RDONLY);

            read(rfl, cur, 1); /*Entra em deadlock até que algúem o abra*/
            
            kill(pid, SIGINT);

            _exit(0);
        }
        ufd = open(argv[1], O_RDONLY); //-->4

        if (ufd == -1)
        {
            errorMessage("Erro no ficheiro proporcionado pelo utilizador \n");
            kill(tpid, SIGKILL);
            unlink("ffadmin.session");
            return -1;
        }

        uSt = parseUserStructure(ufd); /*parse do ficheiro*/ //-->2
        close(ufd);                                          //<--4

        sprintf(buffer, "%s.session", argv[1]);
        bfd = open(buffer, O_RDONLY);

        if ((argc > 2 && !strcmp(argv[2], "-p++")) || (bfd != -1))
        {
            if ((bfd != -1))
            {
                bSt = parseBackupStructure(bfd);
                close(bfd);
            }
            else
            {
                bSt = NULL;
            }
            usebackup(argv[1], uSt, bSt);
        }
        else
        {
            generaluse(argv[1], uSt);
        }
        //1<--
        unmkStructure(uSt, WIPE_MODE); //2<--

        kill(tpid, SIGKILL);

        unlink("ffadmin.session");
    }else{
        errorMessage(" Nao indicou o nome de nenhum ficheiro \n");
    }

    return 1;
}

/**
 * Esta é a função chamada sempre que o sinal SIGINT é chamado ao programa.
 */

static void safe_quit(int x)
{
    char msg[400] = "\nPrograma abortou com sucesso.\nAs alterações feitas no notebook foram revertidas.\n\n";
    if (!unlink("ffadmin.session"))
    {
        write(1, msg, strlen(msg));
    }
    _exit(-1); // em vez de exit podia criar um ficheiro.
}

/**
 * Esta é a função que comunica com os métodos de criação de um notebook
 *      para o caso geral quando não é usado backup. 
 */

static int generaluse(char *fdes, Structure uSt)
{
    int ufd;

    runStExecute(uSt); //...  fill the structure with output

    ufd = open(fdes, O_WRONLY | O_CREAT | O_TRUNC, 0644); //-->1

    deployStructure(uSt, ufd); /* Escreve para ficheiro*/ //... write user answer
    close(ufd);
    return 0;
}

/**
 *  Esta é a função que comunica com os métodos de criação de um notebook
 *      quando é usado backup.
 */
static int usebackup(char *fdes, Structure uSt, Structure bcSt)
{
    char buffer[4000];
    int ufd, bfd;

    printf("Be advised.\nThe -p++ flag will increase significantly the performance of the notebook.\nHowever realise that if a comand is not modified it's output will be reused\nTherefore if you are modifying files of anykind we can't assure you will get the results you expect\nAdicionaly an extra file will be created in your directory\n");
    sprintf(buffer, "%s.session", fdes);

    if (bcSt)
    { /* ja foi criado previamente um ficheiro de backup*/

        runStUpdate(uSt, bcSt);         //...  fill the structure with output (w/ backup)
        unmkStructure(bcSt, WIPE_MODE); //<--7
    }
    else
    {
        runStExecute(uSt); //...  fill the structure with output
    }

    bfd = open(buffer, O_WRONLY | O_CREAT | O_TRUNC, 0400); //-->3
    deployStructure(uSt, bfd);                              //... write backup
    close(bfd);                                             //<--3

    ufd = open(fdes, O_WRONLY | O_CREAT | O_TRUNC, 0644); //-->1

    deployStructure(uSt, ufd); /* Escreve para ficheiro*/ //... write user answer
    close(ufd);                                           //1<--

    return 1;
}