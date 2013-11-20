#include "moc_plugin.h"
#include "moc_base.h"

#include "bang_pri.h"

NEOERR* bang_info_init(BangInfo **info)
{
    BangInfo *rinfo;

    NEOERR   *err;

    MCS_NOT_NULLA(info);

    rinfo = calloc(1, sizeof(BangInfo));
    if (!rinfo) {
        return nerr_raise(NERR_NOMEM, "error occurs when allocating for bang info");
    }

    rinfo->bang_idle_user_number = 0;
    err = hash_init(&rinfo->bang_idle_user_hash, hash_str_hash, hash_str_comp, NULL);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }

    err = bang_room_init(&rinfo->room);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }

    *info = rinfo;

    return STATUS_OK;
}

void bang_info_destroy(BangInfo *info)
{
    if (!info) {
        return;
    }

    hash_destroy(&info->bang_idle_user_hash);
    base_info_destroy(info->inherited_info);

    free(info);
}

NEOERR* bang_room_init(BangRoom **room)
{
    BangRoom  *rroom;
    BangTable *table;

    MCS_NOT_NULLA(room);

    rroom = calloc(1, sizeof(BangRoom));
    if (!rroom) {
        return nerr_raise(NERR_NOMEM, "error occurs when allocating for bang room");
    }

    rroom->roomid = 1;

    rroom->table = calloc(1, sizeof(BangTable) * BANG_ROOM_TABLE_MAXIMUM);
    if (!rroom->table) {
        return nerr_raise(NERR_NOMEM, "error occurs when allocating for bang tables");
    }

    for (int i = 0; i < BANG_ROOM_TABLE_MAXIMUM; i++) {
        table = rroom->table + i;

        bang_table_init(table);

        table->tableid = i + 1;
        table->room    = rroom;
    }

    *room = rroom;

    return STATUS_OK;
}

void bang_room_destroy(BangRoom *room)
{
    if (!room) {
        return;
    }

    free(room);
}

void bang_table_init(BangTable *table)
{
    if (!table) {
        return;
    }

    table->battling_user_number = 0;
    hash_init(&table->battling_user_hash, hash_str_hash, hash_str_comp, NULL);
}

void bang_table_destroy(BangTable *table)
{
    if (!table) {
        return;
    }

    hash_destroy(&table->battling_user_hash);

    free(table);
}

NEOERR* bang_user_new(BangInfo *info, BangEntry *e,
        QueueEntry *q, BangUser **user)
{
    char     *userid;
    BangUser *ruser;

    NEOERR   *err;

    MCS_NOT_NULLC(info, e, q);

    REQ_GET_PARAM_STR(q->hdfrcv, "userid", userid);

    ruser = calloc(1, sizeof(BangUser));
    if (!ruser) {
        return nerr_raise(NERR_NOMEM, "error occurs when allocating for bang user");
    }
    ruser->info  = info;
    ruser->state = BANG_STATE_IDLE;

    base_user_quit(info->inherited_info, userid, q, bang_user_destroy);
    base_user_new(info->inherited_info, userid, q, (BaseUser*) ruser, bang_user_destroy);

    err = bang_state_transit_idle(info, ruser);
    if(err != STATUS_OK) {
        return nerr_pass(err);
    }

    if (user) {
        *user = ruser;
    }

    return STATUS_OK;
}

void bang_user_destroy(void *arg)
{
    BangUser *user = (BangUser*) arg;
    char     *uid  = user->inherited_user.uid;

    BangInfo *info = user->info;

    /**
     * remove user from possible idle user hash table and all user hash
     * table.
     */
    if (hash_lookup(info->bang_idle_user_hash, uid)) {
        hash_remove(info->bang_idle_user_hash, uid);
        if (info->bang_idle_user_number > 0) {
            info->bang_idle_user_number--;
        }
    }

    hash_remove(info->inherited_info->userh, uid);
    if (info->inherited_info->usernum > 0) {
        info->inherited_info->usernum--;
    }

    hash_remove(user->current_battling_table->battling_user_hash, uid);
    user->current_battling_table->battling_user_number--;

    base_user_destroy(arg);
}

