#include "structers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Task1.c"
#include "Task2.c"
#include "task3.c"
#include "help_functions.c"

/*-------------------------------------------------------------------------*/
void display_menu() {
    printf("\n");
    printf("=============================================\n");
    printf("    SFSD TP - SOCIAL NETWORK MANAGEMENT\n");
    printf("=============================================\n");
    printf("\n");
    printf("MAIN MENU:\n");
    printf("1.  Initialize TOF File (Task 1 - Load 50%% of data)\n");
    printf("2.  Insert N Records (Task 2)\n");
    printf("3.  Search for Rating between Two Students (Task 2)\n");
    printf("4.  Update a Rating (Task 2)\n");
    printf("5.  Display All Friends of a Student (Task 2)\n");
    printf("6.  Reorganize Files (Task 3 - Merge TOF and LNOF)\n");
    printf("7.  Display New Tof File Contents(merged.txt)\n");
    printf("8.  Display any block with its address\n");
   
    printf("9.  Display the entire Task1.txt (first tof) file\n");
    printf("10. Display the entire overflow.txt (lnof ) file\n");
    printf("11. Display the entire merged.txt (last tof) file\n");

    printf("12.  Exit Program\n");

    printf("\n");
    printf("=============================================\n");
    printf("Enter your choice (1-10): ");
}

/*-------------------------------------------------------------------------*/
int main() {
    printf("SFSD TP - Social Network Friendship Ratings System\n");
    printf("==================================================\n");
    printf("\n");
    printf("Important Notes:\n");
    printf("1. Ensure 'full_data.txt' exists in the current directory\n");
    printf("2. Run Task 1 first to initialize the TOF file\n");
    printf("\n");
    
    int choice;
    int running = 1;
    int task1_completed = 0; // Flag to track if Task 1 has been completed
    long max_pos = 3061 , start_pos = 1532 ; 
    
    while (running) {
        display_menu();
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        
        // Clear input buffer
        while (getchar() != '\n');
        
        // Check if Task 1 needs to be run first for certain operations
        if (choice >= 2 && choice <= 10 && !task1_completed) {
            printf("\n=============================================\n");
            printf("WARNING: You must run Task 1 first!\n");
            printf("Please select option 1 to initialize the TOF file.\n");
            printf("=============================================\n\n");
            continue;
        }
        
        switch (choice) {
            case 1:
                if (Tof_loading("Data.txt") == 0) {
                    task1_completed = 1;
                    printf("\nTOF file initialized successfully! You can now use other functions.\n");
                } else {
                    printf("\nFailed to initialize TOF file. Please check 'Data.txt' exists.\n");
                }
                break;
                
            case 2:
                {
                    int N;
                    printf("How many records do you want to insert? ");
                    if (scanf("%d", &N) != 1) {
                        printf("Invalid input!\n");
                        while (getchar() != '\n');
                        break;
                    }
                    while (getchar() != '\n'); // Clear buffer
                    insert_N(N,&start_pos,max_pos) ; 

                    
                }
                break;
                
            case 3:
                {
                    int from, to;
                    printf("Enter student ID 1 (from): ");
                    if (scanf("%d", &from) != 1) {
                        printf("Invalid input for student 1!\n");
                        while (getchar() != '\n');
                        break;
                    }
                    printf("Enter student ID 2 (to): ");
                    if (scanf("%d", &to) != 1) {
                        printf("Invalid input for student 2!\n");
                        while (getchar() != '\n');
                        break;
                    }
                    while (getchar() != '\n'); // Clear buffer
                    search_rat(from, to);
                }
                break;
                
            case 4:
                {
                    int from, to;
                    long newrat;
                    printf("Enter student ID 1 (from): ");
                    if (scanf("%d", &from) != 1) {
                        printf("Invalid input for student 1!\n");
                        while (getchar() != '\n');
                        break;
                    }
                    printf("Enter student ID 2 (to): ");
                    if (scanf("%d", &to) != 1) {
                        printf("Invalid input for student 2!\n");
                        while (getchar() != '\n');
                        break;
                    }
                    printf("Enter new rating: ");
                    if (scanf("%ld", &newrat) != 1) {
                        printf("Invalid input for rating!\n");
                        while (getchar() != '\n');
                        break;
                    }
                    while (getchar() != '\n'); // Clear buffer
                    update_rec(from, to, newrat);
                }
                break;
                
            case 5:
                {
                    int from;
                    printf("Enter student ID: ");
                    if (scanf("%d", &from) != 1) {
                        printf("Invalid input!\n");
                        while (getchar() != '\n');
                        break;
                    }
                    while (getchar() != '\n'); // Clear buffer
                    display_all_friends(from);
                }
                break;
                
            case 6:
                reorg("Task1.txt", "overflow.txt","merged.txt");
                break;
                
            case 7:
                TOF_display("merged.txt");
                break;
                
            case 8:
                {
                    display_block_with_address();
                }
                break;
             case 9:
               //------display

TOF_display("Task1.txt");
              break;
              case 10:
               //------display
display_entire_LNOF("overflow.txt");

              break;
               
            case 11:
                printf("\nExiting program...\n");
                printf("Thank you for using the SFSD TP System!\n");
                running = 0;
                break;
                
            default:
                printf("Invalid choice! Please enter a number between 1 and 10.\n");
                break;
        }
        
        if (running && choice != 10) {
            printf("\nPress Enter to continue...");
            getchar();
        }
    }
    
    return 0;
}