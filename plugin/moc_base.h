#ifndef __MOC_BASE_H__
#define __MOC_BASE_H__

#include "moc_basem.h"

struct base_info {
    int usernum;
    HASH *userh;
};
typedef struct base_info BaseInfo;

struct base_user {
    char *uid;
    char ip[INET6_ADDRSTRLEN];
    int port;
    int fd;
    /*
     * 我们保存tcpsock在此的原因在于要设置其appdata 和 on_close，
     * 好让主线程能在客户端掉线时destroy掉用户(只有请求过1001的连接才会设置这些信息)。
     * 也不排除tcpsock今后有其他用处
     */
    struct tcp_socket *tcpsock;
    struct base_info *baseinfo;
};
typedef struct base_user BaseUser;

NEOERR* base_info_init(BaseInfo **binfo);
void base_info_destroy(BaseInfo *binfo);
struct base_user *base_user_find(BaseInfo *binfo, char *uid);
struct base_user *base_user_new(BaseInfo *binfo, char *uid, QueueEntry *q,
                                BaseUser *ruser, void (*user_destroy)(void *arg));
bool base_user_quit(BaseInfo *binfo, char *uid,
                    QueueEntry *q, void (*user_destroy)(void *arg));
void base_user_destroy(void *arg);

/*
 * alloc & decallc message one time, and can be reply to many users
 */
NEOERR* base_msg_new(char *cmd, HDF *datanode, unsigned char **buf, size_t *size);
NEOERR* base_msg_send(unsigned char *buf, size_t size, int fd);
void base_msg_free(unsigned char *buf);

/*
 * reply a message to only one user
 */
NEOERR* base_msg_touser(char *cmd, HDF *datanode, int fd);

/*
 * pubic logic function
 */
NEOERR* base_cmd_join(struct base_info *binfo, QueueEntry *q);
NEOERR* base_cmd_quit(struct base_info *binfo, QueueEntry *q);

#endif    /* __MOC_BASE_H__ */
