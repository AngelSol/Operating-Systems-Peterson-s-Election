//Angel Solis
//CS 370
//Project 4
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>


typedef struct channel{ // channels to connect the nodes
	sem_t sync;  // initial 0
	sem_t race_c; // initial 1
	int head; //head 
	int tail;
	int *list;
	int size;
	
	
	// queue needed
} channel;
typedef struct node {	
	channel * left;
	channel * right;
	int round,id,nextid;
	int relay;
	
} node;




int pop(channel *current) // removes and returns head item on the list
{
	int data = current->list[current->head];
	current->head ++;
	current->size --;
	return data;
}

void push(channel *current, int value)	// pushes a value into the end of queue
{
	current -> list[current->tail] = value;
	current -> tail++;
	current -> size++;	
	
}
int reader (channel *toread) // reads from the channel and removes it
{
	int value;
	sem_wait(&toread->sync);
	sem_wait(&toread->race_c);
	
	value = pop(toread);
	sem_post(&toread->race_c);
	return value;
	
}
void writer(channel* towrite,int value) // writes to the channel
{
	sem_wait(&towrite->race_c);
	push(towrite,value);
	sem_post(&towrite->race_c);
	sem_post(&towrite->sync);
}


void setup(node* ring, channel* lines, int nodenum)
{
	int i;
	

	//channel setup ring

	
	for (i=0;i<nodenum;i++) //initialize channels
	{
		channel next;
		next.list = calloc(nodenum*2,sizeof(int));
		next.head =0;
		next.tail =0;
		next.size =0;
		sem_init(&next.race_c,0,1);
		sem_init(&next.sync,0,0);
		
		lines[i] = next;
	
	}		

	for(i=0;i<nodenum;i++) //initalize nodes
	{
 		node temp;
		
		scanf("%d",&temp.id);
		temp.round = 1;
		temp.nextid = temp.id;
		temp.left = &lines[i];
		temp.relay = 0;
		if (i == nodenum-1)
			temp.right = &lines[0];
		else
			temp.right = &lines[i+1];
		ring[i] = temp;

	
	}

}
void activenode(node* current)
{
	while (1)
	{
		if (current->relay == 0) //if not a relay
		{
			//print first 
			printf("[%d][%d][%d]\n",current->round,current->id,current->nextid);


			writer(current->right,current->nextid);
			int hop1 = reader(current->left);//read hop 1
			writer(current->right,hop1); //write one hop
			int hop2 = reader(current ->left); 
		

			if (hop1 == current->nextid) //leader found
			{
				printf("Leader: %d\n",current->id);
				writer(current->right,-1); //set sentenal
				pthread_exit(NULL); //exit thread
			}			
			else if (hop1 > hop2 && hop1 > current->nextid)// not a relay and not yet leader
			{
				current->nextid = hop1;
				current->round++;
			}
			else // is now a relay
			{
				current->relay = 1;
			}
		}
		else if(current->relay == 1) // is relay
		{
			int hop1 = reader(current->left);
			writer(current->right,hop1);  //write hop1
			if (hop1 == -1) //check for sentenal
				pthread_exit(NULL);

			int hop2 = reader(current->left);
			writer(current->right,hop2);
		}
	}
}
int main()
{

	int nodenum;
	
	scanf("%d",&nodenum); //get number of nodes
	node *ring = calloc(nodenum, sizeof(node)); // create ring of nodes
	channel *lines = calloc (nodenum, sizeof(channel));// create channels
	setup(ring,lines,nodenum);	// call for the setup
	

	pthread_t threadid[nodenum];
	int i;
	for (i=0;i<nodenum;i++) //create all threads
	{
		pthread_create(&threadid[i],NULL, (void *) activenode,(void*)&ring[i]);
	}
	
	for (i=0;i<nodenum;i++) // wait for threads to finish
	{
		pthread_join(threadid[i],NULL);
	}

	free(ring);	//free memory
	free(lines);
	return 0;
}

