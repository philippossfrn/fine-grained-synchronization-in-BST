#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <stdbool.h> 
#include <pthread.h> 
#include <assert.h>
#include <math.h>
#include "header.h"

#define KRED  "\x1B[31m"
#define RESET "\x1b[0m"
#define KGRN  "\x1B[32m"

// Global vars
int prod_threads;
struct treeNode *global_root;
int tree_size=0;
int queue_size=0;
int total_queues_size=0;
int total_keysum=0;
int queue_total_keysum=0;
pthread_barrier_t   barrier; // Create barrier for producers --used in BSTInsert
pthread_barrier_t   all_users_barrier; // Create barrier for all users --used in BSTSearch
pthread_barrier_t   except_controller_barrier; // Create barrier for all users except controller --used in BSTSearch
pthread_barrier_t   last_barrier; // Create barrier for all users except controller --used in BSTSearch
struct queue *total_queues;
int queue_size_after_delete;
int total_queue_size_after_delete;
struct list *doubly_list ;
int tree_size_after_delete;
int list_size_after_delete;


// struct insert_fun_args {
//     treeNode *node;
//     int prodID;
//     int wait;
// };



void* BSTInsert(void *id_ptr) {
    int prodID = *(int*)id_ptr;

    //printf("Producer = %d\n",prodID);
    int i;
    for (i = 0; i < prod_threads; i++){

        // Which song producer must insert
        int songID = i*prod_threads+prodID;

        pthread_mutex_lock(&global_root->lock);  //----- Why segmentation?
        
        // At first check if my tree is empty
        // if it's empty just create a new node with curr value
        // and make root points to it
        if (global_root == NULL) { 
            global_root = create_node_with_this_value(songID);
            pthread_mutex_unlock(&global_root->lock);
            continue;
            //return;
        }
        pthread_mutex_unlock(&global_root->lock);

        // Tree is NOT empty (has at least one node)
        
        // Our curr node is root(still locked)
        // I will begin to traverse the tree from root 
        // to find the right spot to put the new node
        treeNode *curr =  (treeNode *)malloc(sizeof(treeNode));
        curr = global_root;
        pthread_mutex_lock(&curr->lock);
        treeNode *next =  (treeNode *)malloc(sizeof(treeNode));
        next = NULL;
        while (i <= prod_threads-1) { //WHYYYYYYYYYYYYYYYYYYY not true 
            // The new node must be put on the left subtree 
            if (songID < curr->songID) { 
                // If the left child of curr node is null then the new node 
                // must be put here so create a new node
                // and make the left child of curr node points to it
                if (curr->lc == NULL) {
                    curr->lc = create_node_with_this_value(songID);
                    pthread_mutex_unlock(&curr->lock);
                    break;
                    //return;
                } else { 
                    // A node exists on the left child so our next node 
                    // will point to it and continue to the loop
                    next = curr->lc;
                }
            }else{ 
                // The new node must be put on the right subtree 
                if (songID > curr->songID) {
                    // If the right child of curr node is null then the new node 
                    // must be put here so create a new node
                    // and make the right child of curr node points to it
                    if (curr->rc == NULL) {
                        curr->rc = create_node_with_this_value(songID);
                        pthread_mutex_unlock(&curr->lock);
                        break;
                        //return;
                    } else {
                        // A node exists on the right child so our next node 
                        // will point to it and continue to the loop  
                        next = curr->rc;
                    }
                } else {
                    // Here means that value == curr->value so 
                    // the node already exists so unlock 
                    // curr node and return false
                    pthread_mutex_unlock(&curr->lock);
                    break;
                    //return;
                }
            }
            pthread_mutex_lock(&next->lock);
            pthread_mutex_unlock(&curr->lock);
            curr = next;
        }
        // Producer just inserted his last song
        if(i+1 == prod_threads) {
            //printf("------------------------ Poducers Barrier ------------------------");
            pthread_barrier_wait (&barrier);
            printf("\nEFTASA STO BARRIER KAI IME O PRODUCER %d KAI EVALA TELEFTEO TO %d\n",prodID,songID);
        }
    }
}

// Prrint tree inorder
void inorder(treeNode *root) 
{ 
    if (root != NULL) 
    { 
        inorder(root->lc); 
        printf("%d \n", root->songID); 
        inorder(root->rc); 
    } 
} 

