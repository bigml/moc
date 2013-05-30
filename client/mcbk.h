#ifndef __MCBK_H__
#define __MCBK_H__

__BEGIN_DECLS

NEOERR* mcbk_start(moc_arg *arg);
void mcbk_stop(moc_arg *arg);

struct moc_cbk* mcbk_create();
void mcbk_destroy(struct moc_cbk *c);
struct moc_cbk* mcbk_find(HASH *cbkh, char *module, char *cmd);
void mcbk_regist(HASH *cbkh, char *module, char *cmd, struct moc_cbk *c);


__END_DECLS
#endif  /* __MCBK_H__ */
