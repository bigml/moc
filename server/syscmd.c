#include "mheads.h"
#include "lheads.h"

void sys_stats_init(struct stats *s)
{
    s->msg_tipc = 0;
    s->msg_tcp = 0;
    s->msg_udp = 0;
    s->msg_sctp = 0;

    s->net_version_mismatch = 0;
    s->net_broken_req = 0;
    s->net_unk_req = 0;

    s->pro_busy = 0;
}

int reply_trigger(struct queue_entry *q, uint32_t reply)
{
    if (q == NULL) return 0;

    if (q->hdfsnd == NULL || hdf_obj_child(q->hdfsnd) == NULL) {
        q->req->reply_mini(q->req, reply);
        return 1;
    }
    
    unsigned char *buf = calloc(1, MAX_PACKET_LEN);
    if (buf == NULL) {
        q->req->reply_mini(q->req, REP_ERR_MEM);
        return 0;
    }

    size_t vsize;
    vsize = pack_hdf(q->hdfsnd, buf, MAX_PACKET_LEN);
    if (vsize == 0) goto error;
 
    q->req->reply_long(q->req, reply, buf, vsize);

    free(buf);
    return 1;
    
 error:
    q->req->reply_mini(q->req, REP_ERR_PACK);
    free(buf);
    return 0;
}

NEOERR* sys_cmd_cache_get(struct queue_entry *q, struct cache *cd, bool reply)
{
    unsigned char *val = NULL;
    size_t vsize = 0;
    char *key;
    NEOERR *err = STATUS_OK;

    if (q == NULL || cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_KEY);
        goto done;
    }

    if (!cache_get(cd, (unsigned char*)key, strlen((char*)key), &val, &vsize))
        err = nerr_raise(REP_ERR_CACHE_MISS, "miss %s", key);

 done:
    if (reply) {
        if (err == STATUS_OK) {
            q->req->reply_long(q->req, reply, val, vsize);
        } else {
            q->req->reply_mini(q->req, reply);
        }
    } else {
        if (err == STATUS_OK && val != NULL && vsize > 0) {
            /* if we don't reply to client, store them in replydata */
            hdf_set_value(q->hdfsnd, VNAME_CACHE_VAL, (char*)val);
        }
    }

    return err;
}

NEOERR* sys_cmd_cache_set(struct queue_entry *q, struct cache *cd, bool reply)
{
    char *val = NULL;
    size_t vsize = 0;
    char *key;
    NEOERR *err = STATUS_OK;

    if (q == NULL || cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_KEY);
        goto done;
    }

    HDF *node = hdf_get_obj(q->hdfrcv, VNAME_CACHE_VAL);
    if (!node) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_VAL);
        goto done;
    }
    
    val = hdf_obj_value(node);
    vsize = strlen(val)+1;
    cache_set(cd, (unsigned char*)key, strlen((char*)key),
              (unsigned char*)val, vsize, 0);

 done:
    if (reply) {
        /* nothing to be returned on set, except set status */
        q->req->reply_mini(q->req, reply);
    }

    return err;
}

NEOERR* sys_cmd_cache_del(struct queue_entry *q, struct cache *cd, bool reply)
{
    char *key;
    NEOERR *err = STATUS_OK;

    if (q == NULL || cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    key = hdf_get_value(q->hdfrcv, VNAME_CACHE_KEY, NULL);
    if (!key) {
        err = nerr_raise(REP_ERR_BADPARAM, "need %s", VNAME_CACHE_KEY);
        goto done;
    }
    cache_del(cd, (unsigned char*)key, strlen(key));

 done:
    if (reply) {
        q->req->reply_mini(q->req, reply);
    }

    return err;
}

NEOERR* sys_cmd_cache_empty(struct queue_entry *q, struct cache **cd, bool reply)
{
    NEOERR *err = STATUS_OK;
    size_t num;

    if (q == NULL || cd == NULL || *cd == NULL) {
        err = nerr_raise(REP_ERR, "param null");
        goto done;
    }

    
    struct cache *oldcd = *cd;
    
    num = oldcd->numobjs;
    if (oldcd) cache_free(oldcd);

    *cd = cache_create(num, 0);
    
 done:
    if (reply) {
        q->req->reply_mini(q->req, reply);
    }

    return err;
}
