#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>



int main()
{   
    // Create a shared memory segment
    key_t key = ftok("admin.c",'A');
    if (key == -1){
        perror("Error in ftok of Admin");
        return(1);
    }

    int shmid_admin_manager;
    int *shmptr_admin_manager;
    shmid_admin_manager = shmget(key, sizeof(int), IPC_CREAT | 0666);

    if (shmid_admin_manager == -1){
        perror("Error in creating shared memory segment of Admin");
        return(1);
    }

    shmptr_admin_manager = shmat(shmid_admin_manager, NULL, 0);
    if (shmptr_admin_manager == (void *)-1){
        perror("Error in attaching shared memory segment to Admin");
        return(1);
    }


    while(1){

        char c;
        printf("\nDo you want to close the hotel? Enter Y for Yes and N for No. \n");
        scanf(" %c", &c);
        if(c == 'Y'|| c == 'y'){
            printf("\nHotel is being closed. \n");
            break;
        }
        else if(c == 'N'|| c == 'n'){
            printf("\nHotel remains open. \n");
        }
        else{
            printf("Invalid input , please renter input. \n");
        }

    }
    // Initiate termination of the hotel
    *shmptr_admin_manager = -1;
    while (*shmptr_admin_manager != 1){
        sleep(1);
    }

    if(shmdt(shmptr_admin_manager) == -1){
        perror("Error in detaching shared memory segment of Admin");
        return(1);
    }

    if(shmctl(shmid_admin_manager, IPC_RMID, NULL) == -1){
        perror("Error in deleting shared memory segment of Admin");
        return(1);
    }
    
    
    
    printf("\nHotel is closed. \n");



    return 0;
}
