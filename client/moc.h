/*
 * moc client
 * client should runable on Linux, Mac, Android, ios...
 * so, make it as clean as possible
 *
 * API description
 * ===============
 * moc client API assembled by:
 *     moc_init[_xxx], moc_destroy
 *     moc_set_param[_xxx]
 *     moc_hdfsnd, moc_hdfrcv
 *     moc_trigger
 *     moc_regist_callback
 * typical application scenarios is:
 *     init, regist callback, set parameter, and trigger
 *
 * the API have two sets for the following situations:
 *     set one: easy to use set
 *              these functions hold a static variable for internal use;
 *              they CAN create message_receive_and_fire_callback threads.
 *
 *              but, can't be used on multi_thread_use enviorment, --e.g. you
 *              can't call moc_trigger() on multi thread, especially trigger on
 *              the same backend modlue on multi thread.
 *
 *     set two: thread safe set
 *              these functions need application store and pass the moc_arg parameter;
 *              they CAN'T create message_receive_and_fire_callback threads.
 *
 *              so, these functions just trigger and wait response,
 *              can't hold the server_push_message.
 *
 *              but, they can be used on multi_thread_use enviorment(because moc_arg
 *              stored and passed by caller). you can trigger to many many server and
 *              wait for trustable response by many many thread if you want.
 * these two sets functions can be used mixed.
 * in other words:
 *     hold message_receive_and_fire_callback on main thread;
 *     trigger and wait response on other threadSSS.
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
#define MSG_NOSIGNAL SO_NOSIGPIPE 
#endif

#include <sys/types.h>     /* socket defines */
#include <sys/socket.h>
#include <arpa/inet.h>      /* htonls() and friends */
#include <netinet/tcp.h>    /* TCP stuff */

#ifdef EVENTLOOP
#include <pthread.h>        /* for pthread_t */
#include <sys/select.h>     /* select() */
#endif

#include "clearsilver/ClearSilver.h"
#include "mtrace.h"          /* trace */
#include "moc-private.h"     /* client&server's public lib */
#include "internal.h"        /* client internal */
#include "tcp.h"
#include "moc-trigger.h"

#ifdef EVENTLOOP
#include "eloop.h"
#include "mcbk.h"
#include "mscli.h"          /* process moc server to client stuff */
#endif


__BEGIN_DECLS

#define MOC_CONFIG_FILE        "mocclient.hdf"

/*
 * easy to use set
 * ===============
 */

/*
 * 初始化（从指定配置文件目录）
 * 该函数会从配置文件中读取所所有服务器列表, 初始化
 * path mocclient.hdf 文件放置的路径（一般为软件运行时的绝对路径）
 */
NEOERR* moc_init(char *path);

/*
 * 初始化（从配置文件HDF）
 * used by server only currently, same as moc_init() without:
 * 1. config file load and parse
 * 2. trace init
 * 3. lerr_init()
 */
NEOERR* moc_init_fromhdf(HDF *node, char *path);

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
 * 不建议在多线程环境中并行调用该函数（特别是多线程中对同一个业务模块的并行触发）
 * 如有并行调用需求，请使用后面的线程安全版本 moc_trigger_r()。
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
 * 返回对应模块的错误吗
 */
int moc_errcode(char *module);

/*
 * 绑定回调函数
 * 针对服务器主动发起的命令，绑定对应的回调函数
 */
NEOERR* moc_regist_callback(char *module, char *cmd, MocCallback cmdcbk);




/*
 * thread safe set
 * ===============
 * moc_xxx_r() are the thread safe version of moc_xxx()
 * application should care about moc_arg *arg
 *     store it on moc_init_r() return;
 *     pass them on others moc_xxx_r().
 * These functions without EVENTLOOP support, just trigger, and wait for response.
 */
NEOERR* moc_init_r(char *path, moc_arg **arg);
NEOERR* moc_init_fromhdf_r(HDF *node, moc_arg **arg);
void moc_destroy_r(moc_arg *arg);

HDF* moc_hdfsnd_r(moc_arg *arg, char *module);

NEOERR* moc_set_param_r(moc_arg *arg, char *module, char *key, char *val);
NEOERR* moc_set_param_int_r(moc_arg *arg, char *module, char *key, int val);
NEOERR* moc_set_param_uint_r(moc_arg *arg, char *module, char *key, unsigned int val);
NEOERR* moc_set_param_int64_r(moc_arg *arg, char *module, char *key, int64_t val);
NEOERR* moc_set_param_float_r(moc_arg *arg, char *module, char *key, float val);

int moc_trigger_r(moc_arg *arg, char *module, char *key, unsigned short cmd,
                  unsigned short flags);

HDF* moc_hdfrcv_r(moc_arg *arg, char *module);
int  moc_errcode_r(moc_arg *arg, char *module);


__END_DECLS
#endif    /* __MOC_H__ */
