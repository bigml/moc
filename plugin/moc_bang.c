#include "moc_plugin.h"
#include "moc_base.h"

#include "moc_bang.h"
#include "bang_pri.h"

static BangInfo *m_bang = NULL;

/*
 *[> overwrite join and quit command of base module <]
 */
static NEOERR* cmd_join(struct bang_entry *e, QueueEntry *q)
{
    NEOERR *err;

    err = bang_user_new(m_bang, e, q, NULL);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }
    hdf_set_value(q->hdfsnd, "success", "1");

    if (!(q->req->flags & FLAGS_SYNC)) {
        err = base_msg_touser("login", q->hdfsnd, q->req->fd);
        if (err != STATUS_OK) {
            return nerr_pass(err);
        }
    }

    return STATUS_OK;
}

static NEOERR* cmd_quit(struct bang_entry *e, QueueEntry *q)
{
    char *uid;

    BASE_GET_UID(q, uid);

    base_user_quit(m_bang->inherited_info, uid, q, bang_user_destroy);

    return STATUS_OK;
}

static NEOERR* cmd_to_battle(struct bang_entry *e, QueueEntry *q)
{
    char     *uid;
    BangUser *user;
    BangInfo *info = m_bang;
    NEOERR   *err;

    BASE_GET_USER(q, user);
    uid = user->inherited_user.uid;

    err = bang_broadcast_to_battle_message(info, user);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }

    return STATUS_OK;
}

static NEOERR* cmd_accept_battle(struct bang_entry *e, QueueEntry *q)
{
    char     *uid;
    BangUser *user;
    BangUser *ouser;
    BangInfo *info = m_bang;

    NEOERR   *err;

    BASE_GET_USER(q, user);
    user->state = BANG_STATE_ACCEPT_BATTLE;

    mtc_dbg("user %s accepts battle request", user->inherited_user.uid);

    /* TODO match any users has been responsed, not only one! */
    USER_START(user->current_battling_table->battling_user_hash, ouser) {
        if (ouser != user && ouser->state != BANG_STATE_TO_BATTLE) {
            hash_remove(user->current_battling_table->battling_user_hash, uid);
        }
        ouser = USER_NEXT(user->current_battling_table->battling_user_hash);
    } USER_END;

    /* then we step into battling state finally */
    USER_START(user->current_battling_table->battling_user_hash, ouser) {
        err = bang_state_transit_battling(info, ouser);
        if (err != STATUS_OK) {
            return nerr_pass(err);
        }

        ouser = USER_NEXT(user->current_battling_table->battling_user_hash);
    } USER_END;

    return STATUS_OK;
}

static NEOERR* cmd_turn(struct bang_entry *e, QueueEntry *q)
{
    char     *uid;
    BangUser *user;
    BangUser *ouser;

    BangInfo *info = m_bang;
    NEOERR   *err;

    char          *redir;
    unsigned char *redirbuf = NULL;
    size_t         redirsize = 0;
    HDF           *redirnode;

    BASE_GET_UID(q, uid);
    REQ_GET_PARAM_STR(q->hdfrcv, "redirection", redir);
    user = hash_lookup(info->inherited_info->userh, uid);

    mtc_dbg("%s turns to %s", uid, redir);

    hdf_set_value(q->hdfrcv, PRE_OUTPUT".userid", uid);
    hdf_set_value(q->hdfrcv, PRE_OUTPUT".redirection", redir);
    redirnode = hdf_get_obj(q->hdfrcv, PRE_OUTPUT);

    err = base_msg_new("turn", redirnode, &redirbuf, &redirsize);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }

    /* broadcast user redirection to battling user */
    USER_START(user->current_battling_table->battling_user_hash, ouser) {
        /* skip current user or illegal state users */
        if (ouser == user || ouser->state != BANG_STATE_BATTLING) {
            if (ouser->state != BANG_STATE_BATTLING) {
                mtc_err("impossible state error %s: %d",
                        ouser->inherited_user.uid, ouser->state);
            }
            ouser = USER_NEXT(user->current_battling_table->battling_user_hash);
            continue;
        }
        mtc_dbg("need to tell %s", ouser->inherited_user.uid);
        base_msg_send(redirbuf, redirsize, ouser->inherited_user.fd);
        ouser = USER_NEXT(user->current_battling_table->battling_user_hash);
    } USER_END;

    base_msg_free(redirbuf);

    return STATUS_OK;
}

