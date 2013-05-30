#ifndef __MOCD_H__
#define __MOCD_H__

/*
 * private, internal use 
 */
struct moc {
    size_t numevts;
    size_t hashlen;
    size_t chainlen;
    
    struct event_chain *table;
};

struct event_chain {
    size_t len;
    struct event_entry *first;
    struct event_entry *last;
};

struct timer_entry {
    int timeout;
    bool repeat;
    void (*timer)(struct event_entry *e, unsigned int upsec);
    struct timer_entry *next;
};

struct event_entry {
    /*
     * public, init in moc_start_driver()
     */
    //void *lib;        /* for dlopen() */
    struct queue *op_queue;
    pthread_t *op_thread;
    int loop_should_stop;
    struct event_entry *prev;
    struct event_entry *next;
    struct timer_entry *timers;

    /*
     * different by plugin, init in init_driver()
     */
    unsigned char *name;
    size_t ksize;
    void (*process_driver)(struct event_entry *e, struct queue_entry *q);
    void (*stop_driver)(struct event_entry *e);

    /*
     * extensions after here...
     */
};
struct event_driver {
    unsigned char *name;
    struct event_entry* (*init_driver)(void);
};

typedef struct event_entry EventEntry;

/*
 * public
 */
struct moc* moc_start();
void moc_stop(struct moc *evt);
void moc_add_timer(struct timer_entry **timers, int timeout, bool repeat,
                   void (*timer)(struct event_entry *e, unsigned int upsec));

struct event_entry* find_entry_in_table(struct moc *evt,
                                        const unsigned char *key, size_t ksize);

#endif    /* __MOCD_H__ */
