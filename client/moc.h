/*
 * moc client
 * client should runable on Linux, Mac, Android, ios...
 * so, make it as clean as possible
 */

#ifndef __MOC_H__
#define __MOC_H__

#include <stdlib.h>        /* malloc() */
#include <unistd.h>        /* close() */
#include <stdint.h>        /* uint32_t and friends */
#include <stdbool.h>       /* bool, true, false */
#include <string.h>        /* memcpy() */
#include <time.h>
#include <netdb.h>         /* gethostbyname() */
#include <fcntl.h>
#include <errno.h>
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#include <crt_externs.h>
#define environ (*_NSGetEnviron())
#endif

#include <sys/types.h>     /* socket defines */
#include <sys/socket.h>

#include <arpa/inet.h>      /* htonls() and friends */
#include <netinet/tcp.h>    /* TCP stuff */

#include "ClearSilver.h"
#include "mtrace.h"          /* trace */
#include "moc-private.h"     /* client&server's public lib */
#include "internal.h"        /* client internal */
#include "tcp.h"

#ifdef EVENTLOOP
#include <pthread.h>        /* for pthread_t */
#include <sys/select.h>     /* select() */
#include "eloop.h"
#include "mcbk.h"
#include "mscli.h"          /* process moc server to client stuff */
#endif


__BEGIN_DECLS

#define MOC_CONFIG_FILE        "mocclient.hdf"

/*
 * 初始化
 * 该函数会从配置文件中读取所所有服务器列表, 初始化
 * path 软件运行时的绝对路径
 */
NEOERR* moc_init(char *path);

/*
 * 销毁
 */
void moc_destroy();

/*
 * 设置请求参数（方法一）
 * 返回请求参数数据HDF
 * 可用来设置请求参数
 */
HDF* moc_hdfsnd(char *module);

/*
 * 设置请求参数（方法二）
 * module: 业务模块名
 */
NEOERR* moc_set_param(char *module, char *key, char *val);
NEOERR* moc_set_param_int(char *module, char *key, int val);
NEOERR* moc_set_param_uint(char *module, char *key, unsigned int val);
NEOERR* moc_set_param_int64(char *module, char *key, int64_t val);
NEOERR* moc_set_param_float(char *module, char *key, float val);

/*
 * 触发请求
 * 因为需要支持设置不同参数的循环使用，故每次trigger时会清空对应 hdfsnd 中的数据.
 * 不可以同一次参数设置循环调用
 * module: 业务模块名
 * key: 用来选择处理后端的关键字（如UIN等），提供的话可以有效避免缓存冗余，可以为NULL
 * cmd: 命令号，不可重复使用，必填
 * flags: 请求标志，不可重复使用，必填
 * 返回值为该操作返回码, 分为三段区间, 取值范围参考 moc-private.h 中 REP_xxx
 * 如果服务业务端有其他数据返回时, 返回数据存储在 evt->rcvdata 中
 */
int moc_trigger(char *module, char *key, unsigned short cmd, unsigned short flags);

/*
 * 返回收到的数据HDF
 */
HDF* moc_hdfrcv(char *module);

/*
 * 绑定回调函数
 * 针对服务器主动发起的命令，绑定对应的回调函数
 */
NEOERR* moc_regist_callback(char *module, char *cmd, MocCallback cmdcbk);


__END_DECLS
#endif    /* __MOC_H__ */
