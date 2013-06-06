#include "moc.h"

unsigned int g_reqid = 0;
static moc_arg *m_arg = NULL;

/*
 * local
 * =====
 */
static HDF* _moc_get_confhdf(char *path)
{
    HDF *cfg;
    char fpath[_POSIX_PATH_MAX], fname[_POSIX_PATH_MAX], *sfoo;
    NEOERR *err;

    /*
     * config file
     */
    if (path) strncpy(fpath, path, sizeof(fpath));
    else getcwd(fpath, sizeof(fpath));
    snprintf(fname, sizeof(fname), "%s/%s", fpath, MOC_CONFIG_FILE);
    hdf_init(&cfg);
    err = hdf_read_file(cfg, fname);
    if (err != STATUS_OK) {
        nerr_ignore(&err);
        return NULL;
    }

    /*
     * log file
     */
    snprintf(fname, sizeof(fname), "%s/mocclient", fpath);
    sfoo = hdf_get_value(cfg, "Config.logfile", fname);
    mtc_init(sfoo, hdf_get_int_value(cfg, "Config.trace_level", TC_DEFAULT_LEVEL));
    
    lerr_init();

    return hdf_get_obj(cfg, "modules");

    /* TODO cfg memory leak */
}

static NEOERR* _moc_load_fromhdf(HDF *pnode, HASH *evth)
{
    HDF *node, *cnode;

    MOC_NOT_NULLB(pnode, evth);
    
    node = hdf_obj_child(pnode);
    while (node) {
        /*
         *per backend module
         */
        moc_t *evt;
        char *mname;

        mname = hdf_obj_name(node);

        mtc_dbg("init event %s", mname);
        
        evt = calloc(1, sizeof(moc_t));
        if (!evt) return nerr_raise(NERR_NOMEM, "memory gone");

        evt->ename = strdup(mname);

        hdf_init(&evt->hdfrcv);
        hdf_init(&evt->hdfsnd);
        evt->rcvbuf = calloc(1, MAX_PACKET_LEN);
        evt->payload = calloc(1, MAX_PACKET_LEN);
        if (!evt->payload || !evt->rcvbuf) return nerr_raise(NERR_NOMEM, "memory gone");
        
        cnode = hdf_obj_child(node);
        while (cnode) {
            /*
             * per server
             */
            struct timeval tv;
            char *ip = hdf_get_value(cnode, "ip", "127.0.0.1");
            int port = hdf_get_int_value(cnode, "port", 5000);
            int nblk = hdf_get_int_value(cnode, "non_block", 0);
            tv.tv_sec = hdf_get_int_value(cnode, "timeout_s", 0);
            tv.tv_usec = hdf_get_int_value(cnode, "timeout_u", 0);

            if (moc_add_tcp_server(evt, ip, port, nblk, &tv)) {
                mtc_dbg("%s add server %s %d ok", mname, ip, port);
            } else {
                mtc_dbg("%s add server %s %d failure", mname, ip, port);
            }

            cnode = hdf_obj_next(cnode);
        }

        if (evt->nservers) hash_insert(evth, (void*)strdup(mname), (void*)evt);

        node = hdf_obj_next(node);
    }

    return STATUS_OK;
}

static void _moc_destroy(moc_arg *arg)
{
    char *key = NULL;

    if (!arg) return;

    HASH *table = arg->evth;
    moc_t *evt = (moc_t*)hash_next(table, (void**)&key);
    while (evt != NULL) {
        /* TODO moc_free */
        //moc_free(evt);
        evt = hash_next(table, (void**)&key);
    }

#ifdef EVENTLOOP
    key = NULL;
    table = arg->cbkh;
    struct moc_cbk *c = hash_next(table, (void**)&key);
    while (c) {
        mcbk_destroy(c);
        
        c = hash_next(table, (void**)&key);
    }

    eloop_stop(arg);
    mcbk_stop(arg);
#endif

    mocarg_destroy(arg);
}

static HDF* _moc_hdfsnd(moc_arg *arg, char *module)
{
    if (!arg || !module) return NULL;

    HASH *evth = arg->evth;
    
    moc_t *evt = hash_lookup(evth, module);
    if (!evt) return NULL;

    return evt->hdfsnd;
}

