#ifndef common_h
#define common_h

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

/* Caminho do pipe com nome de comunicação com o processo principal */
#define ADMIN_CALLS_PATH "ffadmin.session"

/* Macro para propagação de anomalias*/
#define MONAD(r, x) ((r == -1) ? r : x)

/* Macro para o cálculo de valores absolutos */
#define ABS(x) (x > 0) ? x : ((-1) * x)

/* Macro para escrever uma mensagem no descritor de erro*/
#define errorMessage(str) write(2, str, strlen(str))

/* Código que indica o erro de um processo filho */
#define WARNING_CODE 43

/* Codificação para os diferentes tipos de libertação de memória */
#define WIPE_MODE 1

#define KEEP_MODE 0

#endif