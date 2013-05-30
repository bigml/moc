#ifndef __LERR_H__
#define __LERR_H__

__BEGIN_DECLS

extern NERR_TYPE REP_ERR;        /* 14 binded with neo_error, see pop/pub/lerr.c */
extern NERR_TYPE REP_ERR_VER;
extern NERR_TYPE REP_ERR_SEND;
extern NERR_TYPE REP_ERR_BROKEN;
extern NERR_TYPE REP_ERR_UNKREQ;
extern NERR_TYPE REP_ERR_MEM;
extern NERR_TYPE REP_ERR_DB;
extern NERR_TYPE REP_ERR_BUSY;
extern NERR_TYPE REP_ERR_PACK;
extern NERR_TYPE REP_ERR_BADPARAM;
extern NERR_TYPE REP_ERR_CACHE_MISS; /* 24 */

NEOERR* lerr_init();

__END_DECLS
#endif  /* __LERR_H__ */
