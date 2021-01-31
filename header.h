// Each node of tree has this form
typedef struct treeNode {
	int songID; 
	struct treeNode *lc;
	struct treeNode *rc;
	pthread_mutex_t lock;
}treeNode;

// A utility function to create a new BST node 
struct treeNode *create_node_with_this_value(int songID) 
{ 
    treeNode *temp =  (treeNode *)malloc(sizeof(treeNode)); 
    temp->songID = songID; 
    temp->lc = temp->rc = NULL; 
	//temp->lock;
	pthread_mutex_init(&temp->lock, NULL);
    return temp; 
} 

// Unbounded Total Queue with locks
struct queue {
	struct queueNode *Head;
	struct queueNode *Tail;
	pthread_mutex_t headLock;
	pthread_mutex_t tailLock;
}queue;

// node of Unbounded queue
struct queueNode {
	int songID;
	struct queueNode *next;
}queueNode;

struct list {
	struct listNode *head;
	struct listNode *tail;
}list;

struct listNode {
	int songID;
	struct listNode *next;
	pthread_mutex_t lock;
}listNode;
