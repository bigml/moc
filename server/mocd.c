#include "mheads.h"
#include "lheads.h"

/*
 * The hash function used is the "One at a time" function, which seems simple,
 * fast and popular. Others for future consideration if speed becomes an issue
 * include:
 *  * FNV Hash (http://www.isthe.com/chongo/tech/comp/fnv/)
 *  * SuperFastHash (http://www.azillionmonkeys.com/qed/hash.html)
 *  * Judy dynamic arrays (http://judy.sf.net)
 *
 * A good comparison can be found at
 * http://eternallyconfuzzled.com/tuts/hashing.html#existing
 */

static uint32_t hash(const unsigned char *key, const size_t ksize)
{
    uint32_t h = 0;
    size_t i;

    for (i = 0; i < ksize; i++) {
        h += key[i];
        h += (h << 10);
        h ^= (h >> 6);
    }
    h += (h << 3);
    h ^= (h >> 11);
    h += (h << 15);
    return h;
}

static void* moc_start_base_entry(void *arg)
{
    int rv;
    struct timespec ts;
    struct queue_entry *q;

    struct event_entry *e = (struct event_entry*)arg;

    for (;;) {
        /* Condition waits are specified with absolute timeouts, see
         * pthread_cond_timedwait()'s SUSv3 specification for more
         * information. We need to calculate it each time.
         * We sleep for 1 sec. There's no real need for it to be too
         * fast (it's only used so that stop detection doesn't take
         * long), but we don't want it to be too slow either. */
        mutil_utc_time(&ts);
        ts.tv_sec += 1;

        rv = 0;
        queue_lock(e->op_queue);
        while (queue_isempty(e->op_queue) && rv == 0) {
            rv = queue_timedwait(e->op_queue, &ts);
        }

        if (rv != 0 && rv != ETIMEDOUT) {
            mtc_err("Error in queue_timedwait()");
            /* When the timedwait fails the lock is released, so
             * we need to properly annotate this case. */
            __release(e->op_queue->lock);
            continue;
        }

        q = queue_get(e->op_queue);
        queue_unlock(e->op_queue);

        if (q == NULL) {
            if (e->loop_should_stop) {
                break;
            } else {
                continue;
            }
        }

        e->process_driver(e, q);

        /* Free the entry that was allocated when tipc queued the
         * operation. This also frees it's components. */
        queue_entry_free(q);
    }
    
    return NULL;
}

static void moc_stop_driver(struct event_entry *e)
{
    if (e == NULL) return;

    //dlclose(e->lib);
    e->loop_should_stop = 1;
    e->stop_driver(e);
    pthread_join(*(e->op_thread), NULL);
    free(e->op_thread);
    queue_free(e->op_queue);
    if (e->name != NULL) free(e->name);
    free(e);
}

static int moc_start_driver(struct moc *evt, struct event_driver *d, void *lib)
{
    if (evt == NULL || evt->table == NULL || d == NULL) return 0;

    struct event_entry *e = d->init_driver();
    if (e == NULL) return 0;

    //e->lib = lib;
    e->op_queue = queue_create();
    e->op_thread = malloc(sizeof(pthread_t));
    pthread_create(e->op_thread, NULL, moc_start_base_entry, (void*)e);
    
    uint32_t h;
    struct event_chain *c;
    h = hash(e->name, e->ksize) % evt->hashlen;
    c = evt->table + h;

    if (c->len == 0) {
        c->first = e;
        c->last = e;
        c->len = 1;
    } else if (c->len <= evt->chainlen) {
        e->next = c->first;
        c->first->prev = e;
        c->first = e;
        c->len += 1;
    } else {
        /* chain is full, we need to evict the last one */
        moc_stop_driver(e);
        return 0;
    }
    
    return 1;
}

/* Looks up the given key in the chain. Returns NULL if not found, or a
 * pointer to the moc entry if it is. The chain can be empty. */
static struct event_entry *find_in_chain(struct event_chain *c,
                                         const unsigned char *key, size_t ksize)
{
    struct event_entry *e;

    for (e = c->first; e != NULL; e = e->next) {
        if (ksize != e->ksize) {
            continue;
        }
        if (memcmp(key, e->name, ksize) == 0) {
            break;
        }
    }

    /* e will be either the found chain or NULL */
    return e;
}



struct moc* moc_start(void)
{
    int ret;

    struct moc *evt = calloc(1, sizeof(struct moc));
    if (evt == NULL) return NULL;
    
    evt->numevts = 0;
    evt->chainlen = 1000000;
    //evt->hashlen = evt->numevts / evt->chainlen;
    evt->hashlen = 16;
    evt->table = calloc(evt->hashlen, sizeof(struct event_chain));

    void *lib;
    char tbuf[1024], *tp;
    struct event_driver *driver;
    HDF *res = hdf_get_obj(g_cfg, PRE_SERVER".plugins.0");
    while (res != NULL) {
        lib = NULL; driver = NULL; memset(tbuf, 0x0, sizeof(tbuf));
        
        snprintf(tbuf, sizeof(tbuf), "%smoc_%s.so", PLUGIN_PATH, hdf_obj_value(res));
        //lib = dlopen(tbuf, RTLD_NOW|RTLD_GLOBAL);
        lib = dlopen(tbuf, RTLD_LAZY|RTLD_GLOBAL);
        if (lib == NULL) {
            mtc_err("open driver %s failure %s", tbuf, dlerror());
            res = hdf_obj_next(res);
            continue;
        }

        snprintf(tbuf, sizeof(tbuf), "%s_driver", hdf_obj_value(res));
        driver = (struct event_driver*)dlsym(lib, tbuf);
        if ((tp = dlerror()) != NULL) {
            mtc_err("find symbol %s failure %s", tbuf, tp);
            res = hdf_obj_next(res);
            continue;
        }

        ret = moc_start_driver(evt, driver, lib);
        if (ret != 1) mtc_err("init driver %s failure", hdf_obj_value(res));
        else {
            mtc_dbg("init driver %s ok", hdf_obj_value(res));
            evt->numevts++;
        }
        
        res = hdf_obj_next(res);
    }

    return evt;
}

void moc_stop(struct moc *evt)
{
    size_t i;
    struct event_chain *c;
    struct event_entry *e, *n;

    if (evt == NULL) return;
    for (i = 0; i < evt->hashlen; i++) {
        c = evt->table + i;
        if (c->first == NULL)
            continue;

        e = c->first;
        while (e != NULL) {
            n = e->next;
            moc_stop_driver(e);
            e = n;
        }
    }

    free(evt->table);
    free(evt);
}

void moc_add_timer(struct timer_entry **timers, int timeout, bool repeat,
                   void (*timer)(struct event_entry *e, unsigned int upsec))
{
    if (!timers || !timer) return;

    struct timer_entry *t = calloc(1, sizeof(struct timer_entry));
    if (t) {
        t->timeout = timeout;
        t->repeat = repeat;
        t->timer = timer;
        t->next = *timers;
        *timers = t;
    }
}

struct event_entry* find_entry_in_table(struct moc *evt,
                                        const unsigned char *key, size_t ksize)
{
    uint32_t h;
    struct event_chain *c;
    h = hash(key, ksize) % evt->hashlen;
    c = evt->table + h;

    return find_in_chain(c, key, ksize);
}