static NEOERR* _moc_set_param(moc_arg *arg, char *module, char *key, char *val)
{
    MOC_NOT_NULLB(arg, arg->evth);
    MOC_NOT_NULLC(module, key, val);

    HASH *evth = arg->evth;
    
    moc_t *evt = hash_lookup(evth, module);
    MOC_NOT_NULLA(evt);

    hdf_set_value(evt->hdfsnd, key, val);

    return STATUS_OK;
}

static NEOERR* _moc_set_param_int(moc_arg *arg, char *module, char *key, int val)
{
    MOC_NOT_NULLB(arg, arg->evth);
    MOC_NOT_NULLC(module, key, val);

    HASH *evth = arg->evth;
    
    moc_t *evt = hash_lookup(evth, module);
    MOC_NOT_NULLA(evt);

    hdf_set_int_value(evt->hdfsnd, key, val);

    return STATUS_OK;
}

static NEOERR* _moc_set_param_uint(moc_arg *arg, char *module, char *key,
                                   unsigned int val)
{
    MOC_NOT_NULLB(arg, arg->evth);
    MOC_NOT_NULLC(module, key, val);

    HASH *evth = arg->evth;
    
    moc_t *evt = hash_lookup(evth, module);
    MOC_NOT_NULLA(evt);

    char buf[64];
    snprintf(buf, sizeof(buf), "%u", val);

    hdf_set_value(evt->hdfsnd, key, buf);

    return STATUS_OK;
}

static NEOERR* _moc_set_param_int64(moc_arg *arg, char *module, char *key, int64_t val)
{
    MOC_NOT_NULLB(arg, arg->evth);
    MOC_NOT_NULLC(module, key, val);

    HASH *evth = arg->evth;
    
    moc_t *evt = hash_lookup(evth, module);
    MOC_NOT_NULLA(evt);

    char buf[64];
    snprintf(buf, sizeof(buf), "%ld", val);

    hdf_set_value(evt->hdfsnd, key, buf);

    return STATUS_OK;
}

static NEOERR* _moc_set_param_float(moc_arg *arg, char *module, char *key, float val)
{
    MOC_NOT_NULLB(arg, arg->evth);
    MOC_NOT_NULLC(module, key, val);

    HASH *evth = arg->evth;
    
    moc_t *evt = hash_lookup(evth, module);
    MOC_NOT_NULLA(evt);

    char buf[64];
    snprintf(buf, sizeof(buf), "%f", val);

    hdf_set_value(evt->hdfsnd, key, buf);

    return STATUS_OK;
}

static int _moc_trigger(moc_arg *arg, char *module, char *key, unsigned short cmd,
                        unsigned short flags)
{
    size_t t, ksize, vsize;
    moc_srv *srv;
    unsigned char *p;
    moc_t *evt;
    uint32_t rv = REP_OK;

    if (!arg || !module) return REP_ERR;

    HASH *evth = arg->evth;
    
    evt = hash_lookup(evth, module);
    if (!evt) {
        mtc_err("can't found %s module", module);
        return REP_ERR;
    }

    evt->cmd = cmd;
    evt->flags = flags;
    ksize = strlen(module);

    if (key) {
        srv = select_srv(evt, key, strlen(key));
    } else {
        srv = &(evt->servers[0]);
    }

    if (g_reqid++ > 0x0FFFFFFC) {
        g_reqid = 1;
    }

    p = evt->payload + TCP_MSG_OFFSET;
    * (uint32_t *) p = htonl( (PROTO_VER << 28) | g_reqid );
    * ((uint16_t *) p + 2) = htons(cmd);
    * ((uint16_t *) p + 3) = htons(flags);
    * ((uint32_t *) p + 2) = htonl(ksize);
    memcpy(p+12, evt->ename, ksize);

    evt->psize = TCP_MSG_OFFSET + 12 + ksize;
    
    /*
     * don't escape the hdf because some body need set ' in param 
     */
    vsize = pack_hdf(evt->hdfsnd, evt->payload + evt->psize, MAX_PACKET_LEN);
    evt->psize += vsize;

    if (evt->psize < 17) {
        * (uint32_t *) (evt->payload+evt->psize) = htonl(DATA_TYPE_EOF);
        evt->psize += sizeof(uint32_t);
    }
    
    t = tcp_srv_send(srv, evt->payload, evt->psize);
    if (t <= 0) {
        evt->errcode = REP_ERR_SEND;
        return REP_ERR_SEND;
    }

    /*
     * application should make sure call moc_trigger() in turn
     * or, the evt->hdfrcv is untrustable
     */
    hdf_destroy(&evt->hdfsnd);
    hdf_init(&evt->hdfsnd);
    hdf_destroy(&evt->hdfrcv);
    hdf_init(&evt->hdfrcv);

#ifdef EVENTLOOP
    if (!(flags & FLAGS_SYNC)) return REP_OK;
    
    struct timespec ts;
    mutil_utc_time(&ts);
    ts.tv_sec += srv->tv.tv_sec + 1;
//    if(srv->tv.tv_usec > 1000000) srv->tv.tv_usec = 900000;
//    ts.tv_nsec += srv->tv.tv_usec * 1000;
    
    mssync_lock(&(arg->mainsync));
    int ret = mssync_timedwait(&(arg->mainsync), &ts);
    if (ret != 0 && ret != ETIMEDOUT) {
        mtc_err("Error in timedwait() %d", ret);
        return REP_ERR;
    }
    mssync_unlock(&(arg->mainsync));

    return REP_OK;
#else
    
    if (flags & FLAGS_SYNC) {
        vsize = 0;
        rv = tcp_get_rep(srv, evt->rcvbuf, MAX_PACKET_LEN, &p, &vsize);
        if (rv == -1) rv = REP_ERR;
        evt->errcode = rv;

        if (vsize > 8) {
            /* reply_long add a vsize parameter */
            unpack_hdf(p+4, vsize-4, &evt->hdfrcv);
        }
    }
#endif
    
    return rv;
}

