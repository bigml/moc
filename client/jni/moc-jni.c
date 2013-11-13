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

#define JNIREG_CLASS "com/hunantv/mongoq/MocClient"

#include "moc.h"
#define MOC_CONFIG_DIR  "conf"
#define MOC_CONFIG_FILE "mocclient.hdf"

static jboolean
moc_jni_init(JNIEnv *env, jclass clazz, jobject assetManager,
        jstring fileStr)
{
    char *fileChars = NULL;
    if (fileStr) {
        fileChars = (char*)(*env)->GetStringUTFChars(env, fileStr, NULL);
        LOGI("moc client would to be initialized by file %s.", fileChars);
    } else {
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

    HDF *node;
    hdf_init(&node);
    if (buffer) {
        hdf_read_string(node, buffer);
    }

    moc_init_fromhdf(node);

    return true;
}

static jint
moc_jni_trigger(JNIEnv *env, jclass clazz,
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
moc_jni_set_param(JNIEnv *env, jclass clazz,
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
moc_jni_destroy(JNIEnv *env, jclass clazz)
{
    LOGI("moc is to be destroyed.");

    moc_destroy();
}

static JNINativeMethod gMethods[] = {
    /* name, signature, funcPtr */
    { "init", "(Landroid/content/res/AssetManager;Ljava/lang/String;)Z",
        (void*)moc_jni_init },
    { "trigger", "(Ljava/lang/String;Ljava/lang/String;SS)I",
        (void*)moc_jni_trigger },
    { "setParam", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V",
        (void*)moc_jni_set_param },
    { "destroy", "()V",
        (void*)moc_jni_destroy },
};

static int registerNativeMethods(JNIEnv *env, const char *className,
        JNINativeMethod *gMethods, int numMethods)
{
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
    if (!registerNativeMethods(env, JNIREG_CLASS,
                gMethods, sizeof(gMethods) / sizeof(gMethods[0]))) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
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
