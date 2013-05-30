#ifndef __MOC_PLUGIN_H__
#define __MOC_PLUGIN_H__

#include "mheads.h"
#include "lheads.h"

#define CACHE_HDF(hdf, timeout, fmt, ...)                               \
    if ((val = calloc(1, MAX_PACKET_LEN)) == NULL)                      \
        return nerr_raise(REP_ERR_MEM, "alloc mem for cache failure");  \
    vsize = pack_hdf(hdf, val, MAX_PACKET_LEN);                         \
    cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);            \
    free(val);

#define CACHE_SET_INT64(cd, num, timeout, fmt, ...)                 \
    do {                                                            \
        vsize = 24;                                                 \
        val = calloc(1, vsize);                                     \
        snprintf((char*)val, vsize, "%23lld", (long long int)num);  \
        cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);    \
        free(val);                                                  \
    } while (0)

#define CACHE_SET_INT(cd, num, timeout, fmt, ...)                   \
    do {                                                            \
        vsize = 12;                                                 \
        val = calloc(1, vsize);                                     \
        snprintf((char*)val, vsize, "%d", num);                     \
        cache_setf(cd, val, vsize, timeout, fmt, ##__VA_ARGS__);    \
        free(val);                                                  \
    } while (0)

#define TRACE_ERR(q, ret, err)                                          \
    do {                                                                \
        /* trace */                                                     \
        STRING zstra;    string_init(&zstra);                           \
        nerr_error_traceback(err, &zstra);                              \
        mtc_err("pro %u failed %d %s", q->operation, ret, zstra.buf);   \
        /* set to hdfsnd */                                             \
        hdf_set_value(q->hdfsnd, PRE_ERRTRACE, zstra.buf);              \
        string_clear(&zstra);                                           \
        NEOERR *neede = mcs_err_valid(err);                             \
        mcs_set_int_value_with_type(q->hdfsnd, PRE_ERRCODE, neede->error, CNODE_TYPE_INT); \
        hdf_set_value(q->hdfsnd, PRE_ERRMSG, neede->desc);              \
        nerr_ignore(&err);                                              \
    } while (0)

#endif    /* __MOC_PLUGIN_H__ */
