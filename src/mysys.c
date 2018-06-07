#include "common.h"
#include "mysys.h"    /* header */
#include <sys/wait.h> /* wait */
#include <fcntl.h> /* open O_RDONLY */
#include "Parse.h"
/*-- Módulo Funcional --------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------*/
/*-- Da autoria de Gonçalo Rui Alves Faria                                                    --*/
/*-- https://github.com/Goncalo-Faria                                                         --*/
/*----------------------------------------------------------------------------------------------*/

/*-- (1) Protótipos ----------------------------------------------------------------------------*/
/* Métodos públicos */
int mysystem(const char *cmd);

/* Métodos privados */
static int core_system(const char *command);
static int fileasif(char **v, int n);
static int swapDescriptor(char **v, int n, int fd);
static int waitcode(void);

static Syntax setupSyntax();

static int pipeOp(G cur, Syntax sy, EXCEPTION);
static int parallelOp(G cur, Syntax dic, EXCEPTION);
static int conjunctionOp(G cur, Syntax dic, EXCEPTION);
static int sequenceOp(G cur, Syntax dic, EXCEPTION);
static int disjunctionOp(G cur, Syntax dic, EXCEPTION);

/*-- (2) Implementação --------------------------------------------------------------------------*/

/*-- (2.1) Métodos privados ---------------------------------------------------------------------*/

/**
 *  Esta função é uma subrotina da função principal a definir.
 *  Recebe uma string, no entanto, assume que se trata 
 *      apenas de um único comando e os seus respetivos argumentos.  
 */

static int core_system(const char *command)
{
    int count, point, i, fcode, kfl, flag;
    char *r, *v[10000];

    count = point = kfl = 0;
    do
    {
        r = word(command + count, &i);
        v[point++] = r;
        count += i;

        if (r && (!strcmp(r, "<") || strstr(r, ">")))
            kfl = 1;

        /* chega a copiar o NULL */
    } while (r);

    if (kfl)
    {
        if (((fcode = fork())) == -1)
        {
            errorMessage("Erro ao duplicar um processo dentro do core_system");
            return WARNING_CODE;
        }

        if (!fcode)
        {
            i = fileasif(v, point);
            _exit(i);
        }
    }
    else
    {
        if (((fcode = fork())) == -1)
        {
            errorMessage("Erro ao duplicar um processo dentro do core_system");
            return WARNING_CODE;
        }

        if (!fcode)
        {
            flag = execvp(v[0], v); // é suposto argv repetir o nome da função
            if (flag == -1)
                flag = WARNING_CODE;
            _exit(flag);
        }
    }

    fcode = waitcode();

    for (point = 0; v[point]; point++)
        free(v[point]);

    return fcode;
}

/**
 *  Esta função espera por um processo filho e descodifica a mensagem que
 *      este enviou
 */

static int waitcode(void)
{
    int flag;
    wait(&flag);
    return WEXITSTATUS(flag);
}

/**
 *  Esta função define a função Operador pipe '|'.
 */

static int pipeOp(G cur, Syntax sy, EXCEPTION var)
{
    int pd[2];
    int code1, code2, fcode;
    pipe(pd);

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro de pipeOp");
        return WARNING_CODE;
    }
    else if (!fcode)
    {
        dup2(pd[1], 1);
        close(pd[1]);
        code1 = evaluate(getLeftG(cur), sy, var);
        _exit(code1);
    }
    close(pd[1]);

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro de pipeOp");
        return WARNING_CODE;
    }
    else if (!fcode)
    {
        dup2(pd[0], 0);
        close(pd[0]);
        code2 = evaluate(getRightG(cur), sy, var);
        _exit(code2);
    }
    close(pd[0]);
    code1 = waitcode();
    code2 = waitcode();
    return ((code1 == WARNING_CODE) ? code1 : code2);
}

/**
 *  Esta função define a função Operador paralelo '&'.
 */

static int parallelOp(G cur, Syntax sy, EXCEPTION var)
{
    int code1, code2, fcode;

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro de pipeOp");
        return WARNING_CODE;
    }
    else if (!fcode)
    {
        code1 = evaluate(getLeftG(cur), sy, var);
        _exit(code1);
    }

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro de parallelOp");
        return WARNING_CODE;
    }
    else if (!fcode)
    {
        code2 = evaluate(getRightG(cur), sy, var);
        _exit(code2);
    }

    code1 = waitcode();
    code2 = waitcode();

    return ((code1 == WARNING_CODE) ? code1 : code2);
}

/**
 *  Esta função define a função Operador conjunção '&&'.
 */

static int conjunctionOp(G cur, Syntax sy, EXCEPTION var)
{
    int code1, code2, pd[2], fcode;

    pipe(pd);

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro de conjunctionOp");
        return WARNING_CODE;
    }
    else if (!fcode)
    {
        dup2(pd[1], 1);
        close(pd[1]);
        code1 = evaluate(getLeftG(cur), sy, THROW);
        _exit(code1);
    }

    code1 = waitcode();

    if (code1 != WARNING_CODE)
    {
        if (((fcode = fork())) == -1)
        {
            errorMessage("Erro ao duplicar um processo dentro de conjunctionOp");
            return WARNING_CODE;
        }
        else if (!fcode)
        {
            dup2(pd[1], 1);
            close(pd[1]);
            code2 = evaluate(getRightG(cur), sy, THROW);
            _exit(code2);
        }
    }
    close(pd[1]);

    code2 = waitcode();

    if ((code1 != WARNING_CODE) && (code2 != WARNING_CODE))
    {
        if (((fcode = fork())) == -1)
        {
            errorMessage("Erro ao duplicar um processo dentro de conjunctionOp");
            return WARNING_CODE;
        }
        else if (!fcode)
        {
            dup2(pd[0], 0);
            close(pd[0]);
            code1 = execlp("cat", "cat", NULL);
            if (code1 == -1)
                code1 = WARNING_CODE;
            _exit(code1);
        }
        close(pd[0]);
        return waitcode();
    }
    close(pd[0]);
    return WARNING_CODE;
}

