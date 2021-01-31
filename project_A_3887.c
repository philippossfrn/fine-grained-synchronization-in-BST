// #include <stdio.h> 
// #include <stdlib.h> 
// #include <unistd.h> 
// #include <pthread.h> 
// #include "header.h"

#include "functions.h"

// The first argument is a pointer to thread_id which is set by this function.
// The second argument specifies attributes. If the value is NULL, then default attributes shall be used.
// The third argument is name of function to be executed for the thread to be created.
// The fourth argument is used to pass arguments to the function, myThreadFun.


int main(int argc, char *argv[]){ 

    global_root =  (treeNode *)malloc(sizeof(treeNode));

    if(argc == 2)
        prod_threads = atoi(argv[1]);
    else{
        printf("Please enter an argument for number of producers.\n");
        return 0;
    }

    //pthread_mutex_init(&root->lock, NULL);

    int thread_ids[prod_threads];
    int err;

    // Create threads
    pthread_t *prod_table;
    prod_table=(pthread_t *)malloc(prod_threads * sizeof(pthread_t ));
    // Initialize barrier --barrier is global
    pthread_barrier_init (&barrier, NULL,prod_threads);
    int i ;
    for (i= 0; i < prod_threads; i++){
        //printf("\nStarting this prod:%d\n",i);
        thread_ids[i] = i;
        err = pthread_create(&prod_table[i],NULL,&BSTInsert,&thread_ids[i]); //Default Attributes
        if(err != 0 ){
            printf("Failed to create thread with id: %d\n",i);
        }
    }

    for(i=0;i<prod_threads;i++){
        pthread_join(prod_table[i],NULL);
    }
    pthread_barrier_destroy(&barrier);

    // (global_root == NULL) ? printf("ROOT is null\n") : printf("TREE is:\n");
    // inorder(global_root); 
   // tree_check(global_root);
   
    // Thread 0 will do some checks on tree size and key's sum
    pthread_create(&prod_table[0],NULL,&tree_check,global_root); //Default Attributes
    pthread_join(prod_table[0],NULL);
    
    printf("-------------------------------- Insert in tree --------------------------------\n");
    int keysum_expected = pow(prod_threads, 2)*(prod_threads-1)*(prod_threads+1)/2;
    
    if( (tree_size != prod_threads*prod_threads) && (total_keysum != keysum_expected) ){
        printf(KRED"Oops...tree size check failed (expected: %d, found: %d)"RESET"\n",prod_threads*prod_threads,tree_size);
        printf(KRED"Oops...total keysum check failed (expected: %d, found: %d)"RESET"\n",keysum_expected,total_keysum);
        printf(KGRN"Program ended!!!"RESET"\n");
        exit(-42);
    }
    if(tree_size != prod_threads*prod_threads){
        printf(KRED"Oops...tree size check failed (expected: %d, found: %d)"RESET"\n",prod_threads*prod_threads,tree_size);
        printf(KGRN"Program ended!!!"RESET"\n");
        exit(-42);
    }
    if(total_keysum != keysum_expected){
        printf(KRED"Oops...total keysum check failed (expected: %d, found: %d)"RESET"\n",keysum_expected,total_keysum);
        printf(KGRN"Program ended!!!"RESET"\n");
        exit(-42);    
    }
    printf(KGRN"Total tree size check passed (expected: %d, found: %d)"RESET"\n",prod_threads*prod_threads,tree_size);
    printf(KGRN"Total tree keysum check passed (expected: %d, found: %d)"RESET"\n",keysum_expected,total_keysum);

    // Users
    
    int users = prod_threads;

    //printf("STIN MAIN: %d",n);
    
    //printf("STIN MAIN2: %d",sizeof total_queues / sizeof total_queues[0]);
    pthread_t *users_table;
    users_table=(pthread_t *)malloc(users * sizeof(pthread_t ));
    int users_ids[users];
    total_queues=(struct queue *)malloc((users/2)*sizeof(struct queue));
    pthread_barrier_init (&all_users_barrier, NULL,users);
    pthread_barrier_init (&except_controller_barrier, NULL,users-1);
    for (i = 0; i < users; i++){
        //printf("\nStarting this prod:%d\n",i);
        users_ids[i] = i;
        err = pthread_create(&users_table[i],NULL,&BSTSearch,&users_ids[i]); //Default Attributes
        if(err != 0 ){
            printf(KRED"Failed to create thread with id: %d"RESET"\n",i);
        }
    }

    for(i=0;i<users;i++){
        pthread_join(users_table[i],NULL);
    }
    pthread_barrier_destroy(&all_users_barrier);
    pthread_barrier_destroy(&except_controller_barrier);

    // Thread 0 will do some checks on queue size and key's sum


    // for (size_t i = 0; i < (users/2); i++){
    //     // enqueue( &total_queues[i],rand());
    //     printf("Printing %d queue------------------------\n",i);
    //     display(( &total_queues[i])->Head);
    // }

    printf("-------------------------------- Search & insert in queues --------------------------------\n");
    pthread_create(&users_table[0],NULL,&call_queue_check,global_root); //Default Attributes
    pthread_join(users_table[0],NULL);

    int managers = prod_threads;
    pthread_t *nanager_table;
    nanager_table=(pthread_t *)malloc(managers * sizeof(pthread_t ));
    int manager_ids[managers];

    doubly_list = (struct list*) malloc(sizeof(struct list));
    doubly_list->head  = (struct listNode*) malloc(sizeof(struct listNode));
    doubly_list->tail  = (struct listNode*) malloc(sizeof(struct listNode));

    struct listNode *tmp  = (struct listNode*) malloc(sizeof(struct listNode));
    tmp->next = NULL;
    tmp->songID = -1;

    doubly_list->head = doubly_list->tail = tmp;
    pthread_barrier_init (&last_barrier, NULL,(managers)/2);
    for (i = 0; i < managers; i++){
        //printf("\nStarting this prod:%d\n",i);
        manager_ids[i] = i;
        err = pthread_create(&nanager_table[i],NULL,&BSTDelete,&manager_ids[i]); //Default Attributes
        if(err != 0 ){
            printf(KRED"Failed to create thread with id: %d"RESET"\n",i);
        }
    }

    for(i=0;i<managers;i++){
        pthread_join(nanager_table[i],NULL);
    }
    pthread_barrier_destroy(&last_barrier);
    // for (size_t i = 0; i < (users/2); i++){
    //     // enqueue( &total_queues[i],rand());
    //     printf("Printing %d queue------------------------\n",i);
    //     display(( &total_queues[i])->Head);
    // }

    // inorder(global_root);
    printf("-------------------------------- Delete operations --------------------------------\n");
    //call_queue_check_after_delete(global_root);
    pthread_create(&nanager_table[0],NULL,&call_queue_check_after_delete,global_root); //Default Attributes
    pthread_join(nanager_table[0],NULL);
    // printf("Tree after delete:\n");
    // inorder(global_root);

    return 0; 
} 