#include "moc-private.h"

/*
 * moc system error
 */
int REP_ERR = 0;                /* 14 */
int REP_ERR_VER = 0;            /* 15 */
int REP_ERR_SEND = 0;           /* 16 */
int REP_ERR_BROKEN = 0;         /* 17 */
int REP_ERR_UNKREQ = 0;         /* 18 */
int REP_ERR_MEM = 0;            /* 19 */
int REP_ERR_DB = 0;             /* 20 */
int REP_ERR_BUSY = 0;           /* 21 */
int REP_ERR_PACK = 0;           /* 22 */
int REP_ERR_BADPARAM = 0;       /* 23 */
int REP_ERR_CACHE_MISS = 0;     /* 24 */

NEOERR* lerr_init()
{
    NEOERR *err;
    static int lerrInited = 0;

    if (lerrInited == 0) {
        err = nerr_init();
        if (err != STATUS_OK) return nerr_pass(err);

        err = nerr_register(&REP_ERR, "后台处理失败");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_VER, "通信协议版本不对");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_SEND, "后台处理发送失败");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_BROKEN, "后台网络包丢失");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_UNKREQ, "事件后台无效");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_MEM, "后台内存错误");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_DB, "后台数据库错误");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_BUSY, "后台繁忙");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_PACK, "后台打包失败");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_BADPARAM, "后台参数错误");
        if (err != STATUS_OK) return nerr_pass(err);
        err = nerr_register(&REP_ERR_CACHE_MISS, "后台缓存获取失败");
        if (err != STATUS_OK) return nerr_pass(err);

        lerrInited = 1;
    }

    return STATUS_OK;
}