// Thread with id 0 will do this check
void* tree_check(void *root){ 
    
    treeNode *gl_root = (treeNode*)root;
    if (gl_root != NULL) 
    { 
        tree_check(gl_root->lc); 
        tree_size++;
        total_keysum+=(gl_root->songID);
        tree_check(gl_root->rc); 
    } 
} 

void* BSTSearch(void *id_ptr){
    
    int userID = *(int*)id_ptr;
    int users = prod_threads;
    int queue_to_use =(userID+1) % (users/2);
    int song_to_search = users * userID;
    //printf("Producer = %d\n",prodID);
    int i;
    for (i = 0; i < users; i++){
        int song_found = 0;
        // Traverse through queues
        (queue_to_use >= (users/2)) ? queue_to_use = 0 : queue_to_use;

        // search and if true then enqueue
        //printf("IME O USER %d KAI THA ISAGO TO %d STIN OURA %d\n",userID,song_to_search,queue_to_use);
        //------------------------------------------------------------------------------------------------------
        pthread_mutex_lock(&global_root->lock);   
        // Tree is empty
        if (global_root == NULL) {
            pthread_mutex_unlock(&global_root->lock);
            //printf(KRED"IME O USER %d KAI THA ISAGO TO %d STIN OURA %d ALLA TO DENTRO INE KENO"RESET"\n",userID,song_to_search,queue_to_use);
            song_found = 0;
            continue;
        }
        pthread_mutex_unlock(&global_root->lock);

        treeNode *curr =  (treeNode *)malloc(sizeof(treeNode));
        curr = global_root;
        treeNode *next =  (treeNode *)malloc(sizeof(treeNode));
        next = NULL;

        pthread_mutex_lock(&curr->lock);

        // Traverse the tree
        while (true) {
            // Curr node has the value
            // which i want to find
            if (song_to_search == curr->songID) {
                pthread_mutex_unlock(&curr->lock);
                song_found = 1;
                //printf(KGRN"IME O USER %d KAI THA ISAGO TO %d STIN OURA %d KAI TO VRIKA"RESET"\n",userID,song_to_search,queue_to_use);
                //printf("STIN SEARCH: %d",sizeof(total_queues));
                //if(song_to_search>=0 && song_to_search <=(prod_threads*prod_threads)-1){}
                
                enqueue( (&total_queues[queue_to_use]),song_to_search);
                
                
                if(i+1 == users){
                    pthread_barrier_wait (&all_users_barrier);
                    //printf(KRED"IME O USER %d KAI MOLIS EVALA TO %d STIN OURA %d"RESET"\n",userID,song_to_search,queue_to_use);
                    if(userID != 0){
                        pthread_barrier_wait (&except_controller_barrier);
                        //printf(KRED"2nd barrier IME O USER %d KAI MOLIS EVALA TO %d STIN OURA %d"RESET"\n",userID,song_to_search,queue_to_use);
                    }
                }
                // call enqueue
                break;
            }else{
                // Value is in the left subtree
                if (song_to_search < curr->songID) {
                    // if there is no left child sth went wrong
                    if (curr->lc == NULL) {
                        pthread_mutex_unlock(&curr->lock);
                        song_found = 0;
                        //printf(KRED"IME O USER %d KAI THA ISAGO TO %d STIN OURA %d KAI DEN TO VRIKA"RESET"\n",userID,song_to_search,queue_to_use);
                        break;
                    }
                    // Continue to left subtree
                    next = curr->lc;
                // Value is in the right subtree
                }else{
                    // if there is no right child sth went wrong
                    if (curr->rc == NULL) {
                        pthread_mutex_unlock(&curr->lock);
                        song_found = 0;
                        //printf(KRED"IME O USER %d KAI THA ISAGO TO %d STIN OURA %d KAI DEN TO VRIKA"RESET"\n",userID,song_to_search,queue_to_use);
                        break;
                    }else{
                        // Continue to rigt subtree
                        next = curr->rc;
                    }
                }
            }
            
            pthread_mutex_lock(&next->lock);
            pthread_mutex_unlock(&curr->lock);
            curr = next;
        }
        //------------------------------------------------------------------------------------------------------
        
        queue_to_use++;
        song_to_search++;
        
    } 
}
int isempty(struct queue *q)
{
    return (q->Tail == NULL);
}
void enqueue(struct queue *first,int songID){
    struct queueNode *tmp2 = (struct queueNode*) malloc(sizeof(struct queueNode));
    //tmp2 = (queueNode*) malloc(sizeof(struct queueNode));
    tmp2->songID = songID;
    tmp2->next = NULL;
    pthread_mutex_init(&first->tailLock, NULL);
    pthread_mutex_lock(&first->tailLock);
    // pthread_mutex_lock(&first->tailLock); 
    // pthread_mutex_unlock(&first->tailLock);
    if (first->Tail == NULL){
        first->Head = first->Tail = tmp2;
        pthread_mutex_unlock(&first->tailLock);
    }else{
        first->Tail->next = tmp2;
        first->Tail = tmp2;
        pthread_mutex_unlock(&first->tailLock);
    }

   
    // if(!isempty(first))
    // {
        
    //     first->Tail->next = tmp2;
    //     first->Tail = tmp2;
    //     //pthread_mutex_unlock(&first->tailLock);
    // }
    // else
    // {
        
    //     first->Head = first->Tail = tmp2;
    //     //pthread_mutex_unlock(&first->tailLock);
    // }
    
}
void display(struct queueNode *head)
{
    if(head == NULL)
    {
        printf("DONE PRINTING QUEUE\n");
    }
    else
    {
        printf("\n%d\n", head -> songID);
        display(head->next);
    }
}

