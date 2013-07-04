#ifndef __LGLOBAL_H__
#define __LGLOBAL_H__

#define PLUGIN_PATH    "/usr/local/lib/"

/*
 * time since the Epoch (00:00:00 UTC, January 1, 1970), measured in seconds
 */
extern volatile time_t g_ctime;
/*
 * time since the Epoch (00:00:00 UTC, January 1, 1970),
 * measured in seconds and microseconds
 */
extern volatile double g_ctimef;

extern struct stats g_stat;
extern struct moc *g_moc;

#endif    /* __LGLOBAL_H__ */
