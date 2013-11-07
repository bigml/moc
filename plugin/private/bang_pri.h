#ifndef __BANG_PRI_H__
#define __BANG_PRI_H__

#define PLUGIN_NAME "bang"
#define CONFIG_PATH "Plugin."PLUGIN_NAME

#define PREFIX_BANG "Bang"
#define IDLE_USER_NUMBER_MAX 5

#define BANG_SERVER_ROMM_MAXIMUM 25
#define BANG_ROOM_USER_MAXIMUM   100
#define BANG_ROOM_TABLE_MAXIMUM  25
#define BANG_TABLE_USER_MAXIMUM  100 / 25

/* should be defined in tazai.h */
#define REP_ERR_BANG_ERROR 132

struct bang_stats {
    unsigned long msg_total;
    unsigned long msg_unrec;
    unsigned long msg_badparam;
    unsigned long msg_stats;

    unsigned long proc_suc;
    unsigned long proc_fai;
};
typedef struct bang_stats BANG_STATS;

struct bang_entry {
    EventEntry   inherited_entry;
    mmg_conn    *db;
    Cache       *cd;
    struct bang_stats st;
};
typedef struct bang_entry BangEntry;

struct bang_info {
    struct base_info *inherited_info;

    struct bang_room *room;

    int   bang_idle_user_number;
    HASH *bang_idle_user_hash;
};
typedef struct bang_info BangInfo;

struct bang_room {
    int    roomid;
    struct bang_table *table;
};
typedef struct bang_room BangRoom;

struct bang_table {
    struct bang_room *room;
    int   tableid;

    int   battling_user_number;
    HASH *battling_user_hash;
};
typedef struct bang_table BangTable;

struct bang_user {
    BaseUser inherited_user;

    struct   bang_info  *info;
    struct   bang_table *current_battling_table;
    int      state;
};
typedef struct bang_user BangUser;

/**
 * possible states of bang.
 */
typedef enum bang_battle_state {
    BANG_STATE_IDLE,           /**< user ready to battle */
    BANG_STATE_TO_BATTLE,      /**< user send battle request to sever */
    BANG_STATE_RECEIVE_BATTLE, /**< user receive battle request */
    BANG_STATE_ACCEPT_BATTLE,  /**< user accept battle request */
    BANG_STATE_SINGLE_BATTLE,  /**< user just battle with battle in single mode */
    BANG_STATE_BATTLING,       /**< user is battling! */
    BANG_STATE_UNKNOWN,        /**< unknown state */
} BANG_BATTLE_STATE;

/**
 * internal used functions declaration.
 */
NEOERR* bang_info_init(BangInfo **info);
void    bang_info_destroy(BangInfo *info);

NEOERR* bang_room_init(BangRoom **room);
void    bang_room_destroy(BangRoom *room);

void bang_table_init(BangTable *table);
void bang_table_destroy(BangTable *table);

NEOERR* bang_user_new(BangInfo *info, BangEntry *e,
        QueueEntry *q, BangUser **user);
void    bang_user_destroy(void *arg);

/**
 * states transtion functions
 */
typedef NEOERR* (*bang_state_transit)(BangInfo *info, BangUser *user);
typedef bang_state_transit BANG_STATE_TRANSIT;

NEOERR* bang_state_transit_idle(BangInfo *info, BangUser *user);
NEOERR* bang_state_transit_to_battle(BangInfo *info, BangUser *user);
NEOERR* bang_state_transit_receive_battle(BangInfo *info, BangUser *user);
NEOERR* bang_state_transit_accept_battle(BangInfo *info, BangUser *user);
NEOERR* bang_state_transit_battling(BangInfo *info, BangUser *user);

NEOERR* bang_broadcast_to_battle_message(BangInfo *info, BangUser *user);
NEOERR* bang_invite_to_battle(BangInfo *info, BangUser *user);
NEOERR* bang_push_begin_battle_message(BangInfo *info, BangUser *user);

BangTable* bang_autoselect_table_global(BangInfo *info);
BangTable* bang_autoselect_table_in_room(BangRoom *room);

/**
 * callbacks
 */

/**
 * timers
 */
void timerup_bang(struct event_entry *e, unsigned int upsec, void *data);

#endif

/* bang_internal.c ends here */
