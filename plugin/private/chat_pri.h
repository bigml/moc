#ifndef __CHAT_PRI_H__
#define __CHAT_PRI_H__

#define PLUGIN_NAME    "chat"
#define CONFIG_PATH    PRE_SERVER"."PLUGIN_NAME

struct chat_stats {
    unsigned long msg_total;
    unsigned long msg_unrec;
    unsigned long msg_badparam;
    unsigned long msg_stats;

    unsigned long proc_suc;
    unsigned long proc_fai;
};

struct chat_entry {
    EventEntry base;
    mdb_conn *db;
    Cache *cd;
    struct chat_stats st;
};

#endif  /* __CHAT_PRI_H__ */
