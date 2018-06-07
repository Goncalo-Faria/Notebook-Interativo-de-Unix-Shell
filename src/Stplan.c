#include "common.h"
#include "Stplan.h"   /* header file */
#include "readb.h"    /* mkBuffer unmkBuffer readln */
#include "sshell.h"   /* s_open s_sript s_close s_echo*/
#include <sys/wait.h> /* wait*/
#include <stdio.h>    /* perror */
#include <ctype.h>    /* isdigit */
#include "GArray.h"   /* mkGArray unmkGArray getG setG lengthG */

/*-- Módulo de tipo abstrato de dados ----------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
/*-- Da autoria de Gonçalo Rui Alves Faria                                                    --*/
/*-- https://github.com/Goncalo-Faria                                                         --*/
/*----------------------------------------------------------------------------------------------*/

/*-- ( ) Declaração da Estrutura ---------------------------------------------------------------*/

typedef struct gather
{
    GArray list;
} * Structure;

typedef struct rec
{
    char s;
    /* 
       0 if text
       1 if comand
    */
    char *written;
    char *output;
} * G;

/*-- (1) Protótipos ----------------------------------------------------------------------------*/

/* Métodos públicos */
Structure mkStructure();
void unmkStructure(Structure a, int flag);
void addStCommand(Structure a, char *cmd, char *out);
void addStText(Structure a, char *text);
void deployStructure(Structure a, int file);
Structure deserializeStructure(int fd, int flag);
void runStExecute(Structure a);
void runStUpdate(Structure new, Structure backup);

/* Métodos privados */
static void fullDelete(void *a);
static int cmdStExecute(Structure fileStr, int indx);
static s_pipe reRoute(int indx, Structure fileStr, int num, char *cmd);
static int setStOut(Structure a, int i, char *val);
static char *getStOut(Structure a, int i, int indx);

/*-- (2) Implementação --------------------------------------------------------------------------*/

/*-- (2.1) Métodos privados ---------------------------------------------------------------------*/

static void fullDelete(void *a)
{ /*A caixa é copiada logo não é eliminada*/
    G x = (G)a;
    free(x->written);
    if (x->s)
        free(x->output);
}

static int cmdStExecute(Structure fileStr, int indx)
{
    s_pipe pout;
    int i;
    char number[20], *processOutStr, *text;
    G cur = getG(fileStr->list, indx);

    if (cur && cur->s)
    {                        /* É entrada de código */
        text = cur->written; /* comando */

        if (text[0] == '$')
        { /* ter a certeza que está tudo bem */
            if (text[1] == '|')
            { /*comando com redirect simples */
                pout = reRoute(indx, fileStr, 1, text);
            }
            else if (isdigit(text[1]))
            { /* comando com redirect arbitrário*/
                i = 0;
                while (isdigit(text[i + 1]))
                {
                    number[i] = text[i + 1];
                    i++;
                }
                number[i] = '\0';
                pout = reRoute(indx, fileStr, atoi(number), text);
            }
            else
            {
                pout = s_open(S_NONE, text + 1);
            }
            processOutStr = s_script(pout); /* passar o conteudo do pipe para string*/
            //printf(":::::%s\n:::::",processOutStr);
            s_close(pout); /* fechar pipe criado pelo comando*/
            setStOut(fileStr, indx, processOutStr);
        }
    }
    return 0;
}

static s_pipe reRoute(int indx, Structure fileStr, int num, char *cmd)
{
    char *processOutStr;
    s_pipe pout;

    if (lengthSt(fileStr) < num + 1)
    {
        /* Tentar fazer $| sem usar comando anteriormente */
        perror("erro -> | Routing  \n");
        _exit(-1);
    }
    //printf(" :::> %d <:::\n", num);
    processOutStr = getStOut(fileStr, (-1) * num, indx);
    if (cmd[1] == '|')
        cmd += 2;
    else
        cmd += 3;

    pout = s_open(s_echo(processOutStr), cmd + (num / 10));

    return pout;
}

/*-- (2.2) Métodos públicos ---------------------------------------------------------------------*/

/* Cria uma instância desta Estrutura */
Structure mkStructure()
{
    Structure x = (Structure)malloc(sizeof(struct gather));
    x->list = mkGArray(sizeof(struct rec));
    return x;
}

/* Destrói uma instância desta estrutura */
void unmkStructure(Structure a, int flag)
{
    /*flag == 1 apagar || flag == 0 não apagar*/

    if (flag == WIPE_MODE)
    {
        unmkGArray(a->list, fullDelete);
    }
    else /* KEEP_MODE*/
    {
        unmkGArray(a->list, NULL);
    }
    free(a);
}

/* Adiciona no fim da Estrutura uma entrada de comando */
void addStCommand(Structure a, char *cmd, char *out)
{
    struct rec ele;

    ele.s = 1;
    ele.written = cmd;
    ele.output = out;
    setG(a->list, lengthG(a->list), (void *)&ele);
}

