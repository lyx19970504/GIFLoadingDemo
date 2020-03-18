#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include "gif_lib.h"
#include <android/log.h>
#include <android/bitmap.h>
#define  LOG_TAG    "myLog"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)

typedef struct GifBean{

    int current_frame;   //记录当前帧
    int total_frame;     //总帧数
    int *delays;       //2帧之间的间隙时间

} ;

/**
 * 绘制当前帧，这里主要是将每一帧的所有信息取出，然后将他们设置到pixels上
 * 帧：源数据    pixels：新数据
 */
void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {

    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame]; //获取当前帧
    //帧的数据形式是int类型，这里获取的
    int* px = (int *)pixels;
    int  pointPixel;
    GifImageDesc frameInfo = savedImage.ImageDesc;    //获取帧的详细信息
    GifByteType  gifByteType;//压缩数据
//    获取rbg颜色的压缩工具
    ColorMapObject* colorMapObject=frameInfo.ColorMap;

    px = (int *) ((char*)px + info.stride * frameInfo.Top);
    //    获取每一行的首地址
    int *line;
    for (int y = frameInfo.Top; y <frameInfo.Top + frameInfo.Height ; ++y) {
        line=px;
        for (int x = frameInfo.Left; x <frameInfo.Left + frameInfo.Width ; ++x) {

            pointPixel=  ( y - frameInfo.Top) * frameInfo.Width + ( x - frameInfo.Left);

            gifByteType= savedImage.RasterBits[pointPixel];
            GifColorType gifColorType=colorMapObject->Colors[gifByteType];
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);  //对每一行进行赋值
        }
        px = (int *) ((char*)px + info.stride);  //偏移到下一行
    }


}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_example_loadgif_GifProcessor_loadPath(JNIEnv *env, jobject instance, jstring path) {
    const char *charPath = env->GetStringUTFChars(path, 0);   //获取char形式的路径

    int err;

    GifFileType  *gifFileType= DGifOpenFileName(charPath, &err);  //通过函数获取GIf图

    DGifSlurp(gifFileType);
//new GifBean
    GifBean *gifBean = (GifBean *) malloc(sizeof(GifBean));   //为GifBean开辟空间，类似与java的 XX xx = new XX()
    memset(gifBean, 0, sizeof(GifBean));
    gifBean->delays = (int *) malloc(sizeof(int) * gifFileType->ImageCount);  //为数组开辟空间
    memset(gifBean->delays, 0, sizeof(int) * gifFileType->ImageCount);

    gifFileType->UserData = gifBean;   //UserData是携带对象，类似于View.setTag()
    gifBean->current_frame = 0;
    gifBean->total_frame=gifFileType->ImageCount;
    ExtensionBlock *graphicsBlock = nullptr;
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
           SavedImage frame=gifFileType->SavedImages[i];   //获取每一帧图像
        for (int j = 0; j <  frame.ExtensionBlockCount; ++j) {      //遍历每一帧图像的控制块
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                graphicsBlock = &frame.ExtensionBlocks[j];   //获取图形控制块
                break;
            }
        }
        if (graphicsBlock) {
            int frame_delay = 10 * (graphicsBlock->Bytes[1] | (graphicsBlock->Bytes[2] << 8));//得到毫秒单位的延迟时间
            LOGE("1  %d   ",graphicsBlock->Bytes[1]);
            LOGE("2  %d   ",graphicsBlock->Bytes[2]);
            gifBean->delays[i] = frame_delay;
        }
    }

    env->ReleaseStringUTFChars(path, charPath);
    return (jlong)gifFileType;   //返回对象的地址
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_loadgif_GifProcessor_getWidth(JNIEnv *env, jobject instance, jlong ndkGif) {

    GifFileType *gifFileType = (GifFileType *) ndkGif;  //根据取地址获取对象
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_loadgif_GifProcessor_getHeight(JNIEnv *env, jobject instance, jlong ndkGif) {
    GifFileType *gifFileType = (GifFileType *) ndkGif;   //根据取地址获取对象
    return gifFileType->SHeight;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_loadgif_GifProcessor_update(JNIEnv *env, jobject instance, jlong ndkGif,
                                                       jobject bitmap) {

    GifFileType *gifFileType = (GifFileType *) ndkGif;

    GifBean * gifBean= (GifBean *)gifFileType->UserData;

    AndroidBitmapInfo info;

    AndroidBitmap_getInfo(env, bitmap, &info);  //通过AndroidBitmap_getInfo方法可以获取Bitmap对象的信息

    void *pixels;
    /**
     * AndroidBitmap_lockPixels和AndroidBitmap_unlockPixels是一对的，
     * 在他们之前可以直接对bitmap的像素进行修改，通过传递的第三个参数pixels
     */
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    drawFrame(gifFileType, gifBean, info, pixels);
    gifBean->current_frame += 1;
    if (gifBean->current_frame >= gifBean->total_frame - 1) {    //循环播放
        gifBean->current_frame = 0;
    }

    AndroidBitmap_unlockPixels(env, bitmap);   //与上面的AndroidBitmap_lockPixels对应
    return gifBean->delays[gifBean->current_frame];
}