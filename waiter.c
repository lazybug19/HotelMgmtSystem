#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define MAX_ORDER 100
#define MAX_ITEMS 10

int calculate_bill(int *orderarr){
    int bill=0;

    FILE *file = fopen("menu.txt", "r");
    if (file == NULL) {
        perror("Error in reading menu.txt");
        return 1;
    }

    int prices[MAX_ITEMS];
    int itemNumber = 0;
    int price;

    while (fscanf(file, "%*d. %*[^0-9] %d INR ", &price) == 1) {
        prices[itemNumber] = price;
        itemNumber++;
    }

    fclose(file);

    for(int i=3;i<MAX_ORDER+3;i++){

        if((orderarr[i]>itemNumber || orderarr[i]<1  ) && orderarr[i]!=-1){
            return -1;
        }

        if (orderarr[i]==-1){
            
            return bill;
        }

        bill+=prices[orderarr[i]-1];
        
        
    }



    return 0;

}
int main()
{
    int waiter_id;
    printf("Enter Waiter ID: ");

    scanf("%d", &waiter_id);

    //shared memory for hotel manager

    key_t key_manager = ftok("hotelmanager.c", waiter_id);


    if (key_manager == -1){
        perror("Error in ftok of Waiter");
        return(1);
    }



    int shmid_manager;
    int *shmptr_manager;

    shmid_manager = shmget(key_manager, 3*sizeof(int), IPC_CREAT | 0666);

    if (shmid_manager == -1){
        perror("Error in creating shared memory segment of Waiter");
        return(1);
    }

    shmptr_manager = shmat(shmid_manager, NULL, 0);

    if (shmptr_manager == (void *)-1){
        perror("Error in attaching shared memory segment to Waiter");
        return(1);
    }

    shmptr_manager[0] = waiter_id;
    shmptr_manager[1] = 1;
    shmptr_manager[2] = -1;

    //shared memory for table

    key_t key_table = ftok("table.c", waiter_id);

    if (key_table == -1){
        perror("Error in ftok of Waiter");
        return(1);
    }

    int shmid_table;
    int *shmptr_table;


    shmid_table = shmget(key_table, (MAX_ORDER+3)*sizeof(int), IPC_CREAT | 0666);

    if (shmid_table == -1){
        perror("Error in creating shared memory segment of Waiter");
        return(1);
    }

    shmptr_table = shmat(shmid_table, NULL, 0);

    if (shmptr_table == (void *)-1){
        perror("Error in attaching shared memory segment to Waiter");
        return(1);
    }

    //shmptr_table[0] = table_id - wriitten by table
    //shmptr_table[1] = table_status - written by table & waiter
    //shmptr_table[2] = money spent by table - written by waiter
    //shmptr_table[3] to shmptr_table[MAX_ORDER+3] = orders by table

    printf("\nWaiter %d is ready to take orders\n", waiter_id);
    
    int table_id = shmptr_table[0];
    shmptr_table[2] = 0;

    while(1){

        
        int table_status = shmptr_table[1];
        
        //table status
        // 0 - unoccupied but dont terminate
        // 1 - occupied
        // 2 - orders written by table
        // 3 - bill written by waiter
        // 4 - bill read by table 
        // -1 - error in order table must get order again and overwrite to 1
        // -2 - terminate
        

        if (table_status == -2)
        {
            //deattach shared memory


            if(shmdt(shmptr_table)==-1)
            {
                perror("Error in detaching shared memory segment of  waiter with table");
                return(1);
            }

            shmptr_manager[1] = -1;

            if(shmdt(shmptr_manager)==-1)
            {
                perror("Error in detaching shared memory segment of waiter with manager");
                return(1);
            }

            printf("\nWaiter %d is terminating\n", waiter_id);

            exit(0);
        }

        else if (table_status>0){
        
        printf("\nWaiter %d is taking orders\n", waiter_id);

        while(shmptr_table[1]!=2){
            sleep(1);
        }

        int bill = calculate_bill(shmptr_table);
        

        if (bill == -1){

            shmptr_table[1] = -1;
            printf("\nError in order, retaking \n");
            sleep(1);

            //table must get order again
            }

        else {

            shmptr_table[1]= 3;
            shmptr_table[2] = bill;

            shmptr_manager[2] = bill;

            //bill calculated waiting for table and manager to read
            while ((shmptr_table[1]!=4)&&(shmptr_manager[2]!=-1)){
                
                sleep(1);

            }
            //bill read by table
            printf("\nBill Amount for Table %d : %d INR\n",waiter_id,bill);
            shmptr_table[1] = 0;
            shmptr_table[2] = 0;

        }

        }

        else{
            //table is unoccupied but dont terminate yet
            sleep(1);
        }

        
    }



}