HDF* _moc_hdfrcv(moc_arg *arg, char *module)
{
    if (!arg || !module) return NULL;

    HASH *evth = arg->evth;
    
    moc_t *evt = hash_lookup(evth, module);
    if (!evt) return NULL;

    return evt->hdfrcv;
}

static NEOERR* _moc_regist_callback(moc_arg *arg, char *module, char *cmd,
                                    MocCallback cmdcbk)
{
    MOC_NOT_NULLB(arg, arg->cbkh);
    MOC_NOT_NULLC(module, cmd, cmdcbk);
    
#ifdef EVENTLOOP
    struct moc_cbk *c = mcbk_create();
    if (!c) return nerr_raise(NERR_NOMEM, "alloc cbk");
    c->ename = strdup(module);
    c->cmd = strdup(cmd);
    c->callback = cmdcbk;

    mcbk_regist(arg->cbkh, module, cmd, c);
    
#else
    mtc_foo("can't regist callback without EVENTLOOP");
#endif
    return STATUS_OK;
}


/*
 * easy to use version
 * ===================
 */
NEOERR* moc_init(char *path)
{
    HDF *node;
    NEOERR *err;
    
    if (m_arg) return nerr_raise(NERR_ASSERT, "moc inited already");
    
    m_arg = mocarg_init();
    if (!m_arg) return nerr_raise(NERR_NOMEM, "alloc moc arg");

    node = _moc_get_confhdf(path);
    if (!node) return nerr_raise(NERR_ASSERT, "config error");
    err = _moc_load_fromhdf(node, m_arg->evth);
    if (err != STATUS_OK) return nerr_pass(err);

#ifdef EVENTLOOP
    err = eloop_start(m_arg);
    if (err != STATUS_OK) return nerr_pass(err);

    err = mcbk_start(m_arg);
    if (err != STATUS_OK) return nerr_pass(err);
#endif

    return STATUS_OK;
}

NEOERR* moc_init_fromhdf(HDF *node, char *path)
{
    NEOERR *err;

    if (m_arg) return nerr_raise(NERR_ASSERT, "moc inited already");
    
    m_arg = mocarg_init();
    if (!m_arg) return nerr_raise(NERR_NOMEM, "alloc moc arg");

    err = _moc_load_fromhdf(node, m_arg->evth);
    if (err != STATUS_OK) return nerr_pass(err);

#ifdef EVENTLOOP
    err = eloop_start(m_arg);
    if (err != STATUS_OK) return nerr_pass(err);

    err = mcbk_start(m_arg);
    if (err != STATUS_OK) return nerr_pass(err);
#endif

    return STATUS_OK;
}

void moc_destroy()
{
    _moc_destroy(m_arg);
}

HDF* moc_hdfsnd(char *module)
{
    return _moc_hdfsnd(m_arg, module);
}

NEOERR* moc_set_param(char *module, char *key, char *val)
{
    return nerr_pass(_moc_set_param(m_arg, module, key, val));
}