void call_queue_check(struct queueNode *head)
{
    if(prod_threads>=14){
        ( &total_queues[0]) ->Head = ( &total_queues[0]) ->Head->next;
    }
    int i;
    for (i= 0; i < (prod_threads/2); i++){
        // enqueue( &total_queues[i],rand());
        //printf("Printing %d queue------------------------\n",i);
        queue_check(( &total_queues[i])->Head);
        //(i==0) ? queue_check(( &total_queues[i])->Head->next) : queue_check(( &total_queues[i])->Head);
        (queue_size != (2*prod_threads) ) ? printf(KRED"Queue %d has %d elements."RESET"\n",i,queue_size) : printf(KGRN"Queue %d has %d elements."RESET"\n",i,queue_size);
        //printf(KRED"Queue %d has %d elements."RESET"\n",i,queue_size);
        queue_size = 0;
    }
    
    
    if((total_queues_size != (prod_threads*prod_threads)) && (queue_total_keysum != ((prod_threads*prod_threads)*(prod_threads-1)*(prod_threads+1)/2)) ){
        printf(KRED"Total queue's size(expected: %d , found: %d)"RESET"\n",prod_threads*prod_threads,total_queues_size);
        printf(KRED"Total queue's keysum check passed(expected: %d , found: %d)"RESET"\n",(prod_threads*prod_threads)*(prod_threads-1)*(prod_threads+1)/2,queue_total_keysum);
        exit(-42);
    }else{
        if((total_queues_size != (prod_threads*prod_threads))){
            printf(KRED"Total queue's size(expected: %d , found: %d)"RESET"\n",prod_threads*prod_threads,total_queues_size);
            printf(KGRN"Total queue's keysum check passed(expected: %d , found: %d)"RESET"\n",(prod_threads*prod_threads)*(prod_threads-1)*(prod_threads+1)/2,queue_total_keysum);
            exit(-42);
        }
        if((queue_total_keysum != ((prod_threads*prod_threads)*(prod_threads-1)*(prod_threads+1)/2))){
            printf(KGRN"Total queue's size(expected: %d , found: %d)"RESET"\n",prod_threads*prod_threads,total_queues_size);
            printf(KRED"Total queue's keysum check passed(expected: %d , found: %d)"RESET"\n",(prod_threads*prod_threads)*(prod_threads-1)*(prod_threads+1)/2,queue_total_keysum);
            exit(-42);
        }
        printf(KGRN"Total queue's size(expected: %d , found: %d)"RESET"\n",prod_threads*prod_threads,total_queues_size);
        printf(KGRN"Total queue's keysum check passed(expected: %d , found: %d)"RESET"\n",(prod_threads*prod_threads)*(prod_threads-1)*(prod_threads+1)/2,queue_total_keysum);
    }
    
}

