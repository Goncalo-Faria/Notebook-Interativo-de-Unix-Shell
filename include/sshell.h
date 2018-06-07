#ifndef sshell_h
#define sshell_h

/* Macros para os valores de anomalia do s_pipe */
#define S_NONE 0
#define S_ERR -1

/*
    Este módulo visa simplificar a execução de comandos na terminal.
*/

typedef int s_pipe;

/*
    Executa a sequência de comandos descriminada (mysystem) e devolve
        o output desse comando num descritor s_pipe.
*/
s_pipe s_open(s_pipe fin, char *command);

/*
    Esta função termina uma instância de um descritor s_pipe.
*/

void s_close(s_pipe s);

/*
    Esta função coloca a cadeia de caracters num descritor s_pipe e devolve-o
        como resultada da chamada desta função.
*/
s_pipe s_echo( char* str );

/*
    Esta função faz a transcrição do conteudo de um dado descritor s_pipe para 
        uma cadeia de caracters.
*/
char *s_script(s_pipe in);

/*
    Esta função escreve o conteudo de um descritor s_pipe num dado ficheiro devolvendo
        um descritor s_pipe no estado antes de ter sido executada a função.
*/

s_pipe s_fork(s_pipe fin, int fileds);

#endif