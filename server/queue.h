#ifndef _QUEUE_H
#define _QUEUE_H

#define QUEUE_SIZE_INFO        100
#define QUEUE_SIZE_WARNING     1000000
#define MAX_QUEUE_ENTRY        2097152

#ifdef __CHECKER__
# define __acquires(x) __attribute__((exact_context(x,0,1)))
# define __releases(x) __attribute__((exact_context(x,1,0)))
# define __with_lock_acquired(x) __attribute__((exact_context(x,1,1)))
# define __acquire(x) __context__(x,1,0)
# define __release(x) __context__(x,-1,1)
#else
# define __acquires(x)
# define __releases(x)
# define __with_lock_acquired(x)
# define __acquire(x) (void)0
# define __release(x) (void)0
#endif

struct queue {
    pthread_mutex_t lock;
    pthread_cond_t cond;

    size_t size;
    struct queue_entry *top, *bottom;
};

struct queue_entry {
    uint32_t operation;
    struct req_info *req;

    unsigned char *ename;
    size_t esize;
    HDF *hdfrcv;
    HDF *hdfsnd;

    struct queue_entry *prev;
    /* A pointer to the next element on the list is actually not
     * necessary, because it's not needed for put and get.
     */
};

typedef struct queue_entry QueueEntry;

struct queue *queue_create();
void queue_free(struct queue *q);

struct queue_entry *queue_entry_create();
void queue_entry_free(struct queue_entry *e);

size_t queue_entry_size(struct queue_entry *e);

void queue_lock(struct queue *q)
    __acquires(q->lock);
void queue_unlock(struct queue *q)
    __releases(q->lock);
void queue_signal(struct queue *q);
int queue_timedwait(struct queue *q, struct timespec *ts)
    __with_lock_acquired(q->lock);

void queue_put(struct queue *q, struct queue_entry *e)
    __with_lock_acquired(q->lock);
void queue_cas(struct queue *q, struct queue_entry *e)
    __with_lock_acquired(q->lock);
struct queue_entry *queue_get(struct queue *q)
    __with_lock_acquired(q->lock);
int queue_isempty(struct queue *q)
    __with_lock_acquired(q->lock);

#endif

