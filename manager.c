#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/wait.h>

//Numero de tarefas que podem ser lidas
#define N 20

int main() {

    // usa como delimitadores o espaco em branco e o fim da linha
    char *token, *delim = " \n";

    int pid[5], i = 0, j = 0, curtas = 0, medias = 0, longas = 0, estado, total_tarefas, tarefas_executadas = 0;
    int idfila_client_manager, idfila_manager_client, idfila_manager_worker, idfila_worker_manager, idfila_fim_trabalho;

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
    double tempo_exec;
    time_t tempo_inicio, tempo_fim;
    
    /* obtem fila de mensagens do CLIENT para manager*/
    if ((idfila_client_manager = msgget(4841, 0x1B6)) < 0) {
        printf("erro na obtencao da fila\n");
    }
    
    /* obtem fila de mensagens do MANAGER para CLIENT*/
    if ((idfila_manager_client = msgget(4840, 0x1B6)) < 0) {
        printf("erro na obtencao da fila\n");
    }
    
    // recebe mensagem do cliente
    if(msgrcv(idfila_client_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, 0)< 0){
        printf("Erro ao receber mensagem do cliente"); 
    }
    
    //Pega tempo do inicio da execucao
    tempo_inicio = time(NULL);

    //iterando a mensagem linha por linha	
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
    
    total_tarefas = j;
    printf("TOTAL DE TAREFAS (COMECA DO 0) -> %d\n", total_tarefas);
    
    //cria 3 arrays separando as tarefas pelo tipo
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
        } 
    }   
    
    // cria processos filho
    for (i = 0; i < 4; i++) {
        if ((pid[i] = fork()) < 0) {
            printf("erro na criacao de processos worker\n");
        }
        //processo filho
        if (pid[i] == 0) {
            if (execl("worker", "worker", (char *) 0) < 0){
                printf("erro no execl do worker = %d\n", errno);  
            }
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
    
    // envia tarefas curtas para workers
    i=0;    
    while(i < curtas) {  
        if(msgrcv(idfila_worker_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, IPC_NOWAIT)>= 0){
            if(mensagem_rec.pid == 999){
                tarefas_executadas++;
                printf("Tarefas executadas -> %d\n", tarefas_executadas);
            }
            mensagem_env.pid = taskscurtas[i].tipo;
            strcpy(mensagem_env.msg, taskscurtas[i].executavel);
            msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0);
            i++;
        } 
    }
    
    // envia tarefas medias para workers
    i=0;    
    while(i < medias) {          
        if(msgrcv(idfila_worker_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, IPC_NOWAIT)>= 0){
            if(mensagem_rec.pid == 999){
                tarefas_executadas++;
                printf("Tarefas executadas -> %d\n", tarefas_executadas);
            }
            mensagem_env.pid = tasksmedias[i].tipo;
            strcpy(mensagem_env.msg, tasksmedias[i].executavel);
            msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0);
            i++;
        } 
    }
    
    // envia tarefas longas para workers
    i=0;
    while(i < longas) {          
        if(msgrcv(idfila_worker_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, IPC_NOWAIT)>= 0){
            //worker informa que executou tarefa
            if(mensagem_rec.pid == 999){
                tarefas_executadas++;                
                printf("Tarefas executadas -> %d\n", tarefas_executadas);
            }
            mensagem_env.pid = taskslongas[i].tipo;
            strcpy(mensagem_env.msg, taskslongas[i].executavel);
            msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0);
            i++;
        } 
    }
    
    i = 0;
    while(i < 4){
        if(msgrcv(idfila_worker_manager, &mensagem_rec, sizeof (mensagem_rec) - sizeof (long), 0, IPC_NOWAIT)>= 0){
            if(mensagem_rec.pid == 999){
                tarefas_executadas++;
                printf("Tarefas executadas -> %d\n", tarefas_executadas);
            }
        }
        
        if(total_tarefas == tarefas_executadas){  
            //sinaliza para workers que trabalho acabou
            //enviando uma mensagem para cada worker
            mensagem_env.pid = 999;
            strcpy(mensagem_env.msg, "fim do trabalho");
            if(msgsnd(idfila_manager_worker, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
                printf("Erro ao enviar mensagem para Worker");
            } 
            i++;
        }
        
    }
       
    //espera pelo exit dos processos filho
    wait(&estado);
    wait(&estado);
    wait(&estado);
    wait(&estado);
    
    tempo_fim = time(NULL);
    tempo_exec = difftime(tempo_fim, tempo_inicio);
    
    // avisa para cliente fim da execucao e tempo de execucao
    mensagem_env.pid = 1;
    sprintf(mensagem_env.msg, "%f", tempo_exec);
    if(msgsnd(idfila_manager_client, &mensagem_env, sizeof (mensagem_env) - sizeof (long), 0) < 0){
        printf("Erro ao enviar mensagem para Client");
    } 
    
    printf("TAREFAS EXECUTADAS: %d\n", tarefas_executadas+1);
    exit(1);
    return (1);
}