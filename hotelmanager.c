#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define MAX_WAITER 10


int main()
{
    // Creating as many shared memory segments that have 3 integers , table_id, table_status, money_spent as there are tables
    int max_tables;
    printf("Enter the Total Number of Tables at the Hotel:\n");
    scanf("%d", &max_tables);


    if (max_tables < 1 || max_tables > MAX_WAITER){
        printf("Invalid input. Please enter a positive integer. \n");
        return(1);
    }

    //creating earnings.txt 
    FILE *file = fopen("earnings.txt", "w");
    if (file == NULL) {
        perror("Error in creating earnings.txt");
        return 1;
    }

    //writing "Earning from Table 1: 0 INR" till max_tables
    for(int i = 1; i <= max_tables; i++){
        fprintf(file, "Earning from Table %d: 0 INR\n", i);
    }

    fclose(file); 


    key_t key[max_tables];
    int shmid_waiter[max_tables];

    for(int i = 0; i < max_tables; i++){
        key[i] = ftok("hotelmanager.c", i+1);
        if (key[i] == -1){
            perror("Error in ftok of Hotel Manager");
            return(1);
        }

        shmid_waiter[i] = shmget(key[i], 3*sizeof(int), IPC_CREAT | 0666);
        if (shmid_waiter[i] == -1){
            perror("Error in creating shared memory segment of Hotel Manager");
            return(1);
        }
    }

//shmid_waiter[0] = waiter_id - written by waiter
//shmid_waiter[1] = waiter_status - written by table & waiter 
//shmid_waiter[2] = money spent by table - written by waiter


//waiter_status = -1 means table is empty and has been terminated
//waiter_status = 1 means table is being used

    
    int *shmptr_waiter[max_tables];

    for(int i = 0; i < max_tables; i++){
        shmptr_waiter[i] = shmat(shmid_waiter[i], NULL, 0);
        if (shmptr_waiter[i] == (void *)-1){
            perror("Error in attaching shared memory segment to Hotel Manager");
            return(1);
        }
    }

    // Initialize the shared memory segments
    for(int i = 0; i < max_tables; i++){
        shmptr_waiter[i][0] = i + 1;
        shmptr_waiter[i][1] = -1;
        shmptr_waiter[i][2] = 0;
    }

    //Create a shared memory segment for the admin manager

    key_t key_admin_manager = ftok("admin.c",'A');
    if (key_admin_manager == -1){
        perror("Error in ftok of Hotel Manager");
        return(1);
    }

    

    int shmid_admin_manager;
    int *shmptr_admin_manager;
    shmid_admin_manager = shmget(key_admin_manager, sizeof(int), IPC_CREAT | 0666);

    if (shmid_admin_manager == -1){
        perror("Error in creating shared memory segment of Hotel Manager");
        return(1);
    }

    shmptr_admin_manager = shmat(shmid_admin_manager, NULL, 0);

    if (shmptr_admin_manager == (void *)-1){
        perror("Error in attaching shared memory segment to Hotel Manager");
        return(1);
    }

    
    

    // keep checking the status of the tables and if a table status goes increases to a non negative value wait till it goes back to 0 and then print the money spent on the table and assign -1 to status again and keep a count of non negative status
    int used_tables=0;
    int earning_update[max_tables] ;
    for (int i = 0; i < max_tables; i++){
        earning_update[i] = 0;

    }


    while(1){
        used_tables = 0;
       

        for(int i = 0; i < max_tables; i++){

            if(shmptr_waiter[i][1] > 0){
                used_tables++;
                
            }

        }

        int updation = 0;

        for (int i = 0; i < max_tables; i++){
            if(shmptr_waiter[i][2]>0){
                earning_update[i] = shmptr_waiter[i][2];
                updation = 1;
                shmptr_waiter[i][2] = -1;
            }
        }

        if (updation == 1){

            

            FILE *file = fopen("earnings.txt", "r");
            if (file == NULL) {
                perror("Error in opening earnings.txt");
                return 1;
            }
            int current_earning[max_tables];
            for(int i = 0; i < max_tables; i++){
                fscanf(file, "Earning from Table %*d: %d INR\n", &current_earning[i]);
               
            }

            fclose(file);

            file = fopen("earnings.txt", "w");
            if (file == NULL) {
                perror("Error in opening earnings.txt");
                return 1;
            }

            for(int i = 0; i < max_tables; i++){
                if (earning_update[i] > 0){
                    current_earning[i] += earning_update[i];
                    earning_update[i] = 0;  // Reset the earning_update
                }
                int t_id=i+1;
                fprintf(file, "Earning from Table %d: %d INR\n", t_id, current_earning[i]);
                
            }



            fclose(file);

        }

 

        if (used_tables == 0 && *shmptr_admin_manager == -1){
            // Hotel is being closed and all shared memory segments are being detached and deleted
            
            int total_earning = 0;
            FILE *file = fopen("earnings.txt", "r");
            if (file == NULL) {
                perror("Error in opening earnings.txt");
                return 1;
            }

            for(int i = 0; i < max_tables; i++){
                int earning;
                fscanf(file, "Earning from Table %*d: %d INR\n", &earning);
                total_earning += earning;
            }

            fclose(file);

            FILE *file1 = fopen("earnings.txt", "a");

            printf("Total Earning of Hotel: %d INR\n", total_earning);
            fprintf(file1, "Total Earning of Hotel: %d INR\n", total_earning);

            int waiter_earning = total_earning*40/100;

            printf("Total Wages of Waiters: %d INR\n", waiter_earning);
            fprintf(file1, "Total Wages of Waiters: %d INR\n", waiter_earning);

            int profit = total_earning - waiter_earning;

            printf("Total Profit : %d INR\n", profit);
            fprintf(file1, "Total Profit : %d INR\n", profit);

            fclose(file1);

            for(int i = 0; i < max_tables; i++){

                //error checking
                if (shmdt(shmptr_waiter[i]) == -1){
                    perror("Error in detaching shared memory segment of Hotel Manager");
                    return(1);
                }

                if (shmctl(shmid_waiter[i], IPC_RMID, NULL) == -1){
                    perror("Error in deleting shared memory segment of Hotel Manager");
                    return(1);
                }

            
            }

            
            shmptr_admin_manager[0] = 1;
            
            if (shmdt(shmptr_admin_manager) == -1){
                perror("Error in detaching shared memory segment of Hotel Manager");
                return(1);
            }

            printf("\n\nThank you for visiting the Hotel! \n");
            return(1);
        }
        sleep(1);
    }

    return 0;
    }