/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


    
//    system("ipcrm --all=msg");
    
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
//      

//                waitpid(pid[j], &estado, WNOHANG);





//MANDANDO SINAL DE FIM DO TRABALHO PARA PROCESSOS FILHO
        
//    }
    
        for(i=0; i<4; i++){                
            printf("pids %d %d\n", i, pid[i]);
        }
    
        if(kill(pid[0], SIGUSR1) == -1) {
            printf("%d\n", errno);
        }
        if(kill(pid[1], SIGUSR1) == -1) {
            printf("%d\n", errno);
        }
        if(kill(pid[2], SIGUSR1) == -1) {
            printf("%d\n", errno);
        }
        if(kill(pid[3], SIGUSR1) == -1) {
            printf("%d\n", errno);
        }
        
        
        
        
        
        
        void funcao_sigusr1()
{
   printf("%d recebi O SINAL sigusr1\n", getpid());
   
   sleep(10);
   exit(1);
}
        
        
        
    
    signal(SIGUSR1, funcao_sigusr1);