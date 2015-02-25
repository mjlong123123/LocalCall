
#include "customrtp.h"
#include <android/log.h>


const char* const LOG_TAG = "customrtp";

#define LOG_LOCAL(...)   __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__);

//throw java exception.
void jniThrowExceptionExt(JNIEnv* env, const char* exc, const char* msg = NULL)
{
    jclass excClazz = env->FindClass(exc);
    env->ThrowNew(excClazz, msg);
}

JavaRtp::JavaRtp(JNIEnv* env, jobject & thiz, jobject & weak_ref,jmethodID callbackid)
{
    LOG_LOCAL("JavaRtp::JavaRtp() start");

    mIsOpen = false;
    mFrameSize = 0;
    mPortBase = 0;

    if(env == NULL || thiz == NULL || weak_ref == NULL)
    {
        LOG_LOCAL("JavaRtp::JavaRtp() env == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp env == NULL || thiz == NULL || weak_ref == NULL error");
        return;
    }
    env->GetJavaVM(&mJavaVM);
    if(mJavaVM == NULL)
    {
        LOG_LOCAL("JavaRtp::JavaRtp() mJavaVM == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp mJavaVM == NULL error");
    }
    jclass cls = env->GetObjectClass(thiz);
    if(cls == NULL)
    {
        LOG_LOCAL("JavaRtp::JavaRtp() cls == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp cls == NULL error");
    }
    mClass =(jclass) env->NewGlobalRef(cls);
    if(mClass == NULL)
    {
        LOG_LOCAL("JavaRtp::JavaRtp() mClass == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp mClass == NULL error");
    }
    mObject = (jobject)env->NewGlobalRef(weak_ref);
    if(mObject == NULL)
    {
        LOG_LOCAL("JavaRtp::JavaRtp() mObject == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp mObject == NULL error");
    }
    mCallback = callbackid;
    if(mCallback == NULL)
    {
        LOG_LOCAL("JavaRtp::callJavaCallBack() mCallback is null");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp mCallback == NULL error");
        return;
    }
    mMyRTPSession = new CustomRTPSession(this);
    if(mMyRTPSession == NULL)
    {
        LOG_LOCAL("JavaRtp::JavaRtp() sess == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp sess == NULL error");
    }
    mSpeexRtp = new CustomSpeex();
    if(mSpeexRtp == NULL)
    {
        LOG_LOCAL("JavaRtp::JavaRtp() mSpeexRtp == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::JavaRtp mSpeexRtp == NULL error");
    }

}
JavaRtp::~JavaRtp()
{
    JNIEnv* env = NULL;
    LOG_LOCAL("JavaRtp::~JavaRtp() start");
    if(mJavaVM == NULL)
    {
        LOG_LOCAL("JavaRtp::~JavaRtp() mJavaVM == NULL error");
        jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::~JavaRtp mJavaVM == NULL error");
        return;
    }
    else
    {
        if (mJavaVM->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
        {
            LOG_LOCAL("JavaRtp::~JavaRtp() mJavaVM->GetEnv error");
            jniThrowExceptionExt(env, "java/lang/RuntimeException", "JavaRtp::~JavaRtp get env error");
        }
    }
    if(env)
    {
        LOG_LOCAL("JavaRtp::~JavaRtp() delete class and object");
        env->DeleteGlobalRef(mClass);
        env->DeleteGlobalRef(mObject);
    }
    delete(mMyRTPSession);
    mMyRTPSession = NULL;
    delete(mSpeexRtp);
    mSpeexRtp = NULL;
    mClass = NULL;
    mObject = NULL;
    mJavaVM = NULL;
    mCallback = NULL;
}
void JavaRtp::callJavaCallBack(int returncode, short * buffer, int size)
{
    jshortArray bufret;
    LOG_LOCAL("JavaRtp::callJavaCallBack() start");

    mJavaVM->AttachCurrentThread(&mEnv, NULL);
    if(mEnv == NULL)
    {
        LOG_LOCAL("JavaRtp::callJavaCallBack() mEnv is null");
        return;
    }

    LOG_LOCAL("JavaRtp::callJavaCallBack() start 6");

    bufret = mEnv->NewShortArray(mSpeexRtp->getFrameSize());
    mEnv->SetShortArrayRegion(bufret, 0, mSpeexRtp->getFrameSize(), (const jshort * )buffer);

    mEnv->CallStaticVoidMethod(mClass,mCallback,bufret,mSpeexRtp->getFrameSize());
    LOG_LOCAL("JavaRtp::callJavaCallBack() start 7");
    mJavaVM->DetachCurrentThread();
}
