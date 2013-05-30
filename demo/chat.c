#include "moc.h"

#define DIE_NOK_MTL(err)                        \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        mtc_err("%s", zstra.buf);               \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
        exit(-1);                               \
    }

#define OUTPUT_NOK(err)                         \
    if (err != STATUS_OK) {                     \
        STRING zstra;    string_init(&zstra);   \
        nerr_error_traceback(err, &zstra);      \
        printf("%s", zstra.buf);                \
        string_clear(&zstra);                   \
        nerr_ignore(&err);                      \
    }

static void rand_string_with_len(char *s, int len)
{
    int x = 0;

    for (x = 0; x < len; x++) {
        s[x] = (char)(65 + neo_rand(90-65));
    }
    s[x] = '\0';
}

void joincbk(HDF *datanode)
{
    printf("%s 插入\n", hdf_get_value(datanode, "userid", NULL));
}
void quitcbk(HDF *datanode)
{
    printf("%s 抽出\n", hdf_get_value(datanode, "userid", NULL));
}
void bcstcbk(HDF *datanode)
{
    mtc_dbg("oncmd bcst");

    fprintf(stdout, "%s: %s\n",
            hdf_get_value(datanode, "userid", NULL),
            hdf_get_value(datanode, "msg", NULL));
}

int main()
{
    NEOERR *err;
    char msg[1000], nick[8];
    int ret;

    err = moc_init(NULL);
    OUTPUT_NOK(err);

    err = moc_regist_callback("chat", "join", joincbk);
    OUTPUT_NOK(err);
    err = moc_regist_callback("chat", "quit", quitcbk);
    OUTPUT_NOK(err);
    err = moc_regist_callback("chat", "bcst", bcstcbk);
    OUTPUT_NOK(err);

    rand_string_with_len(nick, 7);
    moc_set_param("chat", "userid", nick);
    ret = moc_trigger("chat", NULL, 1001, FLAGS_SYNC);
    if (PROCESS_NOK(ret)) {
        printf("regist nick %s error %d\n", nick, ret);
        moc_destroy();
        return 0;
    }

    printf("regist %s ok, rubbish please, bye to quit\n", nick);

    while (1) {
        memset(msg, 1000, 0x0);
        scanf("%s", msg);
        
        if (!strcmp(msg, "bye")) break;

        moc_set_param("chat", "msg", msg);
        
        ret = moc_trigger("chat", NULL, 1011, FLAGS_SYNC);
        if (PROCESS_OK(ret)) {
            hdf_dump(moc_hdfrcv("chat"), NULL);
        } else {
            printf("error %d\n", ret);
        }
    }

    moc_destroy();
    
    return 0;
}
