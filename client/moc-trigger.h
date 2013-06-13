#ifndef __MOC_TRIGGER_H__
#define __MOC_TRIGGER_H__

__BEGIN_DECLS

#define PRE_HTTP        "HTTP"
#define PRE_CGI         "CGI"
#define PRE_COOKIE      "Cookie"
#define PRE_QUERY       "Query"
#define PRE_OUTPUT      "Output"
#define PRE_RESERVE     "Reserve"
#define PRE_TEMP        "Temp"

#define PRE_ERRTRACE    PRE_OUTPUT".errtrace"
#define PRE_ERRMSG      PRE_OUTPUT".errmsg"
#define PRE_ERRCODE     PRE_OUTPUT".errcode"
#define PRE_SUCCESS     PRE_OUTPUT".success"

#define MOC_TRIGGER(module, key, cmd, flags)                            \
    do {                                                                \
        if (PROCESS_NOK(moc_trigger(module, key, cmd, flags))) {        \
            char *msg = hdf_get_value(moc_hdfrcv(module), PRE_ERRMSG, NULL); \
            char *trace = hdf_get_value(moc_hdfrcv(module), PRE_ERRTRACE, NULL); \
            NEOERR *e = nerr_raise(moc_errcode(module), msg);           \
            return nerr_pass_ctx(e, trace);                             \
        }                                                               \
    } while(0)
#define MOC_TRIGGER_VOID(module, key, cmd, flags)                   \
    do {                                                            \
        if (PROCESS_NOK(moc_trigger(module, key, cmd, flags))) {    \
            char *zpa = NULL;                                       \
            hdf_write_string(moc_hdfrcv(module), &zpa);             \
            mtc_err("pro %s %d failure %d %s",                      \
                    module, cmd, moc_errcode(module), zpa);         \
            if (zpa) free(zpa);                                     \
            return;                                                 \
        }                                                           \
    } while(0)
#define MOC_TRIGGER_RET(ret, module, key, cmd, flags)               \
    do {                                                            \
        if (PROCESS_NOK(moc_trigger(module, key, cmd, flags))) {    \
            char *zpa = NULL;                                       \
            hdf_write_string(moc_hdfrcv(module), &zpa);             \
            mtc_err("pro %s %d failure %d %s",                      \
                    module, cmd, moc_errcode(module), zpa);         \
            if (zpa) free(zpa);                                     \
            return ret;                                             \
        }                                                           \
    } while(0)
#define MOC_TRIGGER_NRET(module, key, cmd, flags)                   \
    do {                                                            \
        if (PROCESS_NOK(moc_trigger(module, key, cmd, flags))) {    \
            char *zpa = NULL;                                       \
            hdf_write_string(module->hdfrcv, &zpa);                 \
            mtc_err("pro %s %d failure %d %s",                      \
                    module, cmd, moc_errcode(module), zpa);         \
            if (zpa) free(zpa);                                     \
        }                                                           \
    } while(0)


#define MOC_TRIGGER_R(arg, module, key, cmd, flags)                     \
    do {                                                            \
        if (PROCESS_NOK(moc_trigger_r(arg, module, key, cmd, flags))) { \
            char *msg = hdf_get_value(moc_hdfrcv_r(arg, module), PRE_ERRMSG, NULL); \
            char *trace = hdf_get_value(moc_hdfrcv_r(arg, module), PRE_ERRTRACE, NULL); \
            NEOERR *e = nerr_raise(moc_errcode_r(arg, module), msg); \
            return nerr_pass_ctx(e, trace);                         \
        }                                                           \
    } while(0)
#define MOC_TRIGGER_VOID_R(arg, module, key, cmd, flags)                \
    do {                                                            \
        if (PROCESS_NOK(moc_trigger_r(arg, module, key, cmd, flags))) { \
            char *zpa = NULL;                                       \
            hdf_write_string(moc_hdfrcv_r(arg, module), &zpa);      \
            mtc_err("pro %s %d failure %d %s",                      \
                    module, cmd, moc_errcode_r(arg, module), zpa);  \
            if (zpa) free(zpa);                                     \
            return;                                                 \
        }                                                           \
    } while(0)
#define MOC_TRIGGER_RET_R(ret, arg, module, key, cmd, flags)            \
    do {                                                            \
        if (PROCESS_NOK(moc_trigger_r(arg, module, key, cmd, flags))) { \
            char *zpa = NULL;                                       \
            hdf_write_string(moc_hdfrcv_r(arg, module), &zpa);      \
            mtc_err("pro %s %d failure %d %s",                      \
                    module, cmd, moc_errcode_r(arg, module), zpa);  \
            if (zpa) free(zpa);                                     \
            return ret;                                             \
        }                                                           \
    } while(0)
#define MOC_TRIGGER_NRET_R(module, key, cmd, flags)                     \
    do {                                                        \
        if (PROCESS_NOK(moc_trigger_r(arg, module, key, cmd, flags))) { \
            char *zpa = NULL;                                   \
            hdf_write_string(module->hdfrcv, &zpa);             \
            mtc_err("pro %s %d failure %d %s",                  \
                    module, cmd, moc_errcode_r(arg, module), zpa); \
            if (zpa) free(zpa);                                 \
        }                                                       \
    } while(0)


__END_DECLS
#endif  /* __MOC_TRIGGER_H__ */