/**
 *  Esta função define a função Operador sequencial ';'.
 */

static int sequenceOp(G cur, Syntax sy, EXCEPTION var)
{
    int code1, code2 = 0, fcode;

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro de sequenceOp");
        return WARNING_CODE;
    }
    else if (!fcode)
    {
        code1 = evaluate(getLeftG(cur), sy, var);

        _exit(code1);
    }
    code1 = waitcode();

    if (code1 != WARNING_CODE)
    {
        if (((fcode = fork())) == -1)
        {
            errorMessage("Erro ao duplicar um processo dentro de sequenceOp");
            return WARNING_CODE;
        }
        else if (!fcode)
        {
            code2 = evaluate(getRightG(cur), sy, var);
            _exit(code2);
        }
        code2 = waitcode();
    }
    return ((code1 == WARNING_CODE) ? code1 : code2);
}

/**
 *  Esta função define a função Operador disjunçãp '||'.
 */

static int disjunctionOp(G cur, Syntax sy, EXCEPTION var)
{
    int code1, fcode;

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro de disjunctionOp");
        return WARNING_CODE;
    }
    else if (!fcode)
    {
        code1 = evaluate(getLeftG(cur), sy, THROW);
        _exit(code1);
    }

    code1 = waitcode();

    if (code1 == WARNING_CODE)
    {
        if (((fcode = fork())) == -1)
        {
            errorMessage("Erro ao duplicar um processo dentro de disjunctionOp");
            return WARNING_CODE;
        }
        else if (!fcode)
        {
            code1 = evaluate(getRightG(cur), sy, var);
            _exit(code1);
        }
        code1 = waitcode();
    }

    return code1;
}
/**
 * Esta função define o syntax que a função de parse generalizado
 *  vai usar para obter o valor semântico da string que a função mysystem recebe. 
 */

static Syntax setupSyntax()
{
    Syntax sy = makeSy(5, core_system);

    addSy(sy, 0, ";", sequenceOp);
    addSy(sy, 1, "&", parallelOp);
    addSy(sy, 2, "&&", conjunctionOp);
    addSy(sy, 3, "|", pipeOp);
    addSy(sy, 4, "||", disjunctionOp);
    return sy;
}

/**
 *  Esta função processa os operadores < , >, n> e >>  da unix shell.
 */

static int fileasif(char **v, int n)
{
    int i, fcode, fd, outp, inp, comp, num, len, fl;
    char *file, *sign, buffer[40];

    fl = fcode = outp = inp = 0;
    for (i = 0; v[i]; i++)
    {

        inp = !strcmp(v[i], "<");
        outp = !strcmp(v[i], ">");
        comp = (strlen(v[i]) > 1 && strstr(v[i], ">"));

        if ((inp || outp || comp) && (i + 1 < n))
        {
            file = v[i + 1];
            sign = v[i];
            v[i] = NULL;
            fl = 1;
            break;
        }
    }
    if (inp)
    {
        fd = open(file, O_RDONLY);
        fcode = swapDescriptor(v, 0, fd);
        close(fd);
    }
    else if (strcmp(sign, ">>"))
    {

        fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0642);
        if (outp)
        {
            fcode = swapDescriptor(v, 1, fd);
        }
        else
        {
            len = strlen(v[i]) - 1; // tirar o numero de n de n>
            memcpy(buffer, v[i], len);
            buffer[len - 1] = '\0';
            num = atoi(buffer);

            fcode = swapDescriptor(v, num, fd);
        }
        close(fd);
    }
    else
    {
        fd = open(file, O_CREAT | O_WRONLY | O_APPEND, 0642);
        fcode = swapDescriptor(v, 1, fd);
        close(fd);
    }

    if (fl)
        v[i] = sign;

    return fcode;
}

/**
 *  Esta função troca os descritores ou de output ou de input de um dado comando.  
 */

static int swapDescriptor(char **v, int n, int fd)
{

    int fcode, rcode;

    if (fd == -1)
        return WARNING_CODE;

    if (((fcode = fork())) == -1)
    {
        errorMessage("Erro ao duplicar um processo dentro do fileasifoutput\n");
        return WARNING_CODE;
    }

    if (!fcode)
    {
        dup2(fd, n);
        close(fd);

        rcode = execvp(v[0], v);
        if (rcode == -1)
            rcode = WARNING_CODE;
        _exit(rcode);
    }

    return 0;
}

/*-- (2.2) Métodos públicos ---------------------------------------------------------------------*/

/**
 *  Esta função recebe um comando de terminal e executa-o.  
 */
int mysystem(const char *cmd)
{
    char *str;
    G arborescence = NULL;
    Syntax sy = setupSyntax();

    str = malloc(sizeof(char) * (strlen(cmd) + 1));
    strcpy(str, cmd);

    SemanticAnalysis(arborescence, str, sy);
    Evaluation(arborescence, sy);

    destG(arborescence);
    destSy(sy);
    free(str);
    return 0;
}