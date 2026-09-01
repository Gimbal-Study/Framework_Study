#ifndef MCAL_COMMON_H
#define MCAL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* ====================*Status* ==================== */

typedef enum
{
    MCAL_OK      = 0x00U,

    MCAL_ERROR   = 0x01U

} mcal_status_t;

/*Queue*/

#define QUEUE_MAX 1000

typedef struct
{
    int front;
    int rear;
    uint8_t data_array[QUEUE_MAX];
} queue_t;

int queue_full(queue_t *q);
int queue_empty(queue_t *q);
bool read_queue(queue_t *q, uint8_t *data);
void queue_init(queue_t *q);
void insert_queue(queue_t *q, uint8_t value);


#endif /* MCAL_COMMON_H */

