#include "moc_plugin.h"
#include "moc_chat.h"
#include "chat_pri.h"
#include "moc_base.h"

static BaseInfo *m_base = NULL;

static NEOERR* cmd_join(struct chat_entry *e, QueueEntry *q)
{
    BaseUser *user;
    char *uid;
    unsigned char *msgbuf = NULL;
    size_t msgsize = 0;
    HDF *msgnode;
    NEOERR *err;

    REQ_GET_PARAM_STR(q->hdfrcv, "userid", uid);

    base_user_quit(m_base, uid);
    base_user_new(m_base, uid, q);

    hdf_set_value(q->hdfrcv, PRE_OUTPUT".userid", uid);
    msgnode = hdf_get_obj(q->hdfrcv, PRE_OUTPUT);

    err = base_msg_new("join", msgnode,  &msgbuf, &msgsize);
    if (err != STATUS_OK) return nerr_pass(err);
    
    USER_START(m_base->userh, user) {
        if (strcmp(uid, user->uid)) {
            mtc_dbg("need to tel %s", user->uid);

            base_msg_reply(msgbuf, msgsize, user->fd);
        }

        user = USER_NEXT(m_base->userh);
    } USER_END;

    base_msg_free(msgbuf);

    return STATUS_OK;
}

static NEOERR* cmd_quit(struct chat_entry *e, QueueEntry *q)
{
    BaseUser *user;
    char *uid;
    unsigned char *msgbuf = NULL;
    size_t msgsize = 0;
    HDF *msgnode;
    NEOERR *err;

    REQ_GET_PARAM_STR(q->hdfrcv, "userid", uid);

    hdf_set_value(q->hdfrcv, PRE_OUTPUT".userid", uid);
    msgnode = hdf_get_obj(q->hdfrcv, PRE_OUTPUT);

    err = base_msg_new("quit", msgnode,  &msgbuf, &msgsize);
    if (err != STATUS_OK) return nerr_pass(err);
    
    USER_START(m_base->userh, user) {
        if (strcmp(uid, user->uid)) {
            mtc_dbg("need to tel %s", user->uid);

            base_msg_reply(msgbuf, msgsize, user->fd);
        }

        user = USER_NEXT(m_base->userh);
    } USER_END;

    base_msg_free(msgbuf);

    base_user_quit(m_base, uid);

    return STATUS_OK;
}

static NEOERR* cmd_bcst(struct chat_entry *e, QueueEntry *q)
{
    char *uid, *msg;
    BaseUser *user;
    unsigned char *msgbuf = NULL;
    size_t msgsize = 0;
    HDF *msgnode;
    NEOERR *err;

    BASE_GET_UID(q, uid);
    REQ_GET_PARAM_STR(q->hdfrcv, "msg", msg);

    mtc_dbg("%s broadcast: %s", uid, msg);

    hdf_set_value(q->hdfrcv, PRE_OUTPUT".userid", uid);
    hdf_set_value(q->hdfrcv, PRE_OUTPUT".msg", msg);
    msgnode = hdf_get_obj(q->hdfrcv, PRE_OUTPUT);

    err = base_msg_new("bcst", msgnode,  &msgbuf, &msgsize);
    if (err != STATUS_OK) return nerr_pass(err);
    
    USER_START(m_base->userh, user) {
        if (strcmp(uid, user->uid)) {
            mtc_dbg("need to tel %s", user->uid);

            base_msg_reply(msgbuf, msgsize, user->fd);
        }

        user = USER_NEXT(m_base->userh);
    } USER_END;

    base_msg_free(msgbuf);

    return STATUS_OK;
}

static NEOERR* cmd_pm(struct chat_entry *e, QueueEntry *q)
{
    char *uid;

    REQ_GET_PARAM_STR(q->hdfrcv, "userid", uid);

    return STATUS_OK;
}

static void chat_process_driver(EventEntry *entry, QueueEntry *q)
{
    struct chat_entry *e = (struct chat_entry*)entry;
    NEOERR *err = NULL;
    int ret;
    
    struct chat_stats *st = &(e->st);

    st->msg_total++;
    
    mtc_dbg("process cmd %u", q->operation);
    switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, e->cd, err);
    case REQ_CMD_BASE_JOIN:
        err = cmd_join(e, q);
        break;
    case REQ_CMD_BASE_QUIT:
        err = cmd_quit(e, q);
        break;
    case REQ_CMD_CHAT_BCST:
        err = cmd_bcst(e, q);
        break;
    case REQ_CMD_CHAT_PM:
        err = cmd_pm(e, q);
        break;
    case REQ_CMD_STATS:
        st->msg_stats++;
        err = STATUS_OK;
        hdf_set_int_value(q->hdfsnd, "msg_total", st->msg_total);
        hdf_set_int_value(q->hdfsnd, "msg_unrec", st->msg_unrec);
        hdf_set_int_value(q->hdfsnd, "msg_badparam", st->msg_badparam);
        hdf_set_int_value(q->hdfsnd, "msg_stats", st->msg_stats);
        hdf_set_int_value(q->hdfsnd, "proc_suc", st->proc_suc);
        hdf_set_int_value(q->hdfsnd, "proc_fai", st->proc_fai);
        break;
    default:
        st->msg_unrec++;
        err = nerr_raise(REP_ERR_UNKREQ, "unknown command %u", q->operation);
        break;
    }
    
    NEOERR *neede = mcs_err_valid(err);
    ret = neede ? neede->error : REP_OK;
    if (PROCESS_OK(ret)) {
        st->proc_suc++;
    } else {
        st->proc_fai++;
        if (ret == REP_ERR_BADPARAM) {
            st->msg_badparam++;
        }
        TRACE_ERR(q, ret, err);
    }
    if (q->req->flags & FLAGS_SYNC) {
        reply_trigger(q, ret);
    }
}

static void chat_stop_driver(EventEntry *entry)
{
    struct chat_entry *e = (struct chat_entry*)entry;

    /*
     * e->base.name, e->base will free by moc_stop_driver() 
     */
    mdb_destroy(e->db);
    cache_free(e->cd);
}



static EventEntry* chat_init_driver(void)
{
    struct chat_entry *e = calloc(1, sizeof(struct chat_entry));
    if (e == NULL) return NULL;
    NEOERR *err;

    e->base.name = (unsigned char*)strdup(PLUGIN_NAME);
    e->base.ksize = strlen(PLUGIN_NAME);
    e->base.process_driver = chat_process_driver;
    e->base.stop_driver = chat_stop_driver;
    //moc_add_timer(&e->base.timers, 60, true, hint_timer_up_term);

    //char *s = hdf_get_value(g_cfg, CONFIG_PATH".dbsn", NULL);
    //err = mdb_init(&e->db, s);
    //JUMP_NOK(err, error);

    err = base_info_init(&m_base);
    JUMP_NOK(err, error);
    
    e->cd = cache_create(hdf_get_int_value(g_cfg, CONFIG_PATH".numobjs", 1024), 0);
    if (e->cd == NULL) {
        mtc_err("init cache failure");
        goto error;
    }
    
    return (EventEntry*)e;
    
error:
    if (e->base.name) free(e->base.name);
    if (e->db) mdb_destroy(e->db);
    if (e->cd) cache_free(e->cd);
    free(e);
    return NULL;
}

struct event_driver chat_driver = {
    .name = (unsigned char*)PLUGIN_NAME,
    .init_driver = chat_init_driver,
};
