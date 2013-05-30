#include "moc_plugin.h"
#include "moc_base.h"
#include "base_pri.h"

NEOERR* base_info_init(struct base_info **binfo)
{
    NEOERR *err;

    MCS_NOT_NULLA(binfo);

    if (!*binfo) {
        struct base_info *linfo = calloc(1, sizeof(struct base_info));
        if (!linfo) return nerr_raise(NERR_NOMEM, "alloc base failure");
        linfo->usernum = 0;
        err = hash_init(&linfo->userh, hash_str_hash, hash_str_comp);
        if (err != STATUS_OK) return nerr_pass(err);

        *binfo = linfo;
    }

    return STATUS_OK;
}

/*
 * user
 */
struct base_user *base_user_find(struct base_info *binfo, char *uid)
{
    if (!binfo || !uid) return NULL;
    
    return (struct base_user*)hash_lookup(binfo->userh, uid);
}

struct base_user *base_user_new(struct base_info *binfo, char *uid, QueueEntry *q)
{
    if (!binfo || !uid || !q || !q->req) return NULL;
    
    struct base_user *user = calloc(1, sizeof(struct base_user));
    if (!user) return NULL;

    struct sockaddr_in *clisa = (struct sockaddr_in*)q->req->clisa;

    //strncpy(user->ip, inet_ntoa(clisa), 16);
    user->uid = strdup(uid);
    user->fd = q->req->fd;
    inet_ntop(clisa->sin_family, &clisa->sin_addr,
              user->ip, sizeof(user->ip));
    user->port = ntohs(clisa->sin_port);
    user->tcpsock = q->req->tcpsock;
    user->baseinfo = binfo;

    /*
     * used on user close
     */
    if (q->req->tcpsock) {
        q->req->tcpsock->appdata = user;
        q->req->tcpsock->on_close = base_user_destroy;
    }
    
    /*
     * binfo
     */
    hash_insert(binfo->userh, (void*)strdup(uid), (void*)user);
    binfo->usernum++;
    
    mtc_dbg("%s %s %d join", uid, user->ip, user->port);
    
    return user;
}

bool base_user_quit(struct base_info *binfo, char *uid)
{
    struct tcp_socket *tcpsock;
    struct base_user *user;

    user = base_user_find(binfo, uid);
    if (!user) return false;
    
    mtc_dbg("%s %s %d quit", user->uid, user->ip, user->port);

    close(user->fd);

    /*
     * TODO
     * we should do this on main thread.
     * it works fine on test, don't known how works on future
     */
    tcpsock = user->tcpsock;
    if (tcpsock) {
        tcpsock->appdata = NULL;
        tcpsock->on_close = NULL;
        event_del(tcpsock->evt);
        tcp_socket_free(tcpsock);
    }
    
    base_user_destroy(user);
    
    return true;
}

void base_user_destroy(void *arg)
{
    struct base_user *user = (struct base_user*)arg;
    struct base_info *binfo = user->baseinfo;

    if (!user || !binfo) return;
    
    mtc_dbg("%s %s %d destroy", user->uid, user->ip, user->port);

    hash_remove(binfo->userh, user->uid);
    if (binfo->usernum > 0) binfo->usernum--;

    SAFE_FREE(user->uid);
    SAFE_FREE(user);

    return;
}


/*
 * msg
 */
static unsigned char static_buf[MAX_PACKET_LEN];

NEOERR* base_msg_new(char *cmd, HDF *datanode, unsigned char **buf, size_t *size)
{
    NEOERR *err;

    MCS_NOT_NULLC(cmd, datanode, buf);
    MCS_NOT_NULLA(size);

    size_t bsize;
    unsigned char *rbuf;
    uint32_t t;

    memset(static_buf, MAX_PACKET_LEN, 0x0);

    hdf_set_value(datanode, "_Reserve", "moc");
    err = hdf_set_attr(datanode, "_Reserve", "cmd", cmd);
    if (err != STATUS_OK) return nerr_pass(err);

    TRACE_HDF(datanode);

    bsize = pack_hdf(datanode, static_buf, MAX_PACKET_LEN);
    if(bsize <= 0) return nerr_raise(NERR_ASSERT, "packet error");

    /*
     * copy from tcp.c tcp_reply_long()
     */
    bsize = 4 + 4 + 4 + 4 + bsize;
    rbuf = calloc(1, bsize);
    if (!rbuf) return nerr_raise(NERR_NOMEM, "alloc msg buffer");

    t = htonl(bsize);
    memcpy(rbuf, &t, 4);

    /*
     * server 主动发给 client 的包，reqid == 0, && reply == 10000
     */
    t = 0;
    memcpy(rbuf + 4, &t, 4);
    t = htonl(10000);
    memcpy(rbuf + 8, &t, 4);
    
    t = htonl(bsize);
    memcpy(rbuf + 12, &t, 4);
    memcpy(rbuf + 16, static_buf, bsize - 16);

    *buf = rbuf;
    *size = bsize;

    return STATUS_OK;
}

NEOERR* base_msg_reply(unsigned char *buf, size_t size, int fd)
{
    MCS_NOT_NULLA(buf);
    if (fd <= 0) return nerr_raise(NERR_ASSERT, "fd 非法");

    /*
     * copy from tcp.c rep_send()
     */
    size_t rv, c;

    MSG_DUMP("send: ",  buf, size);
    
    c = 0;
    while (c < size) {
        rv = send(fd, buf + c, size - c, 0);

        if (rv == size) return STATUS_OK;
        else if (rv == 0) return STATUS_OK;
        else if (rv < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) continue;
            return nerr_raise(NERR_IO, "send return %ld", rv);
        }

        c += rv;
    }

    return STATUS_OK;
}

void base_msg_free(unsigned char *buf)
{
    if (!buf) return;
    free(buf);
}
