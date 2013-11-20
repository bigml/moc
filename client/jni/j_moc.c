/**
 * Moc client jni for Android.
 */
#include <jni.h>
#include <assert.h>

/* for native asset manager */
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#define TAG "moc"
#include <android/log.h>
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__)

#define MOC_CLIENT_CLASS    "com/hunantv/tazai/MocClient"
#define CALLBACK_CHAT_CLASS "com/hunantv/tazai/ChatActivity"
#define CALLBACK_BANG_CLASS "com/hunantv/tazai/BangActivity"

#include "moc.h"
#define MOC_CONFIG_DIR  "conf"
#define MOC_CONFIG_FILE "mocclient.hdf"

JavaVM *gVm                = NULL;
JNIEnv *gCallbackEnv       = NULL;
jclass  gCallbackChatClass = NULL;
jclass  gCallbackBangClass = NULL;

/*
 * methods definition for module 'chat'
 */
jmethodID gMethodChatJoinId = NULL;
jmethodID gMethodChatQuitId = NULL;

/*
 * methods definition for module 'bang'
 */
jmethodID gMethodBangJoinId = NULL;
jmethodID gMethodBangQuitId = NULL;
jmethodID gMethodBattleInviteId = NULL;
jmethodID gMethodBattleBeginId  = NULL;
jmethodID gMethodTurnId = NULL;

static jstring
hdf_write_jstring(HDF *node)
{
    char *buffer = NULL;
    if (node) {
        hdf_write_string(node, &buffer);
    }

    jstring jstrBuffer = (*gCallbackEnv)->NewStringUTF(gCallbackEnv, buffer);

    return jstrBuffer;
}

/*
 * construct jobject via default construct for class to invoke
 * non-static method.
 */
static jobject
getInstance(JNIEnv *env, jclass objClass)
{
    jmethodID construct_id = (*env)->GetMethodID(env, objClass, "<init>", "()V");
    jobject   obj          = (*env)->NewObject(env, objClass, construct_id);

    return    obj;
}

/*
 * For native implementation for java methods, if its' static method
 * belongs to class itself, then second parameter should be jclass(it
 * refers to MocClient here), otherwise, it should be jobject because
 * it's just instance of specified class.
 */
static jboolean
j_moc_init(JNIEnv *env, jobject objClass,
        jobject assetManager, jstring fileStr)
{
    char *fileChars = NULL;
    if (fileStr) {
        fileChars = (char*)(*env)->GetStringUTFChars(env, fileStr, NULL);
        LOGI("moc client would to be initialized by file %s.", fileChars);
    } else {
        fileChars = "conf/mocclient.hdf";
        LOGI("moc client would to be initialized by default configurations.");
    }

    AAssetManager *manager = AAssetManager_fromJava(env, assetManager);
    assert(manager != NULL);

    AAsset *file = AAssetManager_open(manager, "conf/mocclient.hdf", AASSET_MODE_UNKNOWN);
    if (file == NULL) {
        LOGE("sepecified file does not exists in apk.");
    }

    /* read contents from config file */
    off_t bufferSize = AAsset_getLength(file);
    char *buffer     = (char*) malloc(bufferSize + 1);
    buffer[bufferSize] = 0;

    AAsset_read(file, buffer, bufferSize);

    /* close file */
    AAsset_close(file);

    moc_init_frombuf(buffer);

    return true;
}

static void
j_moc_bang_login_callback(HDF *datanode)
{
    LOGI("moc is callbacking.");

    /*
     * Java VM would check whether jni environment thread id equals to caller
     * thread id.
     */
    (*gCallbackEnv)->CallStaticVoidMethod(gCallbackEnv, gCallbackBangClass,
            gMethodBangJoinId, hdf_write_jstring(datanode));
}

static void
j_moc_regist_callback(JNIEnv *env, jobject objClass)
{
    /*
     * Load 'bang' module callbacks
     */
    gCallbackBangClass = (*env)->FindClass(env, CALLBACK_BANG_CLASS);

    gMethodBangJoinId  = (*env)->GetStaticMethodID(env, gCallbackBangClass,
            "loginCallback", "(Ljava/lang/String;)V");

    moc_regist_callback("bang", "login", j_moc_bang_login_callback);
}

static jint
j_moc_trigger(JNIEnv *env, jobject objClass,
        jstring moduleStr, jstring keyStr, jshort cmd, jshort flags)
{
    char *moduleChars = (char*)(*env)->GetStringUTFChars(env, moduleStr, NULL);
    char *keyChars    = NULL; /* keyChars may be null */
    if (keyStr) {
        keyChars = (char*)(*env)->GetStringUTFChars(env, keyStr, NULL);
    }

    LOGI("module is %s, key is %s, cmd is %d, flags is %d.",
            moduleChars, keyChars != NULL ? keyChars : "null", cmd, flags);

    return moc_trigger(moduleChars, keyChars, cmd, flags);
}

static void
j_moc_set_param(JNIEnv *env, jobject objClass,
        jstring moduleStr, jstring keyStr, jstring valStr)
{
    char *moduleChars = (char*)(*env)->GetStringUTFChars(env, moduleStr, NULL);
    char *keyChars    = (char*)(*env)->GetStringUTFChars(env, keyStr, NULL);
    char *valChars    = (char*)(*env)->GetStringUTFChars(env, valStr, NULL);

    LOGI("module is %s, key is %s, val is %s.",
            moduleChars, keyChars, valChars);

    moc_set_param(moduleChars, keyChars, valChars);
}

static void
j_moc_destroy(JNIEnv *env, jobject objClass)
{
    LOGI("moc is to be destroyed.");

    moc_destroy();
}

static JNINativeMethod gMethods[] = {
    /* name, signature, funcPtr */
    { "init", "(Landroid/content/res/AssetManager;Ljava/lang/String;)Z",
        (void*)j_moc_init },
    { "registCallback", "()V",
        (void*)j_moc_regist_callback },
    { "trigger", "(Ljava/lang/String;Ljava/lang/String;SS)I",
        (void*)j_moc_trigger },
    { "setParam", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
        (void*)j_moc_set_param },
    { "destroy", "()V",
        (void*)j_moc_destroy },
};

static int registerNativeMethods(JNIEnv *env, const char *className,
        JNINativeMethod *gMethods, int numMethods)
{
    /* Note: parameter class would refer to MocClient! */
    jclass clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }

    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

static int registerNatives(JNIEnv *env)
{
    if (!registerNativeMethods(env, MOC_CLIENT_CLASS,
                gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    /* cache the JavaVM interface pointer */
    gVm = vm;

    JNIEnv *env    = NULL;
    jint    result = -1;

    if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK){
        return -1;
    }

    LOGI("Register native methods inside moc client.");
    if (!registerNatives(env)) {
        return -1;
    }

    result = JNI_VERSION_1_4;

    return result;
}

/* moc-jni.c ends here */
