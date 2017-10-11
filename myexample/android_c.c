#include <jni.h>
#include <android/log.h>
#include <stddef.h>
#include "libusb.h"
#include <glib.h>
#define LOG_TAG "MediaRecorder"
#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, __VA_ARGS__)
//#define FALSE 0
JNIEXPORT void JNICALL
Java_com_example_testcusejava_Testcusenative_libusbcallfunc(JNIEnv* env, jobject obj)
{
    //openUsbDevice(4703,12587);
    int rc,r;
    libusb_device_handle *handle = NULL;
    libusb_device *device;
    libusb_context *ctx = NULL;
     r = libusb_init(&ctx);//启动libusb

     jclass class  = (*env)->FindClass (env, "com/example/testcusejava/Testcusenative");
    jmethodID openUsbDevice = (*env)->GetStaticMethodID(env, class, "openUsbDevice", "(II)I");//GetStaticMethodID GetMethodID，通过jni获取Java的方法openUsbDevice
    int fd =(*env)->CallStaticIntMethod(env, class,openUsbDevice ,4703,12587);//CallVoidMethod,CallStaticIntMethod调用openUsbDevice 获取fd
    LOG("!!!!!一二三!!!!! fd = %d",fd);
    __android_log_write(6, "android-service", "Trying to open USB device.get_usb_device_fd");
    //rc = libusb_open_fd(device, &handle,fd);//打开usb设备
    if (rc != 0) { __android_log_write(6, "channel-usbredir", "Failed to open USB device."); }

    //libusb_close(handle);
    libusb_exit(ctx); //结束libusb
}