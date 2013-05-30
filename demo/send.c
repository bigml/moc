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

int main()
{
    NEOERR *err;
    int ret;

    err = moc_init(NULL);
    OUTPUT_NOK(err);

    moc_set_param("base", "userid", "747");

    ret = moc_trigger("skeleton", "747", REQ_CMD_STATS, FLAGS_SYNC);
    if (PROCESS_OK(ret)) {
        hdf_dump(moc_hdfrcv("skeleton"), NULL);
    } else {
        printf("error %d\n", ret);
    }

    moc_destroy();
    
    return 0;
}