void queue_check(struct queueNode *head)
{
    if(head == NULL)
    {
        //printf("Finish printing queue\n");
    }
    else
    {   

        queue_total_keysum += head -> songID;
        queue_size++;
        total_queues_size++;
        //printf("\n%d\n", head -> songID);
        queue_check(head->next);
    }
}


bool LL_insert(struct list *first,int songID){
    
    struct listNode *curr  = (struct listNode*) malloc(sizeof(struct listNode));
    struct listNode *pred  = (struct listNode*) malloc(sizeof(struct listNode));
    struct listNode *tmp  = (struct listNode*) malloc(sizeof(struct listNode));
    
    bool result; 
    bool return_flag = 0;

    while (true){
        pred = first->head;
        curr = pred->next;
        while( curr->songID < songID){
            pred = curr;
            tmp = curr->next;
            
            if(tmp == NULL) break;
            curr = tmp;
            
            
        }
        curr = pred;
        //lock(pred); lock(curr);
        if(true){//validate(pred,curr->next) == 
            if(songID == curr->songID){
                result = false;
                return_flag = 1;
            }else{
                struct listNode *new = (struct listNode*) malloc(sizeof(struct listNode));
                new->next= curr->next;
                
                new->songID = songID;
                pred->next = new;
                result = true;
                return_flag = 1;
            }
        }
       // break;
        // unlock(pred) unlock(curr)
        if (return_flag) return result;
    }
}
void* BSTDelete(void *id_ptr){
    int managerID = *(int*)id_ptr;
    int managers = prod_threads;
    int queue_to_use =(managerID+1) % (managers/2);
    int songID;
    
    int i;
    for (i= 0; i < (managers/2); i++){
        (queue_to_use >= (managers/2)) ? queue_to_use = 0 : queue_to_use;
        songID = dequeue( (&total_queues[queue_to_use]));
        printf("IME O MANAGER %d KAI THA DIAGRAPSO APO TIN %d OURA TO %d\n",managerID,queue_to_use,songID);
        bstdelete(songID);
        if (i+1 == managers/2 ) pthread_barrier_wait (&last_barrier);
        LL_insert(&doubly_list,songID);
        queue_to_use++;
    }
}
void bstdelete(int songID){
    //pthread_mutex_init(&global_root->lock, NULL);
   // pthread_mutex_lock(&global_root->lock);   
    
    // Tree is empty
    if (global_root == NULL) { 
        //pthread_mutex_unlock(&global_root->lock);
        return;
    } 
    
    // Root must be deleted
    if (global_root->songID == songID) { 
        
        // Tree had only one node(root)
        if (global_root->lc == NULL && global_root->rc == NULL) {
            //pthread_mutex_unlock(&global_root->lock);
            global_root = NULL;
            return;
        }

        // Root has only one child
        if (global_root->lc == NULL || global_root->rc == NULL) {
            if(global_root->rc == NULL){
                global_root = global_root->lc;
            }else{
                global_root = global_root->rc;
            }
            //pthread_mutex_unlock(&global_root->lock);
            return;
        }

        // root has two childs
        // find the minimum node on the right subtree of 
        // root replace root songID with minimum and
        // delete minimum
        treeNode *prev =  (treeNode *)malloc(sizeof(treeNode));
        prev = global_root;

        
        treeNode *min_in_right_subtree =  (treeNode *)malloc(sizeof(treeNode));
        min_in_right_subtree = global_root->rc;

       // pthread_mutex_lock(&prev->lock);   
        //pthread_mutex_lock(&min_in_right_subtree->lock);

        

        while (true) {
            // i found the minimum node so replace the root with it and delete it
            if (min_in_right_subtree->lc == NULL) { 
                global_root->songID = min_in_right_subtree->songID;
                if(prev == global_root){
                    if(min_in_right_subtree->rc == NULL){
                        prev->rc = NULL;
                    }else{
                        prev->rc = min_in_right_subtree->rc;
                    }
                }else{
                    if(min_in_right_subtree->rc == NULL){
                        prev->lc = NULL;
                    }else{
                        prev->lc = min_in_right_subtree->rc;
                    }
                   // pthread_mutex_unlock(&prev->lock);
                }
               // pthread_mutex_unlock(&global_root->lock);
                break;
            }

           // pthread_mutex_lock(&min_in_right_subtree->lc->lock);   

            // Continue until min songID is found
            if(prev != global_root){
                //pthread_mutex_unlock(&prev->lock);
            }
            prev = min_in_right_subtree;
            min_in_right_subtree = min_in_right_subtree->lc;
        }
    }
    //pthread_mutex_unlock(&global_root->lock);

    // I will begin to traverse the tree
    // to find the node which must be deleted

    treeNode *curr =  (treeNode *)malloc(sizeof(treeNode));
    curr = global_root;


    treeNode *parent =  (treeNode *)malloc(sizeof(treeNode));
    parent = NULL;

    treeNode *next =  (treeNode *)malloc(sizeof(treeNode));
    next = NULL;
    

    //pthread_mutex_lock(&curr->lock);

    
    while (true) {
        // The node which must be deleted IF it's exists 
        // is on the left subtree
        if (songID < curr->songID) {
            // If the left subtree is empty then
            // the node does not exists so return false
            if (curr->lc == NULL) { 
                //pthread_mutex_unlock(&curr->lock);
                if(parent){
                    //pthread_mutex_unlock(&parent->lock);
                }
                break;
            } else {
                // continue to left subtree
                next = curr->lc;
            }

        }else{
            // The node which must be deleted IF it's exists 
            // is on the right subtree
            if (songID > curr->songID){
                // If the right subtree is empty then
                // the node does not exists so return false
                if (curr->rc == NULL) { 
                    //pthread_mutex_unlock(&curr->lock);
                    if(parent){
                        //pthread_mutex_unlock(&parent->lock);
                    }
                    break;
                } else {
                    next = curr->rc;
                }
            // Here means that songID == cur->songID
            // so the node which must be deleted is the curr
            // Same logic as root
            }else{
                // Curr node has no childs
                if (curr->lc == NULL && curr->rc == NULL) {
                    // which child (left or right) is the curr node?
                    if (parent->lc == curr) {
                        // Node deleted
                        parent->lc = NULL;
                    } else {
                        // Node deleted
                        parent->rc = NULL;
                    }
                    if(parent){
                       // pthread_mutex_unlock(&parent->lock);
                    }
                    break;
                // Curr node has one OR two childs
                }else{
                    // Curr node has only one child
                    if (curr->lc == NULL || curr->rc == NULL) {
                        // which child (left or right) is the curr node?
                        if (parent->lc == curr) {
                            // Make the parent's left child point to
                            // the curr child so curr has been deleted
                            if(curr->lc == NULL){
                                parent->lc = curr->rc;
                            }else{
                                parent->lc = curr->lc; 
                            }
                        } else {
                            // Curr node is the right child of his parent
                            // Make the parent's right child point to
                            // the curr child so curr has been deleted
                            if(curr->lc == NULL){
                                parent->rc = curr->rc;
                            }else{
                                parent->rc = curr->lc; 
                            }
                        }
                        if(parent){
                            //pthread_mutex_unlock(&parent->lock);                       
                        }
                        break;
                    }else{
                        // Curr node has two childs
                        // When the node has two childs we find the node with the 
                        // minimum songID in the node's right subtree and 
                        // replace curr node with the minimun on his right subtree
                        // and delete minimum

                        treeNode *prev2 =  (treeNode *)malloc(sizeof(treeNode));
                        prev2 = curr;

                        treeNode *min_in_right_subtree2 =  (treeNode *)malloc(sizeof(treeNode));
                        min_in_right_subtree2 = curr->rc;
                        
                        //pthread_mutex_lock(&min_in_right_subtree2->lock);
                       // pthread_mutex_unlock(&parent->lock);

                        while (true) {
                            // i found the minimum node so replace curr with it and delete it
                            if (min_in_right_subtree2->lc == NULL) { 
                                // curr node get's the min songID of 
                                // his right subtree
                                curr->songID = min_in_right_subtree2->songID;
                                // Delete the node with min songID 
                                // Node has at worst only right child
                                if(prev2 == curr){
                                    if (min_in_right_subtree2->rc == NULL){
                                        prev2->rc = NULL;
                                    }else{
                                        prev2->rc = min_in_right_subtree2->rc;
                                    }
                                }else{
                                    if (min_in_right_subtree2->rc == NULL){
                                        prev2->lc = NULL;
                                    }else{
                                        prev2->lc = min_in_right_subtree2->rc;
                                    }

                                }
                                //pthread_mutex_unlock(&curr->lock);
                                break;
                            }

                            //pthread_mutex_lock(&min_in_right_subtree2->lc->lock);
                            
                            if(prev2 != curr){
                                //pthread_mutex_unlock(&prev2->lock);
                            }

                            // Continue until min songID is found
                            prev2 = min_in_right_subtree2;
                            min_in_right_subtree2 = min_in_right_subtree2->lc;
                        }
                    }
                }
            }
        }
        //pthread_mutex_lock(&next->lock);
        if(parent){
        //pthread_mutex_unlock(&parent->lock);
        }
        parent = curr;
        curr = next;
    }
}
int dequeue(struct queue *first){
    pthread_mutex_init(&first->tailLock, NULL);
    pthread_mutex_lock(&first->tailLock);
    int result;
    if (first->Head->next == NULL){
        result = -42;
        pthread_mutex_unlock(&first->tailLock);
    }else{
        result = first->Head->songID ;
        first->Head=first->Head->next;
        pthread_mutex_unlock(&first->tailLock);
    }
    return result;
}
// Thread with id 0 will do this check
void* tree_checck_after_delete(void *root){ 
    
    treeNode *gl_root = (treeNode*)root;
    if (gl_root != NULL) 
    { 
        tree_checck_after_delete(gl_root->lc); 
        tree_size_after_delete++;
        tree_checck_after_delete(gl_root->rc); 
    } 
} 

