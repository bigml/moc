#ifndef __LHEADS_H__
#define __LHEADS_H__

#include <pthread.h>        /* for mutexes */
#include <event.h>

#include "moc-private.h"

#include "lglobal.h"

#include "cache.h"
#include "queue.h"
#include "parse.h"
#include "req.h"
#include "tcp.h"
#include "net.h"
#include "mocd.h"
#include "syscmd.h"

/*
 * we need talk to other moc server through moc_trigger() and wait for response
 * In this situation, I am a client actor.
 * Other mocv can push message to me use the same method if they want, I'm a
 * server, my major actor.
 */
#include "moc.h"

#endif  /* __LHEADS_H */
