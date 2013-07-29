LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := client_static

LOCAL_MODULE_FILENAME := libclient

LOCAL_SRC_FILES := \
				clearsilver/util/filter.c \
				clearsilver/util/missing.c \
				clearsilver/util/neo_date.c \
				clearsilver/util/neo_err.c \
				clearsilver/util/neo_files.c \
				clearsilver/util/neo_hash.c \
				clearsilver/util/neo_hdf.c \
				clearsilver/util/neo_misc.c \
				clearsilver/util/neo_net.c \
				clearsilver/util/neo_rand.c \
				clearsilver/util/neo_str.c \
				clearsilver/util/snprintf.c \
				clearsilver/util/ulist.c \
				clearsilver/util/wildmat.c \
				eloop.c \
				internal.c \
				lerr.c \
				mcbk.c \
				moc.c \
				mscli.c \
				mtrace.c \
				packet.c \
				tcp.c

LOCAL_C_INCLUDES := \
                clearsilver \
                clearsilver/util \
                .

LOCAL_EXPORT_C_INCLUDES := \
		$(LOCAL_PATH)/clearsilver \
		$(LOCAL_PATH)/clearsilver/util \
		$(LOCAL_PATH)
		
LOCAL_LDLIBS := -lpthread
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog

LOCAL_CFLAGS += -g -Wall -std=c99 -D_XOPEN_SOURCE=600 -fno-strict-aliasing -D_GNU_SOURCE -D_DARWIN_C_SOURCE
LOCAL_CFLAGS += -DEVENTLOOP
LOCAL_CFLAGS += -DANDROID_BUILD

include $(BUILD_STATIC_LIBRARY)
