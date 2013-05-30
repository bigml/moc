#ifndef __MSCLI_H__
#define __MSCLI_H__

__BEGIN_DECLS

/*
 * sync
 */
void mssync_create(struct mssync *s);
void mssync_destroy(struct mssync *s);
void mssync_lock(struct mssync *s);
void mssync_unlock(struct mssync *s);
void mssync_signal(struct mssync *s);
int mssync_timedwait(struct mssync *s, struct timespec *ts);

/*
 * queue
 * without thread safe provide
 * please combine with mssync_xxx to use
 */
struct msqueue* msqueue_create();
void msqueue_destroy(struct msqueue *q);
int msqueue_isempty(struct msqueue *q);

struct msqueue_entry* msqueue_entry_create();
void msqueue_entry_destroy(struct msqueue_entry *e);

struct msqueue_entry *msqueue_get(struct msqueue *q);
void msqueue_put(struct msqueue *q, struct msqueue_entry *e);
void msqueue_cas(struct msqueue *q, struct msqueue_entry *e);

/*
 * parse
 */
void msparse_buf(moc_t *evt, int order, int fd,
                 unsigned char *buf, size_t len, moc_arg *arg);

void mssrv_close(moc_t *evt, int order, int fd);


__END_DECLS
#endif  /* __MSCLI_H__ */
