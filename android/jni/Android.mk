LOCAL_PATH      := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE    := moc

LOCAL_SRC_FILES :=                \
    client/clearsilver/util/filter.c     \
    client/clearsilver/util/missing.c    \
    client/clearsilver/util/neo_date.c   \
    client/clearsilver/util/neo_err.c    \
    client/clearsilver/util/neo_files.c  \
    client/clearsilver/util/neo_hash.c   \
    client/clearsilver/util/neo_hdf.c    \
    client/clearsilver/util/neo_misc.c   \
    client/clearsilver/util/neo_net.c    \
    client/clearsilver/util/neo_rand.c   \
    client/clearsilver/util/neo_str.c    \
    client/clearsilver/util/snprintf.c   \
    client/clearsilver/util/ulist.c      \
    client/clearsilver/util/wildmat.c    \
    client/clearsilver/util/java/j_neo_util.c \
    client/eloop.c                       \
    client/internal.c                    \
    client/lerr.c                        \
    client/mcbk.c                        \
    client/moc.c                         \
    client/mscli.c                       \
    client/mtrace.c                      \
    client/packet.c                      \
    client/tcp.c                         \
    j_moc.c\

LOCAL_C_INCLUDES :=      	              \
    $(LOCAL_PATH)/client/clearsilver      \
    $(LOCAL_PATH)/client/clearsilver/util \
    $(LOCAL_PATH)/client/

LOCAL_EXPORT_C_INCLUDES :=                \
    $(LOCAL_PATH)/client/clearsilver      \
    $(LOCAL_PATH)/client/clearsilver/util \
    $(LOCAL_PATH)/client/

LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog
LOCAL_LDLIBS += -landroid

LOCAL_CFLAGS += -g -Wall -std=c99 -D_XOPEN_SOURCE=600 -fno-strict-aliasing -D_GNU_SOURCE -D_DARWIN_C_SOURCE
LOCAL_CFLAGS += -DEVENTLOOP
LOCAL_CFLAGS += -DANDROID_BUILD

include $(BUILD_SHARED_LIBRARY)