NEOERR* moc_set_param_int(char *module, char *key, int val)
{
    return nerr_pass(_moc_set_param_int(m_arg, module, key, val));
}

NEOERR* moc_set_param_uint(char *module, char *key, unsigned int val)
{
    return nerr_pass(_moc_set_param_uint(m_arg, module, key, val));
}

NEOERR* moc_set_param_int64(char *module, char *key, int64_t val)
{
    return nerr_pass(_moc_set_param_int64(m_arg, module, key, val));
}
NEOERR* moc_set_param_float(char *module, char *key, float val)
{
    return nerr_pass(_moc_set_param_float(m_arg, module, key, val));
}

int moc_trigger(char *module, char *key, unsigned short cmd,
                  unsigned short flags)
{
    return _moc_trigger(m_arg, module, key, cmd, flags);
}

HDF* moc_hdfrcv(char *module)
{
    return _moc_hdfrcv(m_arg, module);
}

NEOERR* moc_regist_callback(char *module, char *cmd, MocCallback cmdcbk)
{
    return nerr_pass(_moc_regist_callback(m_arg, module, cmd, cmdcbk));
}

/*
 * thread safe version
 * ===================
 */
NEOERR* moc_init_r(char *path, moc_arg **arg)
{
    HDF *node;
    NEOERR *err;
    
    MOC_NOT_NULLA(arg);

    moc_arg *rarg = mocarg_init();
    if (!rarg) nerr_raise(NERR_NOMEM, "alloc moc arg");

    *arg = rarg;

    node = _moc_get_confhdf(path);
    if (!node) return nerr_raise(NERR_ASSERT, "config error");
    err = _moc_load_fromhdf(node, rarg->evth);
    if (err != STATUS_OK) return nerr_pass(err);

#ifdef EVENTLOOP
    err = eloop_start(rarg);
    if (err != STATUS_OK) return nerr_pass(err);

    err = mcbk_start(rarg);
    if (err != STATUS_OK) return nerr_pass(err);
#endif

    return STATUS_OK;
}

NEOERR* moc_init_fromhdf_r(HDF *node, moc_arg **arg)
{
    NEOERR *err;

    MOC_NOT_NULLB(node, arg);

    moc_arg *rarg = mocarg_init();
    if (!rarg) nerr_raise(NERR_NOMEM, "alloc moc arg");

    *arg = rarg;

    err = _moc_load_fromhdf(node, rarg->evth);
    if (err != STATUS_OK) return nerr_pass(err);

#ifdef EVENTLOOP
    err = eloop_start(rarg);
    if (err != STATUS_OK) return nerr_pass(err);

    err = mcbk_start(rarg);
    if (err != STATUS_OK) return nerr_pass(err);
#endif

    return STATUS_OK;
}

void moc_destroy_r(moc_arg *arg)
{
    _moc_destroy(arg);
}

HDF* moc_hdfsnd_r(moc_arg *arg, char *module)
{
    return _moc_hdfsnd(arg, module);
}

NEOERR* moc_set_param_r(moc_arg *arg, char *module, char *key, char *val)
{
    return nerr_pass(_moc_set_param(arg, module, key, val));
}
NEOERR* moc_set_param_int_r(moc_arg *arg, char *module, char *key, int val)
{
    return nerr_pass(_moc_set_param_int(arg, module, key, val));
}
NEOERR* moc_set_param_uint_r(moc_arg *arg, char *module, char *key, unsigned int val)
{
    return nerr_pass(_moc_set_param_uint(arg, module, key, val));
}
NEOERR* moc_set_param_int64_r(moc_arg *arg, char *module, char *key, int64_t val)
{
    return nerr_pass(_moc_set_param_int64(arg, module, key, val));
}
NEOERR* moc_set_param_float_r(moc_arg *arg, char *module, char *key, float val)
{
    return nerr_pass(_moc_set_param_float(arg, module, key, val));
}

int moc_trigger_r(moc_arg *arg, char *module, char *key, unsigned short cmd,
                  unsigned short flags)
{
    return _moc_trigger(arg, module, key, cmd, flags);
}

HDF* moc_hdfrcv_r(moc_arg *arg, char *module)
{
    return _moc_hdfrcv(arg, module);
}

NEOERR* moc_regist_callback_r(moc_arg *arg, char *module, char *cmd, MocCallback cmdcbk)
{
    return nerr_pass(_moc_regist_callback(arg, module, cmd, cmdcbk));
}
