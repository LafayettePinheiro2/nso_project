#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>       
#include <unistd.h>

int main() {
    int pid, idfila_client_manager, idfila_manager_client;

    struct mensagem {
        long pid;
        char msg[300];
    };
    struct mensagem mensagem_env, mensagem_rec;
    
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

    /* cria fila para mandar msgs para manager*/
    if ((idfila_client_manager = msgget(4841, IPC_CREAT | 0x1B6)) < 0) {
        printf("erro na criacao da fila\n");
        exit(1);
    }
    
    /* cria fila para receber msgs de manager*/
    if ((idfila_manager_client = msgget(4840, IPC_CREAT | 0x1B6)) < 0) {
        printf("erro na criacao da fila\n");
        exit(1);
    }

    mensagem_env.pid = getpid();
    strcpy(mensagem_env.msg, buffer);
    if(msgsnd(idfila_client_manager, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
        printf("Erro ao enviar mensagem para Manager");
        exit(1);
    }

    free(buffer);
    
//     recebe mensagem do manager com tempo de execucao da aplicacao
    if(msgrcv(idfila_manager_client, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, 0)< 0){
        printf("Erro ao receber mensagem do manager"); 
    }
    
    printf("Tempo de execucao da aplicacao: %s", mensagem_rec.msg);
    
    system("ipcrm --all=msg");

    exit(1);
    return (1);
}