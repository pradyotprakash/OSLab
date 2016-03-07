#include <stdio.h>
struct queue{
	struct node *head, *tail;
	int max_len;
	int curr_len;
} waiting;

struct node{
	int val;
	struct node* next;
};

void add(struct queue *Q, int fd){

	struct node * n = (struct node*) malloc(sizeof(struct node));
	n->val = fd;
	// while(Q->curr_len == Q->max_len && Q->max_len!=0){
	// 	pthread_cond_wait(&has_space, &mutex);
	// }

	if(Q->head == NULL){
		Q->head = Q->tail = n;
	}
	else{
		Q->tail->next = n;
		Q->tail = n;
	}

	Q->curr_len++;
	n->next = NULL;
}

int pop_head(struct queue *Q){

	int ret;
	if(Q->head == NULL){
		return -1;
	}
	ret = Q->head->val;
	// while(Q->curr_len == 0){
	// 	pthread_cond_wait(&has_data, &mutex);
	// }

	if(Q->head == Q->tail){

		free(Q->head);
		Q->head = Q->tail = NULL;
	}
	else{
		struct node * temp = Q->head;
		Q->head = Q->head->next;
		free(temp);
	}
	Q->curr_len--;
	return ret;
}

void print(struct queue* Q) {
	if(Q->head==NULL){
		printf("Empty queue\n");
		return;
	}
	struct node* ptr = Q->head;
	while(ptr != NULL){
		printf("%d ->", ptr->val);
		ptr = ptr->next;
	}
	printf("\n");
}

int main(){
	struct queue Q;
	Q.head = Q.tail = NULL;
	// printf("%p\n", Q.head);
	add(&Q, 1);
	add(&Q, 2);
	print(&Q);
	int a = pop_head(&Q);
	int b = pop_head(&Q);
	pop_head(&Q);
	printf("Popped val: %d\n", a);
	print(&Q);
}