/**
 * app logic
 */
NEOERR* bang_state_transit_idle(BangInfo *info, BangUser *user)
{
    char *uid;

    MCS_NOT_NULLB(info, user);

    uid = user->inherited_user.uid;
    mtc_dbg("user %s %d turns back to idle", uid, user->state);

    /* user battle is idle already, created etc. */
    if (hash_lookup(info->bang_idle_user_hash, uid)) {
        user->state = BANG_STATE_IDLE;
        return STATUS_OK;
    }

    /* insert user to idle user hash table */
    hash_insert(info->bang_idle_user_hash, (void*) strdup(uid), (void*) user);
    info->bang_idle_user_number++;

    user->state = BANG_STATE_IDLE;

    return STATUS_OK;
}

NEOERR* bang_state_transit_to_battle(BangInfo *info, BangUser *user)
{
    // to be continued
    return STATUS_OK;
}

NEOERR* bang_state_transit_receive_battle(BangInfo *info, BangUser *user)
{
    // to be continued
    return STATUS_OK;
}

NEOERR* bang_state_transit_accept_battle(BangInfo *info, BangUser *user)
{
    // to be continued
    return STATUS_OK;
}

NEOERR* bang_state_transit_battling(BangInfo *info, BangUser *user)
{
    unsigned char *msgbuf  = NULL;
    size_t         msgsize = 0;
    HDF           *msgnode;

    char *uid;

    NEOERR *err;

    MCS_NOT_NULLB(info, user);

    uid = user->inherited_user.uid;

    /* we couldn't find current user in idle user hash, it's illegal */
    if (!hash_lookup(info->bang_idle_user_hash, uid)) {
        user->state = BANG_STATE_UNKNOWN;
        return STATUS_OK;
    }

    /* remove current user from idle user hash table when begins to battle */
    hash_remove(info->bang_idle_user_hash, uid);
    if (info->bang_idle_user_number > 0) {
        info->bang_idle_user_number--;
    }

    user->state = BANG_STATE_BATTLING;

    /**
     * server should push message to notify user that table has been
     * selected automatically, then user would to be ready to begin
     * 'BANG' battle.
     */
    hdf_init(&msgnode);
    hdf_set_value(msgnode, "user_has_been_matched", "1");
    hdf_set_int_value(msgnode, "tableid", user->current_battling_table->tableid);
    err = base_msg_new("battlebegin", msgnode, &msgbuf, &msgsize);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }

    err = base_msg_send(msgbuf, msgsize, user->inherited_user.fd);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }

    /**
     * server should push messages to notify other user that user has
     * been joined to battle.
     */

    return STATUS_OK;
}

/**
 * broadcast to battle message to all other users in idle users hash
 * table, then send battle request message to choosen idle users.
 */
