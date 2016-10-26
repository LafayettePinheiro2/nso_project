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
    double tempo_exec;
    char remove_fila[80];
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
    
    /* cria fila para receber msgs do manager*/
    if ((idfila_manager_client = msgget(4840, IPC_CREAT | 0x1B6)) < 0) {
        printf("erro na criacao da fila\n");
        exit(1);
    }

    // Envia mensagem com tarefas do arquivo para manager
    mensagem_env.pid = getpid();
    strcpy(mensagem_env.msg, buffer);
    if(msgsnd(idfila_client_manager, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
        printf("Erro ao enviar mensagem para Manager");
    }

    free(buffer);
    
    // Recebe mensagem do manager com tempo de execucao da aplicacao
    if(msgrcv(idfila_manager_client, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, 0)< 0){
        printf("Erro ao receber mensagem do manager"); 
    }

    sscanf(mensagem_rec.msg, "%lf", &tempo_exec);
    
    printf("Tempo de execucao da aplicacao: %.2f segundos", tempo_exec); 
    
    //remove filas de mensagem criadas pelo client
    sprintf(remove_fila, "ipcrm -q %d", idfila_client_manager);  
    system(remove_fila);
    sprintf(remove_fila, "ipcrm -q %d", idfila_manager_client);  
    system(remove_fila);
    return (1);
}