/* Adiciona uma entrada de texto no final da Estrutura */
void addStText(Structure a, char *text)
{
    struct rec ele;

    ele.s = 0;
    ele.written = text;
    ele.output = NULL;
    setG(a->list, lengthG(a->list), (void *)&ele);
}

/* Devolve o output de uma dada entrada de código */
static char *getStOut(Structure a, int i, int indx)
{
    G x;
    int j, count;
    i = ABS(i);
    count = i;

    if (indx < lengthSt(a) && i <= indx)
    { /* tira da cauda */
        for (j = indx - 1; j >= 0; j--)
        {
            x = (G)getG(a->list, j);
            if (x && x->s)
            { /* Entrada de código */
                count--;
                if (!count)
                    return x->output;
            }
        }
        return NULL;
    }

    return NULL;
}

/* Adiciona a uma entrada de código o output deste */
static int setStOut(Structure a, int i, char *val)
{
    G x = (G)getG(a->list, i);

    if (x && x->s)
    { /* Entrada de código  */
        if (x->output)
            free(x->output);
        x->output = val;
        setG(a->list, i, x);
        return 1;
    }
    return 0;
}

/* Devolve o tamanho do Array que suporta a instância da Estrutura */
int lengthSt(Structure a)
{
    return lengthG(a->list);
}

/* Serializa a Estrutura e escreve no ficheiro indicado */
void deployStructure(Structure a, int file)
{
    int i, n, strs, r, erro = 0;
    G ele;

    if ((r = fork()) == -1)
    {
        return;
    }
    else if (!r)
    {
        n = lengthG(a->list);
        for (i = 0; i < n; i++)
        {
            ele = (G)getG(a->list, i);
            strs = strlen(ele->written);

            erro = MONAD(erro, write(file, ele->written, strs));
            erro = MONAD(erro, write(file, "\n", 1));
            if (ele->s)
            {
                erro = MONAD(erro, write(file, ">>>\n", 4));
                erro = MONAD(erro, write(file, ele->output, strlen(ele->output)));
                erro = MONAD(erro, write(file, "<<<\n", 4));
            }
        }
        if (erro == -1)
            perror("Escrita no ficheiro sem sucesso\n");

        close(file);
        _exit(0);
    }
    else
    {
        wait(NULL);
    }
}
/* De-serializa o ficheiro indicado criando no processo uma estrutura e devolvendo esta*/

Structure deserializeStructure(int fd, int flag)
{
    char *text, buf[30000];
    ssize_t n;
    Buffer_t stg;
    Structure fileStr;

    if (fd == -1)
    {
        perror("Algo de errado com o ficheiro de input\n ");
        _exit(-1);
    }

    fileStr = mkStructure();
    //printf(" \n\n\n\n\nmade with sucess\n");
    stg = mkBuffer(fd, 4096); // 4Kb

    while ((n = readln(stg, (void *)&text)) > 0)
    { /* enquanto é diferente de EOF out -1 */
        if (text[n - 1] == '\n')
            text[n - 1] = '\0';

        if (text[0] == '$')
        { /*comando*/
            addStCommand(fileStr, text, NULL);
        }
        else if (!strcmp(text, ">>>"))
        {
            *buf = '\0';
            while ((n = readln(stg, (void *)&text)) > 0 && !strstr(text, "<<<"))
            {
                if (flag)
                    strcat(buf, text);
            }

            if (flag)
            {
                text = strcpy(malloc(sizeof(char) * (strlen(buf) + 1)), buf);
                setStOut(fileStr, lengthSt(fileStr) - 1, text);
            }
        }
        else
        {
            addStText(fileStr, text);
        }
    }
    unmkBuffer(stg);
    close(fd);
    return fileStr;
}

/* Executa todos os comandos da estrutura indicada cujo o output é null */
void runStExecute(Structure a)
{
    int i, n;
    G cur;

    n = lengthSt(a);
    for (i = 0; i < n; i++)
    {
        cur = (G)getG(a->list, i);
        if (cur->s && !cur->output)
            cmdStExecute(a, i);
    }
}

/* Executa todos os comandos e caso o backup do estado contenha comandos em comum não executa este */
void runStUpdate(Structure new, Structure backup)
{
    int i, n, nbackup;
    G curNew, curBackup;
    char *cpy, *orig;

    n = lengthSt(new);
    nbackup = lengthSt(backup);
    n = (n < nbackup) ? n : nbackup;

    for (i = 0; i < n; i++)
    {
        curNew = (G)getG(new->list, i);
        curBackup = (G)getG(backup->list, i);

        if (curNew->s && !strcmp(curNew->written, curBackup->written))
        {
            orig = curBackup->output;
            cpy = malloc(sizeof(char) * (strlen(orig) + 1));
            strcpy(cpy, orig);
            setStOut(new, i, cpy);
        }
    }

    runStExecute(new);
}