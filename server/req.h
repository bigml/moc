#ifndef __REQ_H__
#define __REQ_H__

#define REQTYPE_TCP 2

#define REQ_MAKESURE_PARAM(hdf, key)                                \
    do {                                                            \
        if (!hdf_get_value(hdf, key, NULL))                         \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
    } while (0)

#define REQ_GET_PARAM_INT(hdf, key, ret)                            \
    do {                                                            \
        if (!hdf_get_value(hdf, key, NULL)) {                       \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
        ret = hdf_get_int_value(hdf, key, 0);                       \
    } while (0)

#define REQ_GET_PARAM_FLOAT(hdf, key, ret)                          \
    do {                                                            \
        if (!hdf_get_value(hdf, key, NULL)) {                       \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
        ret = mcs_get_float_value(hdf, key, 0.0);                   \
    } while (0)

#define REQ_GET_PARAM_LONG(hdf, key, ret)                           \
    do {                                                            \
        if (!hdf_get_value(hdf, key, NULL)) {                       \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
        ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10);     \
    } while (0)

#define REQ_GET_PARAM_STR(hdf, key, ret)                            \
    do {                                                            \
        ret = hdf_get_value(hdf, key, NULL);                        \
        if (!ret || *ret == '\0') {                                 \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
    } while (0)

#define REQ_GET_PARAM_OBJ(hdf, key, ret)                            \
    do {                                                            \
        ret = hdf_get_obj(hdf, key);                                \
        if (!ret) {                                                 \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
    } while (0)

#define REQ_GET_PARAM_CHILD(hdf, key, ret)                          \
    do {                                                            \
        ret = hdf_get_child(hdf, key);                              \
        if (!ret) {                                                 \
            return nerr_raise(REP_ERR_BADPARAM, "need %s", key);    \
        }                                                           \
    } while (0)


#define REQ_FETCH_PARAM_INT(hdf, key, ret)          \
    do {                                            \
        ret = 0;                                    \
        if (hdf_get_value(hdf, key, NULL)) {        \
            ret = hdf_get_int_value(hdf, key, 0);   \
        }                                           \
    } while (0)

#define REQ_FETCH_PARAM_FLOAT(hdf, key, ret)            \
    do {                                                \
        ret = 0.0;                                      \
        if (hdf_get_value(hdf, key, NULL)) {            \
            ret = mcs_get_float_value(hdf, key, 0.0);   \
        }                                               \
    } while (0)

#define REQ_FETCH_PARAM_LONG(hdf, key, ret)                         \
    do {                                                            \
        ret = 0;                                                    \
        if (hdf_get_value(hdf, key, NULL)) {                        \
            ret = strtoul(hdf_get_value(hdf, key, NULL), NULL, 10); \
        }                                                           \
    } while (0)

#define REQ_FETCH_PARAM_STR(hdf, key, ret)                  \
    do {                                                    \
        ret = hdf_get_value(hdf, key, NULL);                \
    } while (0)

#define REQ_FETCH_PARAM_OBJ(hdf, key, ret)      \
    do {                                        \
        ret = hdf_get_obj(hdf, key);            \
    } while (0)


struct req_info {
    /* network information */
    int fd;
    int type;

    struct sockaddr *clisa;
    socklen_t clilen;

    /* operation information */
    uint32_t id;
    uint16_t cmd;
    uint16_t flags;
    const unsigned char *payload;
    size_t psize;
    struct tcp_socket *tcpsock;

    /* operations */
    /* reply_err is depracated */
    void (*reply_mini)(const struct req_info *req, uint32_t reply);
    void (*reply_err)(const struct req_info *req, uint32_t reply);
    void (*reply_long)(const struct req_info *req, uint32_t reply,
            unsigned char *val, size_t vsize);
};

#endif  /* __REQ_H__ */
