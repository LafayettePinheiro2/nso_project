#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>

int main() {
    
    int pid, idfila_fim_trabalho, idfila_worker_manager, idfila_manager_worker, estado, i=0, livre=1;

    struct mensagem {
        long pid;
        char msg[300];
    };
    struct mensagem mensagem_env, mensagem_rec;
    char *buffer;    
    long length;    
    
    /* obtem fila para receber do manager */
    if ((idfila_manager_worker = msgget(4842, 0x1B6)) < 0) {
        printf("Erro na obtencao da fila de msg do manager para workers\n");
    }
    
    /* obtem fila para enviar dos workers para manager */
    if ((idfila_worker_manager = msgget(4843, 0x1B6)) < 0) {
        printf("Erro na obtencao da fila de msg dos workers para manager\n");
    }
        
    i=0;
    while(1){
        
        //manda mensagem para Manager pedindo trabalho
        mensagem_env.pid = getpid();
        strcpy(mensagem_env.msg, "estou livre");
        if(msgsnd(idfila_worker_manager, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) >= 0) {
            if(msgrcv(idfila_manager_worker, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, 0)>= 0){
                if(mensagem_rec.pid == 999){
                    printf("%d recebe fim do trabalho --> %ld ---> %s\n", getpid(), mensagem_rec.pid, mensagem_rec.msg);
                    exit(1);
                }
                printf("\n Worker %ld vai executar --> tipo: %ld -> %s\n", mensagem_env.pid, mensagem_rec.pid, mensagem_rec.msg);                    

                if((pid = fork()) < 0){
                    printf("Erro na criacao de processo filho do worker");
                }
//                printf("valor fork worker %d\n", pid);
                if (pid == 0) {
                    if ((execl(mensagem_rec.msg, mensagem_rec.msg, (char *) 0)) < 0){
                        printf("erro no execl do filho do worker = %d\n", errno);  
                    } 
                } else {
                    // worker espera seu filho executar o trabalho e terminar
                    wait(&estado);  
                    mensagem_env.pid = 999;                        
                    strcpy(mensagem_env.msg, "realizei um trabalho");
                    msgsnd(idfila_worker_manager, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0);
                }                   
            }
        }
        i++;
    }  
    return(1);
}