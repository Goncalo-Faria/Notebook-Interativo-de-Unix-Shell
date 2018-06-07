
#ifndef syntaxt_h
#define syntaxt_h

/* Declaração do tipo de dados abstrato Syntax*/
typedef struct syn * Syntax;

/* Declaração do tipo de dados abstrato nó de Syntax*/
typedef struct no * G;

/* Tipo de dados para o tratamento de erros*/
typedef enum{
    THROW,
    CATCH
}EXCEPTION;

/* Tipo de dados para a função de processamento semântico de fórmulas atômicas */
typedef int (*Evaluator)(const char *);

/* Topo de dados para a função de processamento de Operadores */
typedef int (*Operator)(G, Syntax, EXCEPTION);

/* macro para simplificar o uso da função de criação de nodos */
#define SemanticAnalysis(av,str,sy) makeG(&av, str, 0, sy)

/* macro para simplificar o uso do cálculo semântico */
#define Evaluation(av,sy) evaluate(av,sy,CATCH)

int evaluate(G cur, Syntax sy, EXCEPTION var);

/**
 * Cria um novo Syntax. Recebe como parametros a função de cálculo de fórmulas atómicas e o tamanho
 *      do dicionário a ser criado.   
 */
Syntax makeSy(int n, Evaluator std);

/**
 *  Destrói o dicionário dado como argumento. 
 */
void destSy(Syntax sy);

/**
 *  Adiciona um dado Operador da Linguagem ao Syntax numa dada posição na hierarquia.
 */
void addSy(Syntax sy, int i, char *wd, Operator op);

/**
 *  Método de criação da árvore syntatica da cadeia de caracgters recebida, com base no
 *      syntax indicado como argumento.
 */

int makeG(G *cur, char *msg, int lvl, Syntax sy);
/**
 * Expande o ramo direito no nó recebido.
 */
G getRightG(G cur);

/**
 * Expande o ramo esquerdo no nó recebido.
 */
G getLeftG(G cur);

/**
 * Destrói todos os nós e subnós do recebido comn argumento.
 */
void destG(G cur);

/**
 *  Esta função devolve a primeira ocorrência de uma palavra 
 *      na cadeia de caracters recebida como argumento. 
 */
char *word(const char *j, int *flag);

#endif