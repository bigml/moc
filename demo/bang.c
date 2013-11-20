#include "moc.h"

#include <setjmp.h>

#define USERID_LENGTH 7

static bool isInBattle = false;

char* process_keyboard(int key)
{
    char *redir = "leagal";

    switch(key) {
    case 119:
        redir = "up";
        break;
    case 115:
        redir = "down";
        break;
    case 97:
        redir = "left";
        break;
    case 100:
        redir = "right";
        break;
    default:
        redir = "illegal input";
    }

    return redir;
}

void logincbk(HDF *datanode)
{
    printf("login to server successfully.\n");
    hdf_dump(datanode, NULL);
}

void battleinvitecbk(HDF *datanode)
{
    int isAccept;
    printf("received BATTLE invite from table %d.\n",
            hdf_get_int_value(datanode, "tableid", 0));

    printf("Does you to be brave to accept this battle request: (y or n)?\n");
    if ((isAccept = getchar()) != EOF) {
        if (isAccept == 'y') {
            moc_trigger("bang", NULL, 1012, FLAGS_NONE);
        }
    }
}

void battlebegincbk(HDF *datanode)
{
    printf("receive begin battle info from server.\n");
    hdf_dump(datanode, NULL);

    isInBattle = true;
    battle_status = "true";
}

void joincbk(HDF *datanode)
{
    fprintf(stdout, "%s: %s.\n",
            hdf_get_value(datanode, "userid", NULL),
            hdf_get_value(datanode, "direction", NULL));
}

void quitcbk(HDF *datanode)
{
    printf("%s quits.\n", hdf_get_value(datanode, "userid", NULL));
}

void turncbk(HDF *datanode)
{
    fprintf(stdout, "%s: %s.\n",
            hdf_get_value(datanode, "userid", NULL),
            hdf_get_value(datanode, "redirection", NULL));
}

int main()
{
    char key[100], val[100];

    NEOERR *err;
    int   ret, cmd;
    char  nick[8];
    char *dir   = "forwards";
    char *redir = NULL;
    int   pressed_key;

    err = moc_init(NULL);
    //OUTPUT_NOK(err);

    /* register callbacks for module bang commands */
    err = moc_regist_callback("bang", "battleinvite", battleinvitecbk);
    err = moc_regist_callback("bang", "battlebegin", battlebegincbk);

    err = moc_regist_callback("bang", "login", logincbk);
    err = moc_regist_callback("bang", "join", joincbk);

    err = moc_regist_callback("bang", "quit", quitcbk);
    err = moc_regist_callback("bang", "turn", turncbk);

    //neo_rand_string(nick, USERID_LENGTH);
    /* generate userid and then register to server */
    neo_rand_word(nick, USERID_LENGTH);
    printf("user id is %s\n", nick);
    moc_set_param("bang", "userid", nick);
    moc_set_param("bang", "direction", dir);

    ret = moc_trigger("bang", NULL, 1001, FLAGS_NONE);

    if (PROCESS_NOK(ret)) {
        printf("register nick %s error %d.\n", nick, ret);
        moc_destroy();
        return 0;
    }

    printf("login ok, cmd key val please, 1011 bye foo to quit\n");

    while (1) {
        if (!isInBattle) {
            memset(key, 0x0, 100);
            memset(val, 0x0, 100);
            scanf("%d %s %s", &cmd, key, val);

            if (!strcmp(key, "bye")) break;

            moc_set_param("bang", key, val);

            /* XXX dirty code, needs to be fixed immediately! */
            if (!isInBattle && cmd) {
                ret = moc_trigger("bang", NULL, cmd, FLAGS_NONE);
                if (PROCESS_NOK(ret)) {
                    printf("error %d\n", ret);
                }
            }
        } else {
            while ((pressed_key = getchar()) != '\n') {
                redir = process_keyboard(pressed_key);

                printf("what to turn is %s\n", redir);
                printf("redirection, battle status is %s\n", battle_status);

                moc_set_param("bang", "redirection", redir);

                ret = moc_trigger("bang", NULL, 1021, FLAGS_SYNC);
                if (PROCESS_NOK(ret)) {
                    printf("error %d.\n", ret);
                }
            }
        }
    }

    moc_destroy();

    return 0;
}
