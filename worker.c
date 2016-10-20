#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    
    int pid, idfila_worker_manager, idfila_manager_worker, estado, i=0, livre=1;

    struct mensagem {
        long pid;
        char msg[300];
    };
    struct mensagem mensagem_env, mensagem_rec;
    char *buffer;    
    long length;
    
    
    /* obtem fila para enviar do manager -> workers */
    if ((idfila_manager_worker = msgget(4842, 0x1B6)) < 0) {
        printf("Erro na criacao da fila de msg do manager para workers\n");
    }
    
    /* obtem fila para enviar dos workers para manager */
    if ((idfila_worker_manager = msgget(4843, 0x1B6)) < 0) {
        printf("Erro na criacao da fila de msg dos workers para manager\n");
    }
    
    while(1){
        //manda mensagem para Manager pedindo trabalho
        //pede_trabalho();
//        if(livre==1){
            mensagem_env.pid = getpid();
            strcpy(mensagem_env.msg, "estou livre");
            msgsnd(idfila_worker_manager, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0);
//            if(msgsnd(idfila_worker_manager, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
//                printf("Erro ao enviar mensagem para Manager");
//            }
//        }

        if(msgrcv(idfila_manager_worker, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, 0)>= 0){
//            livre=0;
            
            //condicao de parada porque acabou o trabalho
            if(mensagem_rec.pid == 0){
                printf("\nFim da execucao do worker\n");
//                exit(1);
            }            
            
            printf("\n Worker recebe resposta manager: tipo: %d -> %s\n", mensagem_rec.pid,mensagem_rec.msg);
            if(pid = fork() < 0){
                printf("Erro na criacao de processo filho do worker");
            }
            if (pid == 0) {
                if (execl(mensagem_rec.msg, mensagem_rec.msg, "param1", "param2", (char *) 0) < 0){
                    printf("erro no execl do filho do worker = %d\n", errno);  
                }
            } 
            
            wait(&estado);
            
//            livre=1;
            
        }
    }    

    return (1);
}