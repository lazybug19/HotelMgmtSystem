#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define MAX_CUSTOMERS 5
#define MAX_ORDER 100
#define MAX_ORDER_CUS 10
#define MAX_TABLES 10

int customerCount = 0;
int tableNumber = 0;

void displayMenu()
{
    FILE *file = fopen("menu.txt", "r");
    if (file == NULL)
    {
        perror("Could not open menu.txt");
        return;
    }

    char line[256];

    printf("\n\t\t ***MENU*** \t\t\n");



    while (fgets(line, sizeof(line), file))
    {
        printf("%s", line);
        fflush(stdout);

    }

    printf("\n");
  
    if (ferror(file))
    {
        perror("Error reading menu.txt");
    }

    if (fclose(file) != 0)
    {
        perror("Error closing menu.txt");
    }
}



int main()
{
    //shared memory with waiter process

    printf("Enter Table Number: ");
    scanf("%d", &tableNumber);

    if (tableNumber == -1)
    {   
        printf("\nClosing Table\n");
        return 0;
    }

    else if (tableNumber < 1 || tableNumber > MAX_TABLES)
    {
        printf("\nInvalid Table Number\n");
        return -1;
    }

    key_t key_table = ftok("table.c", tableNumber);

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

    //intializing the shared memory to 0

    shmptr_table[0] = tableNumber; 

    for (int i = 1; i < MAX_ORDER+3; i++)
    {
        shmptr_table[i] = 0;
    }

    key_t customer_key = ftok("table.c", 'A'+tableNumber-1);
    int shmid_customer;
    int *shmptr_customer;

    
    if (customer_key == -1)
    {
        perror("Error in ftok of customer");
        return 1;
    }

    shmid_customer = shmget(customer_key, 2*sizeof(int), IPC_CREAT | 0666);

    if (shmid_customer == -1)
    {
        perror("Error in creating shared memory segment of customer");
        return 1;
    }

    shmptr_customer = shmat(shmid_customer, NULL, 0);

    if (shmptr_customer == (void *)-1)
    {
        perror("Error in attaching shared memory segment to customer");
        return 1;
    }


    while (1)
    {
        printf("Enter  Number  of  Customers  at  Table  (maximum  no.  of customers can be 5): ");
        scanf("%d", &customerCount);
        



        if (customerCount == -1)
        {   
            shmptr_table[1] = -2;

            sleep(1);

            if(shmdt(shmptr_table) == -1)
            {
                perror("Error in detaching shared memory segment of table and waiter");
                return(1);
            }

            if (shmctl(shmid_table, IPC_RMID, NULL) == -1)
            {
                perror("Error in deleting shared memory segment of Table and waiter");
                return(1);
            }


            if(shmdt(shmptr_customer) == -1)
            {
                perror("Error in detaching shared memory segment of customer");
                return(1);
            }

            if(shmctl(shmid_customer, IPC_RMID, NULL) == -1)
            {
                perror("Error in deleting shared memory segment of customer");
                return(1);
            }

            printf("Closing Table\n");
            return 0;
        }

        else

        if (customerCount < 1 || customerCount > MAX_CUSTOMERS)
        {
            printf("\nInvalid Customer Count\n");
            continue; // Skip the rest of the loop and start over
        }

        displayMenu();

        int fd[MAX_CUSTOMERS][2];
        pid_t child_pids[MAX_CUSTOMERS]; // Array to store child process IDs

        shmptr_customer[0] = 0;
        shmptr_customer[1] = 0;

        shmptr_table[0] = tableNumber;
        shmptr_table[1] = 1;
        shmptr_table[2] = 0;


        ///////

        for (int i = 0; i < customerCount; i++)
        {
            pipe(fd[i]);

            child_pids[i] = fork(); // Store child process ID
            if (child_pids[i] == 0)
            {
                close(fd[i][0]);

                int c_id= i+1;
                int fd_write = fd[i][1];

                while(shmptr_customer[1]!=1)
                {
                    if (shmptr_customer[0]==i) {


                        printf("\nCustomer %d  is ordering - \n", c_id); //TODO: Display Table id

                        int choices[MAX_ORDER_CUS] = {0};
                        int count = 0;

                        int choice ;
                        while (1)
                        {   
                            printf("\nEnter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: \t");
                            scanf("%d",&choice);
                            if (choice == -1)
                            {

                                break;
                            }

                            choices[count++] = choice;
                        }

                        shmptr_customer[0] = shmptr_customer[0]+1;

                        printf("\nCustomer %d completed ordering \n", i+1);

                        write(fd_write, &c_id, sizeof(int));
                        write(fd_write, &count, sizeof(count));
                        write(fd_write, choices, count * sizeof(int));
                    }

                    else
                    {
                        sleep(1);
                    }
                }


                //detach the shared memory

                if(shmdt(shmptr_customer) == -1)
                {
                    perror("Error in detaching shared memory segment of customer");
                    return(1);
                }

                close(fd_write);
                return 0;
            }
            else
            {
                close(fd[i][1]);
            }
        }

        int numbers[MAX_CUSTOMERS][MAX_ORDER] = {0};
        int counts[MAX_CUSTOMERS] = {0};

        while(shmptr_customer[1]!=1)
        {
            for (int i = 0; i < customerCount; i++)
            {
                
                // Now read from the pipe
                int customer_id, count;
                read(fd[i][0], &customer_id, sizeof(customer_id));
                read(fd[i][0], &count, sizeof(count));
                read(fd[i][0], numbers[customer_id - 1], count * sizeof(int));
                counts[customer_id - 1] = count;


            }



            //writing to the waiter processor
            shmptr_table[0] = tableNumber;
            shmptr_table[1] = 1;
            shmptr_table[2] = 0;

            //printf("\nWriting to the waiter processor\n");
            int total_orders = 0;
            for (int i = 0; i < customerCount; i++)
            {
                for (int j = 0; j < counts[i]; j++)
                {   
                    shmptr_table[total_orders+3] = numbers[i][j];
                    total_orders++;
                }
            }

            shmptr_table[total_orders+3] = -1;


            shmptr_table[1] = 2;

            while(shmptr_table[1] == 2)
            {
                sleep(1);
            }

            if (shmptr_table[1] == -1)
            {
                printf("\nError in orders , retaking order\n");
                for (int i = 1; i < MAX_ORDER+3; i++)
                {
                    shmptr_table[i] = 0;
                }
                shmptr_table[1] = 0;
                shmptr_customer[0] = 0;
                shmptr_customer[1] = 0;
                continue;
                
            }

            else{


            printf("\nThe total bill amount is %d INR\n", shmptr_table[2]);

            shmptr_table[1] = 4;

            shmptr_customer[1] = 1;

            for (int i = 0; i < customerCount; i++)
            {
                int status;
                waitpid(child_pids[i], &status, 0); // Wait for this child to finish
                close(fd[i][0]);

            }



            while(shmptr_table[1] != 0)
            {
                sleep(1);

            }

        }


    }

    }

    
    return 0;
}