NEOERR* bang_broadcast_to_battle_message(BangInfo *info, BangUser *user)
{
    unsigned char *msgbuf  = NULL;
    size_t         msgsize = 0;
    HDF           *msgnode;

    BangUser      *ouser;
    int            noticenum;

    NEOERR        *err;

    MCS_NOT_NULLB(info, user);

    if (user->state != BANG_STATE_IDLE) {
        return nerr_raise(REP_ERR_BANG_ERROR, "%d 状态非空闲，不可接受", user->state);
    }

    BangTable *table = bang_autoselect_table_in_room(info->room);
    user->current_battling_table = table;
    hash_insert(table->battling_user_hash, (void*) user->inherited_user.uid, (void*) user);
    user->current_battling_table->battling_user_number++;

    /**
     * Next we should check whether the table is not empty before
     * autoselected, if it does, transit user state to 'battling' than
     * broadcasting 'to battle' message to idle users.
     */
    if (table->battling_user_number > 1 && table->battling_user_number <= 4) {
        err = bang_state_transit_battling(info, user);
        if (err != STATUS_OK) {
            return nerr_pass(err);
        }
        return STATUS_OK;
    }

    /**
     * broadcast messages to users
     */
    if (info->bang_idle_user_number < BANG_TABLE_USER_MAXIMUM) {
        noticenum = info->bang_idle_user_number;
    } else {
        noticenum = BANG_TABLE_USER_MAXIMUM - 1;
    }

    if (noticenum <= 1) {
        mtc_dbg("no other idle user now");
        user->state = BANG_STATE_SINGLE_BATTLE;
        return STATUS_OK;
    }

    mtc_dbg("notice %d of %d idle user", info->bang_idle_user_number, noticenum);

    hdf_init(&msgnode);
    hdf_set_int_value(msgnode, "tableid", user->current_battling_table->tableid);
    err = base_msg_new("battleinvite", msgnode, &msgbuf, &msgsize);
    if (err != STATUS_OK) {
        return nerr_pass(err);
    }
    hdf_destroy(&msgnode);

    USER_START(info->bang_idle_user_hash, ouser) {
        /* skip current user or illegal state users */
        if (ouser == user || ouser->state != BANG_STATE_IDLE) {
            if (ouser->state != BANG_STATE_IDLE) {
                mtc_err("impossible state error %s: %d",
                        ouser->inherited_user.uid, ouser->state);
            }
            ouser = USER_NEXT(info->bang_idle_user_hash);
            continue;
        }
        if (user->current_battling_table->battling_user_number > noticenum) break;

        /* Finally, we find an idle user */
        ouser->current_battling_table = table;
        hash_insert(user->current_battling_table->battling_user_hash,
                (void*) strdup(ouser->inherited_user.uid), (void*) ouser);
        user->current_battling_table->battling_user_number++;

        /* additional operation to matched other idle user */
        /**
         * XXX server whether or not remove received battle request user
         *     from idle user hash table?
         */
        ouser->state = BANG_STATE_RECEIVE_BATTLE;
        /**
         * Then, we send battle request message to this user, wait for
         * its response(receive, reject or timeout).
         */
        err = base_msg_send(msgbuf, msgsize, ouser->inherited_user.fd);
        TRACE_NOK(err);

        /* let's track to next idle user */
        ouser = USER_NEXT(info->bang_idle_user_hash);
    } USER_END;

    base_msg_free(msgbuf);

    user->state = BANG_STATE_TO_BATTLE; /* now set user state as 'to battle' */

    return STATUS_OK;
}

BangTable* bang_autoselect_table_global(BangInfo *info)
{
    BangRoom  *room;
    BangTable *table;

    for (int i = 0; i < BANG_SERVER_ROMM_MAXIMUM; i++) {
        room = info->room + i;
        if (room) {
            table = bang_autoselect_table_in_room(room);
            if (table) {
                return table;
            } else {
                mtc_dbg("all tables in room %d are full currently.", i + 1);
            }
        }
    }

    if (!table) {
        mtc_dbg("all rooms are extremely full currently.");
    }

    return NULL;
}

BangTable* bang_autoselect_table_in_room(BangRoom *room)
{
    /**
     * Frist, we iterate all available tables of current joined room, if
     * found, server would push specific table info to user.
     */
    BangTable *table;
    for (int i = 0; i < BANG_ROOM_TABLE_MAXIMUM; i++) {
        table = room->table + i;
        if (table) {
            if (table->battling_user_number >= 1 && table->battling_user_number <= BANG_TABLE_USER_MAXIMUM - 1) {
                mtc_dbg("auto matched table id is %d.", table->tableid);
                return table;
            }
        }
    }

    /**
     * If we could not find any available tables(all tables is full or
     * empty), then server should invite online idle user to join to
     * battle with requested user.
     */
    mtc_dbg("no available table found, server would invite online idle user to battle.");

    /**
     * We have to iterate all tables to find an empty table to join.
     */
    for (int i = 0; i < BANG_ROOM_TABLE_MAXIMUM; i++) {
        table = room->table + i;
        if (table) {
            if (table->battling_user_number == 0) {
                return table;
            } else if (i == BANG_ROOM_TABLE_MAXIMUM) {
                mtc_dbg("all tables are full currently.");
            }
        }
    }

    return NULL;
}

NEOERR* bang_push_begin_battle_message(BangInfo *info, BangUser *user)
{
    // to be continued
    return STATUS_OK;
}

/* bang_internal.c ends here */
