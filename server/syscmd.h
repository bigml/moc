#ifndef __SYSCMD_H__
#define __SYSCMD_H__

/* Statistics structure */
struct stats {
    unsigned long msg_tipc;
    unsigned long msg_tcp;
    unsigned long msg_udp;
    unsigned long msg_sctp;

    unsigned long net_version_mismatch; /* 5 */
    unsigned long net_broken_req;
    unsigned long net_unk_req;

    unsigned long pro_busy;
};

#define STATS_REPLY_SIZE 8
#define VNAME_CACHE_KEY    "cachekey"         /* DATA_TYPE_STRING */
#define VNAME_CACHE_VAL    "cacheval"      /* DATA_TYPE_ANY */

#define CASE_SYS_CMD(cmd, q, cd, err)               \
    {                                               \
    case REQ_CMD_CACHE_GET:                         \
        err = sys_cmd_cache_get(q, cd, false);      \
        break;                                      \
    case REQ_CMD_CACHE_SET:                         \
        err = sys_cmd_cache_set(q, cd, false);      \
        break;                                      \
    case REQ_CMD_CACHE_DEL:                         \
        err = sys_cmd_cache_del(q, cd, false);      \
        break;                                      \
    case REQ_CMD_CACHE_EMPTY:                       \
        err = sys_cmd_cache_empty(q, &cd, false);   \
        break;                                      \
    }
        
void sys_stats_init(struct stats *s);
int  reply_trigger(struct queue_entry *q, uint32_t reply);

NEOERR* sys_cmd_cache_get(struct queue_entry *q, struct cache *cd, bool reply);
NEOERR* sys_cmd_cache_set(struct queue_entry *q, struct cache *cd, bool reply);
NEOERR* sys_cmd_cache_del(struct queue_entry *q, struct cache *cd, bool reply);
NEOERR* sys_cmd_cache_empty(struct queue_entry *q, struct cache **cd, bool reply);

#endif  /* __SYS_CMD_H__ */
