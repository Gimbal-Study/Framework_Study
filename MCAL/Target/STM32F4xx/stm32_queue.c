#include <common.h>

int queue_full(queue_t *q)
{
	// queue에서 rear+1 % QUEUE_MAX의 값이 front와 같으면 queue full
	int tmp=(q->rear+1) % QUEUE_MAX;
	if (tmp == q->front)  // front와 같으면 queue full
	return 1;
	else return 0;
}

int queue_empty(queue_t *q)
{
	if (q->rear == q->front)  // front와 같으면 queue empty
	return 1;
	else return 0;
}

bool read_queue(queue_t *q, uint8_t *data)
{
	if (queue_empty(q))
	    return false;

	q->front = (q->front + 1) % QUEUE_MAX;
	*data =  (q->data_array[q->front]);
		
	return true;
}

void queue_init(queue_t *q)  // queue가 텅 빈경우 fron와 rear가 동일한 위치를 가리틴다.
{
	q->front=-1;   // read index
	q->rear=-1;    // insert index
}

void insert_queue(queue_t *q, uint8_t value)
{
	if (queue_full(q))
	{
		return;
	}
	else   // save
	{
		q->rear = (q->rear + 1) % QUEUE_MAX;
		q->data_array[q->rear]=value;
	}
}