void printList(struct list* first) 
{ 
    first->head = first->head->next; 
    while (first->head != NULL) { 
        //printf("\t%d\n",first->head->songID);
        if(first->head->songID != -1)
            list_size_after_delete++;
        first->head = first->head->next; 
        // printf("%d",first->head->next->songID);
        // printf("%d",first->head->next->next->songID);
        // break;
        //head = head->head->next;
    } 
} 

void call_queue_check_after_delete(struct queueNode *head)
{
    // if(prod_threads>=14){
    //     ( &total_queues[0]) ->Head = ( &total_queues[0]) ->Head->next;
    // }
    int i;
    for (i = 0; i < (prod_threads/2); i++){
        // enqueue( &total_queues[i],rand());
        //printf("Printing %d queue------------------------\n",i);
        queue_check_after_delete(( &total_queues[i])->Head);
        //(i==0) ? queue_check(( &total_queues[i])->Head->next) : queue_check(( &total_queues[i])->Head);
        (queue_size_after_delete != (prod_threads) ) ? printf(KRED"Queue %d has %d elements."RESET"\n",i,queue_size_after_delete) : printf(KGRN"Queue %d has %d elements."RESET"\n",i,queue_size_after_delete);
        //printf(KRED"Queue %d has %d elements."RESET"\n",i,queue_size);
        queue_size_after_delete = 0;
        
    }
    printf("Queues’ total size check passed (expected: %d,  found: %d)\n",prod_threads*prod_threads/2,total_queue_size_after_delete);
    tree_checck_after_delete(global_root);
    printf("Tree total size check passed (expected: %d,  found: %d)\n",prod_threads*prod_threads/2,tree_size_after_delete);
    printList(&doubly_list);
    printf("List total size check passed (expected: %d,  found: %d)\n",prod_threads*prod_threads/2,list_size_after_delete);
}



void queue_check_after_delete(struct queueNode *head)
{
    if(head == NULL)
    {
        //printf("Finish printing queue\n");
    }
    else
    {   
        queue_size_after_delete++;
        total_queue_size_after_delete++;
        //printf("\n%d\n", head -> songID);
        queue_check_after_delete(head->next);
    }
}

bool validate(struct listNode *pred,struct listNode *curr){
    //printf("222222\n");
    struct listNode *tmp = (struct listNode*) malloc(sizeof(struct listNode)); 
    tmp =  doubly_list->head;
    while(tmp->songID <= pred->songID){
        if (tmp == pred){
            if(pred->next == curr->next) return true;
            else return false;
        }
        tmp = tmp->next;
    }
    return false;
}



