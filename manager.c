#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


//Numero de tarefas que podem ser lidas
#define N 20

int main() {

    char *token, *delim = " \n";
    // usa como delimitadores o espaco em branco e o fim da linha

    int pid[5], idfila, i = 0, j = 0, curtas = 0, medias = 0, longas = 0, estado;
    int idfila_manager_worker, idfila_worker_manager;

    struct mensagem {
        long pid;
        char msg[300];
    };

    struct mensagem mensagem_rec, mensagem_env;

    struct task {
        long tipo;
        char executavel[300];
    };

    struct task tasks[N], taskscurtas[N], tasksmedias[N], taskslongas[N];    
    
    // cria processos filho
    for (i = 0; i < 4; i++) {
        if ((pid[i] = fork()) < 0) {
            printf("erro na criacao de processos worker\n");
        }
        //processo filho
        if (pid[i] == 0) {
            if (execl("worker", "worker", "param1", "param2", (char *) 0) < 0){
                printf("erro no execl do worker = %d\n", errno);  
            }
        } 
    }    
    
    
    /* obtem fila de mensagens com CLIENTE*/
    if ((idfila = msgget(4841, 0x1B6)) < 0) {
        printf("erro na obtencao da fila\n");
    }
    
    // recebe mensagem do cliente
    if(msgrcv(idfila, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, 0)< 0){
        printf("Erro ao receber mensagem do cliente"); 
    }

    //ITERATE MESSAGE LINE BY LINE	
    token = strtok(mensagem_rec.msg, delim);

    while (token!= NULL) {
        j = (int) i / 2;

        if (isdigit(token[0])) {
            tasks[j].tipo = strtol(token, &token, 10);
        } else {
            strcpy(tasks[j].executavel, token);
        }

        //advance the token
        token = strtok(NULL, delim);
        
        i++;
    }

    for (i = 0; i <= j; i++) {
        //fila de tarefas curtas
        if(tasks[i].tipo == 1){
            taskscurtas[curtas].tipo = tasks[i].tipo;                
            strcpy(taskscurtas[curtas].executavel, tasks[i].executavel);
            curtas++;
        //fila de tarefa medias    
        } else if(tasks[i].tipo == 2){
            tasksmedias[medias].tipo = tasks[i].tipo;                
            strcpy(tasksmedias[medias].executavel, tasks[i].executavel);
            medias++;
        //fila de tarefa longas    
        } else if(tasks[i].tipo == 3){
            taskslongas[longas].tipo = tasks[i].tipo;                
            strcpy(taskslongas[longas].executavel, tasks[i].executavel);
            longas++;
            
        } else {
            //printf("O processo deve ser do tipo curto (1), medio (2) ou longo (3)");            
        }
    }
    
    
    /* cria fila para enviar do manager -> workers */
    if ((idfila_manager_worker = msgget(4842, IPC_CREAT | 0x1B6)) < 0) {
        printf("Erro na criacao da fila de msg do manager para workers\n");
    }
    
    /* cria fila para enviar dos workers para manager */
    if ((idfila_worker_manager = msgget(4843, IPC_CREAT | 0x1B6)) < 0) {
        printf("Erro na criacao da fila de msg dos workers para manager\n");
    }
    
    i=0;
    while(i < curtas) {          
        if(msgrcv(idfila_worker_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, IPC_NOWAIT)>= 0){
            printf("%ld ----> %s ", taskscurtas[i].tipo, taskscurtas[i].executavel);
            mensagem_env.pid = taskscurtas[i].tipo;
            strcpy(mensagem_env.msg, taskscurtas[i].executavel);
            msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0);
//            if(msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
//                printf("Erro ao enviar mensagem para Worker");
//            } 
            
            //zerar do array
            i++;
        } 
    }
    
    i=0;
    while(i < medias) {          
        if(msgrcv(idfila_worker_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, IPC_NOWAIT)>= 0){
             printf("%ld ----> %s ", tasksmedias[i].tipo, tasksmedias[i].executavel);
            mensagem_env.pid = tasksmedias[i].tipo;
            strcpy(mensagem_env.msg, tasksmedias[i].executavel);
            if(msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
//                printf("Erro ao enviar mensagem para Worker");
            } 
            
            //zerar do array
            i++;
        } 
    }
    
    i=0;
    while(i < longas) {          
        if(msgrcv(idfila_worker_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, IPC_NOWAIT)>= 0){
             printf("%ld ----> %s ", taskslongas[i].tipo, taskslongas[i].executavel);
            mensagem_env.pid = taskslongas[i].tipo;
            strcpy(mensagem_env.msg, taskslongas[i].executavel);
            if(msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
//                printf("Erro ao enviar mensagem para Worker");
            } 
            
            //zerar do array
            i++;
        } 
    }
    
    i=0;
    while(i < 20){   
        mensagem_env.pid = 0;
        strcpy(mensagem_env.msg, "-1");
        if(msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
//            printf("Erro ao enviar mensagem para Worker");
        } 
        
        waitpid(-1, &estado, WNOHANG);
        i++;
    }
    
//    sleep(10);
    
    system("ipcrm --all=msg");
    
//    printf("processos curtas\n");
//    for(i=0; i<curtas; i++){
//        printf("\ntipo: %d  --------------- ", taskscurtas[i].tipo);
//        printf("executavel: %s\n", taskscurtas[i].executavel);
//    }
//    
//    printf("\nprocessos medias\n");
//    for(i=0; i<medias; i++){
//        printf("\ntipo: %d  --------------- ", tasksmedias[i].tipo);
//        printf("executavel: %s\n", tasksmedias[i].executavel);
//    }
//    
//    printf("\nprocessos longas\n");
//    for(i=0; i<longas; i++){
//        printf("\ntipo: %d  --------------- ", taskslongas[i].tipo);
//        printf("executavel: %s\n", taskslongas[i].executavel);
//    }

    return (1);    
    
}

//void imprimevetor(struct task tasks[N], int size){
//    
//}