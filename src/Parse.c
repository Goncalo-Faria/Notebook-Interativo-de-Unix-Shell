#include "Parse.h"
#include "common.h"
#include <stdio.h>
#include <fcntl.h>

/*-- Módulo da Estrutra de Parse Generalizada --------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
/*-- Da autoria de Gonçalo Rui Alves Faria                                                    --*/
/*-- https://github.com/Goncalo-Faria                                                         --*/
/*----------------------------------------------------------------------------------------------*/

/*-- ( ) Declaração da Estrutura ---------------------------------------------------------------*/

typedef struct no /***< nó da árvore de syntax */
{
    int type;         /**< nível na heararquia de operadores */
    char *command;    /**< fórmulas atómica */
    struct no *l, *r; /**< nós de nível inferiror na hierarquia */
} * G;

typedef struct syn /***< Estrutura que implementa o syntax e funções */
{                  /*de cálculo semântico de uma dada linguagem  */

    struct ent /***< Estrutura que representa um Operador */
    {
        char *verb; /**< Palavrar que representa um Operador */
        void *op;   /**< função para obter o valor semântico de um dado Operador */
    } * dic;
    Evaluator std;

    int max; /**< Tamanho do syntax */

} * Syntax;

/*-- (1) Protótipos ----------------------------------------------------------------------------*/

/* Métodos públicos */
char *word(const char *j, int *flag);
int makeG(G *cur, char *msg, int lvl, Syntax sy);
int evaluate(G cur, Syntax sy, EXCEPTION var);
Syntax makeSy(int n, Evaluator std);
void destSy(Syntax sy);
void addSy(Syntax sy, int i, char *wd, Operator op);
G getRightG(G cur);
G getLeftG(G cur);

/*-- (2) Implementação --------------------------------------------------------------------------*/

/*-- (2.1) Criação e manipulação de nós ---------------------------------------------------------*/

/**
 *  Método de criação da árvore syntatica da cadeia de caracgters recebida, com base no
 *      syntax indicado como argumento.
 */
int makeG(G *cur, char *msg, int lvl, Syntax sy)
{

    char *back, *front, *wd;
    int count = 0, j, b = 1;

    back = msg;

    if (lvl == sy->max)
    {
        *cur = (G)malloc(sizeof(struct no));
        (*cur)->type = lvl;
        //(*cur)->command = (char*)malloc(sizeof(char) * (strlen(msg) + 1));
        (*cur)->command = msg;
        (*cur)->l = (*cur)->r = NULL;

        return 0; //SS_EER
    }

    do
    {
        wd = word(msg + count, &j);

        if (wd)
        {
            count += j;
            if (!strcmp(sy->dic[lvl].verb, wd))
            {
                front = msg + count;
                front -= j;
                *front = '\0';
                front += j;
                *cur = (G)malloc(sizeof(struct no));
                (*cur)->type = lvl;
                (*cur)->l = (*cur)->r = NULL;

                makeG(&((*cur)->l), back, lvl + 1, sy);
                makeG(&((*cur)->r), front, lvl, sy);
                b = 0;
                break;
            }
            free(wd);
        }

    } while (wd);

    if (b)
    {
        makeG(cur, msg, lvl + 1, sy);
    }

    return 1;
}

/**
 * Destrói todos os nós e subnós do recebido comn argumento.
 */

void destG(G cur)
{
    if (cur)
    {
        destG(cur->l);
        destG(cur->r);
        free(cur);
    }
}

/**
 * Expande o ramo direito no nó recebido.
 */

G getRightG(G cur)
{
    return cur->r;
}

/**
 * Expande o ramo esquerdo no nó recebido.
 */

G getLeftG(G cur)
{
    return cur->l;
}

/*-- (2.2) Criação e manipulação de Dícionarios de Syntax ---------------------------------------*/

/**
 * Cria um novo Syntax. Recebe como parametros a função de cálculo de fórmulas atómicas e o tamanho
 *      do dicionário a ser criado.   
 */

Syntax makeSy(int n, Evaluator std)
{
    Syntax sy = (Syntax)malloc(sizeof(struct syn));
    sy->max = n;
    sy->dic = (struct ent *)malloc(sizeof(struct ent) * n);
    sy->std = std;
    return sy;
}

/**
 *  Destrói o dicionário dado como argumento. 
 */

void destSy(Syntax sy)
{
    int i;

    for (i = 0; i < sy->max; i++)
        free(sy->dic[i].verb);

    free(sy->dic);
    free(sy);
}

/**
 *  Adiciona um dado Operador da Linguagem ao Syntax numa dada posição na hierarquia.
 */

void addSy(Syntax sy, int i, char *wd, Operator op)
{
    if (i < sy->max)
    {
        sy->dic[i].verb = malloc(sizeof(char) * (strlen(wd) + 1));
        strcpy(sy->dic[i].verb, wd);
        sy->dic[i].op = (void *)op;
    }
}

/*-- (3) Função Dedicada ao cálculo semântico  --------------------------------------------------*/

/**
 *  Obtêm o valor semântico de uma dada árvore de Syntax.
 */

int evaluate(G cur, Syntax sy, EXCEPTION var)
{
    Operator actor;
    int flag, fd;

    if (cur->type < sy->max)
    {
        actor = (Operator)sy->dic[cur->type].op;
        flag = actor(cur, sy, var);
    }
    else
    {
        flag = sy->std(cur->command);
    }

    if (var == CATCH && flag == WARNING_CODE)
    {
        fd = open(ADMIN_CALLS_PATH, O_WRONLY);
        //write(fd, "WACK\n", 5);
        close(fd);
    }

    return flag;
}

/*-- (4) Funções auxiliares  --------------------------------------------------------------------*/

/**
 *  Esta função devolve a primeira ocorrência de uma palavra 
 *      na cadeia de caracters recebida como argumento. 
 */

char *word(const char *j, int *flag)
{
    char start[400];
    char *buffer, *tmp = NULL;
    int r = 0;
    *flag = 0;
    buffer = start;
    while (*j)
    {

        if (*j == ' ' && r)
            break;

        if (r)
        {
            *buffer = *j;
            j++;
            buffer++;
            *buffer = '\0';
            continue;
        }

        if (*j != ' ')
        {
            r = 1;
            continue;
        }

        j++;
        (*flag)++;
    }
    if (r)
    {
        tmp = (char *)malloc(sizeof(char) * (strlen(start)));
        *flag += (int)strlen(start);
        sprintf(tmp, "%s", start);
        return tmp;
    }
    *flag = -1;

    return NULL;
}