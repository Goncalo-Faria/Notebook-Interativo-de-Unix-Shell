#ifndef stPlan_h
#define stPlan_h

/*
    Este módulo é a estrutura que suporta todo o ficheiro de Notebook.
*/
typedef struct gather * Structure;

#define parseUserStructure(fd) deserializeStructure(fd,0)
#define parseBackupStructure(fd) deserializeStructure(fd,1)

/* 
    Estes são os métodos construtores deste módulo.
 */

Structure mkStructure();
void unmkStructure(Structure a, int flag);

/* 
    Permitem adicionar Entradas no Notebook.
*/
void addStCommand(Structure a, char *cmd, char *out);
void addStText(Structure a, char *text);
 
/* 
    Estas são as únicas Operações do módulo que realizam Input e Output.
*/
void deployStructure(Structure a, int file);
Structure deserializeStructure(int fd, int flag );

/*
    Estes métodos analisam e processão as entradas do Notebook. 
*/
void runStExecute(Structure a);
void runStUpdate(Structure new, Structure backup);

/*
    Método que consulta o número de entras que o Notebook contêm.
*/
int lengthSt(Structure a);

#endif