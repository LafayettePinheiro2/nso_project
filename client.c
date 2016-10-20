#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int pid, idfila, estado;

    struct mensagem {
        long pid;
        char msg[300];
    };
    struct mensagem mensagem_env;
    FILE *arq;

    char *buffer, url[] = "tasks.txt";
    long length;


    //abre arquivo
    arq = fopen(url, "r");
    if (arq == NULL) {
        printf("Erro, nao foi possivel abrir o arquivo\n");
        exit(1);
    } else {
        //le arquivo
        fseek(arq, 0, SEEK_END);
        length = ftell(arq);
        fseek(arq, 0, SEEK_SET);
        buffer = malloc(length);
        if (buffer) {
            fread(buffer, 1, length, arq);
        }
    }

    fclose(arq);


    /* cria */
    if ((idfila = msgget(4841, IPC_CREAT | 0x1B6)) < 0) {
        printf("erro na criacao da fila\n");
        exit(1);
    }

    mensagem_env.pid = getpid();
    strcpy(mensagem_env.msg, buffer);
    if(msgsnd(idfila, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
        printf("Erro ao enviar mensagem para Manager");
        exit(1);
    }


    free(buffer);
//    system("ipcrm --all=msg");

    return (1);
}