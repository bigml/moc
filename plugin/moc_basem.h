#ifndef __MOC_BASEM_H__
#define __MOC_BASEM_H__

enum {
    REQ_CMD_BASE_JOIN = 1001,
    REQ_CMD_BASE_QUIT
};


#define CASE_BASE_CMD(binfo, q)                 \
    {                                           \
    case REQ_CMD_BASE_JOIN:                     \
        err = base_cmd_join(binfo, q);          \
        break;                                  \
    case REQ_CMD_BASE_QUIT:                     \
        err = base_cmd_quit(binfo, q);          \
        break;                                  \
    }

#define BASE_GET_USER(q, user)                                  \
    do {                                                        \
        if (q->req->tcpsock && q->req->tcpsock->appdata) {      \
            user = (BaseUser*)q->req->tcpsock->appdata;         \
        } else {                                                \
            return nerr_raise(REP_ERR_BADPARAM, "请先登陆");    \
        }                                                       \
    } while (0)
#define BASE_GET_UID(q, uid)                                    \
    do {                                                        \
        if (q->req->tcpsock && q->req->tcpsock->appdata) {      \
            uid = ((BaseUser*)q->req->tcpsock->appdata)->uid;   \
        } else {                                                \
            return nerr_raise(REP_ERR_BADPARAM, "请先登陆");    \
        }                                                       \
    } while (0)

#define BASE_FETCH_UID(q, uid)                                  \
    do {                                                        \
        if (q->req->tcpsock && q->req->tcpsock->appdata) {      \
            uid = ((BaseUser*)q->req->tcpsock->appdata)->uid;   \
        } else {                                                \
            uid = NULL;                                         \
        }                                                       \
    } while (0)

#define USER_START(userh, user)                     \
    {                                               \
    void *t_rsv_s = NULL;                           \
    user = (BaseUser*)hash_next(userh, &t_rsv_s);   \
    while (user)
#define USER_NEXT(userh) (BaseUser*)hash_next(userh, &t_rsv_s);
#define USER_END }

#endif  /* __MOC_BASEM_H__ */
