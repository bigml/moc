#include "mheads.h"
#include "lheads.h"

HDF *g_cfg = NULL;
HASH *g_datah = NULL;

volatile time_t g_ctime = 0;

struct stats g_stat = {0};
struct moc *g_moc = NULL;

static void useage(void)
{
    char h[] = \
        "moc [options]\n"
        "\n"
        " -c fname    config file\n"
        " -f          foreground\n"
        "\n";
    printf("%s", h);
    exit(1);
}

int main(int argc, char *argv[])
{
    NEOERR *err;

    static struct setting {
        char *conffname;
        int foreground;
    } myset = {
        .conffname = "./config.hdf",
        .foreground = 0,
    };

    if (argc < 2) useage();

    int c;
    while ((c = getopt(argc, argv, "c:f")) != -1) {
        switch(c) {
        case 'c':
            myset.conffname = strdup(optarg);
            break;
        case 'f':
            myset.foreground = 1;
            break;
        default:
            useage();
        }
    }

    sys_stats_init(&g_stat);

    err = mcfg_parse_file(myset.conffname, &g_cfg);
    OUTPUT_NOK(err);
    
    mtc_init(hdf_get_value(g_cfg, PRE_CONFIG".logfile", "/tmp/mocserver"),
             hdf_get_int_value(g_cfg, PRE_CONFIG".trace_level", TC_DEFAULT_LEVEL));
    err = lerr_init();
    RETURN_V_NOK(err, 1);

    if (!myset.foreground) {
        pid_t pid = fork();
        if (pid > 0) return 0;
        else if (pid < 0) {
            perror("Error in fork");
            return 1;
        }
        close(0);
        setsid();
    }

    mtc_foo("starting moc");

    g_moc = moc_start();

    net_go();

    moc_stop(g_moc);

    mcfg_cleanup(&g_cfg);

    return 0;
}
