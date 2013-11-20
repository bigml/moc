#ifndef __MOC_BANG_H__
#define __MOC_BANG_H__

#define PREFIX_BANG "Bang"

enum {
    REQ_CMD_BANG_TO_BATTLE = 1011, /**< user battle request */
    REQ_CMD_BANG_ACCEPT_BATTLE,    /**< user accept battle request */

    REQ_CMD_BANG_TURN      = 1021, /**< user turn redirection */
    REQ_CMD_BANG_ACCELERATE,       /**< preserved command of accelerating */
    REQ_CMD_BANG_FALL              /**< preserved command of falling out of edge */
};

enum {
    DIRECTION_FORWARD = 0,
    DIRECTION_BACKWARD,
    DIRECTION_UPWARD,
    DIRECTION_DOWNWARD
};

#endif
