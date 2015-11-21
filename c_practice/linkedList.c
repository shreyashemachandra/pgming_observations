#include<stdio.h>

struct Node{
	int data;
	struct Node *next;
} *head;

int insert_sortedly(){
	struct Node *newNode = (struct Node *) malloc(sizeof(struct Node));
        if(newNode == NULL){
                printf("Mem Emp\n");
                return -1;
        }else{
		scanf("%d",&newNode->data);
		newNode->next = NULL;
		if(head == NULL){
			head = newNode;
		}else{
			struct Node *temp = head;

			while(temp->next != NULL){
				if(temp->data < newNode->data)
					temp = temp->next;
				else
					break;
			}
			newNode->next = temp->next;
			temp->next = newNode;
		}	
	}
	return 1;
}

int insert_at_end(){
	struct Node *newNode = (struct Node *) malloc(sizeof(struct Node));
	if(newNode == NULL){
		printf("Mem Emp\n");
		return -1;
	}else{
		scanf("%d",&newNode->data);
		newNode->next = NULL;
		
		if(head == NULL){
			head = newNode;
		}else{
			struct Node *temp = head;

			while(temp->next != NULL){
				temp=temp->next;
			}
			temp->next=newNode;
		}
		return 1;
	}
}

void display(void){
	struct Node *temp=head;
	printf("\n");
	while(temp != NULL){
		printf("%d ->",temp->data);
		temp=temp->next;
	}
	printf("\n");
}

void delete_last(){
	if(head==NULL){
		printf("List Emp\n");
	}else if(head->next == NULL){
		printf("Deleted Element: %d",head->data);
		head=NULL;
	}else{
		struct Node *temp=head;
		while(temp->next->next != NULL){
			temp=temp->next;
		}
		printf("Deleted Element: %d",temp->next->data);
		temp->next=NULL;
	}
}

void reversal(){
	if(head == NULL){
		printf("No List exist to reverse\n");
	}else if(head->next == NULL){
		printf("Only 1 Element\n");
	}else if(head->next->next == NULL){
		struct Node *temp = head->next;
		temp->next=head;
		head->next=NULL;
		head=temp;
	}else{
		struct Node *i,*j,*k;
		i=head;
		j=head->next;
		k=head->next->next;
		head->next=NULL;
	
		while(k != NULL){
			j->next = i;
			i = j;
			j = k;
			k = k->next;
		}
		head = j;
		j->next = i;
	}
}


int main(){
	int n;
	while(1){
		printf("Enter the choice:");
		printf("\n 1: Insert to the End\n 2: Display\n 3: Delete last\n 4: Reverse\n 5: Insert Sortedly");
		scanf("%d",&n);		
		switch(n){
			case 1: insert_at_end();
				break;
			case 2: display();
				break;
			case 3: delete_last();
				break;
			case 4: reversal();
				break;
			case 5: insert_sortedly();
				break;
			default: exit(0);
		}
	}

	return 0;
}
