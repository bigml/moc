#ifndef __BASE_PRI_H__
#define __BASE_PRI_H__

#define PLUGIN_NAME    "base"
#define CONFIG_PATH    PRE_SERVER"."PLUGIN_NAME

struct base_stats {
    unsigned long msg_total;
    unsigned long msg_unrec;
    unsigned long msg_badparam;
    unsigned long msg_stats;

    unsigned long proc_suc;
    unsigned long proc_fai;
};

struct base_entry {
    EventEntry base;
    mdb_conn *db;
    Cache *cd;
    struct base_stats st;
};

#endif  /* __BASE_PRI_H__ */