static NEOERR* cmd_accelerate(struct bang_entry *e, QueueEntry *q)
{
    return STATUS_OK;
}

static void bang_process_driver(EventEntry *entry, QueueEntry *q)
{
    struct bang_entry *e = (struct bang_entry*) entry;
    NEOERR *err = NULL;
    int ret;

    struct bang_stats *st = &(e->st);

    st->msg_total++;

    mtc_dbg("process cmd %u", q->operation);

    switch (q->operation) {
        CASE_SYS_CMD(q->operation, q, e->cd, err);

    /* overwrite join and quit commands provided by base module */
    case REQ_CMD_BASE_JOIN:
        err = cmd_join(e, q);
        break;
    case REQ_CMD_BASE_QUIT:
        err = cmd_quit(e, q);
        break;

    case REQ_CMD_BANG_TO_BATTLE:
        err = cmd_to_battle(e, q);
        break;
    case REQ_CMD_BANG_ACCEPT_BATTLE:
        err = cmd_accept_battle(e, q);
        break;

    /* specified commands of bang module */
    case REQ_CMD_BANG_TURN:
        err = cmd_turn(e, q);
        break;
    case REQ_CMD_BANG_ACCELERATE:
        err = cmd_accelerate(e, q);
        break;

    case REQ_CMD_STATS:
        st->msg_stats++;
        err = STATUS_OK;

        hdf_set_int_value(q->hdfsnd, "msg_total",    st->msg_total);
        hdf_set_int_value(q->hdfsnd, "msg_unrec",    st->msg_unrec);
        hdf_set_int_value(q->hdfsnd, "msg_badparam", st->msg_badparam);
        hdf_set_int_value(q->hdfsnd, "msg_stats",    st->msg_stats);
        hdf_set_int_value(q->hdfsnd, "proc_suc",     st->proc_suc);
        hdf_set_int_value(q->hdfsnd, "proc_fai",     st->proc_fai);
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

static void bang_stop_driver(EventEntry *entry)
{
    struct bang_entry *e = (struct bang_entry *) entry;

    cache_free(e->cd);
}

static EventEntry* bang_init_driver(void)
{
    struct bang_entry *e = calloc(1, sizeof(struct bang_entry));
    if (e == NULL) {
        return NULL;
    }
    NEOERR *err;

    e->inherited_entry.name           = (unsigned char *) strdup(PLUGIN_NAME);
    e->inherited_entry.ksize          = strlen(PLUGIN_NAME);
    e->inherited_entry.process_driver = bang_process_driver;
    e->inherited_entry.stop_driver    = bang_stop_driver;

    err = bang_info_init(&m_bang);
    JUMP_NOK(err, error);

    err = base_info_init(&(m_bang->inherited_info));
    JUMP_NOK(err, error);

    e->cd = cache_create(hdf_get_int_value(g_cfg, CONFIG_PATH".numobjs", 1024), 0);
    if (e->cd == NULL) {
        mtc_err("init cache failure");
        goto error;
    }

    return (EventEntry *) e;

error:
    if (e->inherited_entry.name) {
        free(e->inherited_entry.name);
    }
    if (e->cd) {
        cache_free(e->cd);
    }

    free(e);
    return NULL;
}

struct event_driver bang_driver = {
    .name        = (unsigned char*) PLUGIN_NAME,
    .init_driver = bang_init_driver,
};

/* moc_bang.c ends here */
