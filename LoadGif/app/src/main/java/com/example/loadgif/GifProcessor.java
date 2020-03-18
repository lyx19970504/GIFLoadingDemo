package com.example.loadgif;

import android.graphics.Bitmap;

public class GifProcessor {

    static {
        System.loadLibrary("native-lib");
    }

    private long mGifAddress;   //类似于Android中的Context，用来传信用的

    public GifProcessor(String path){
        mGifAddress = loadPath(path);
    }

    public int getWidth(){
        return getWidth(mGifAddress);
    }

    public int getHeight(){
        return getHeight(mGifAddress);
    }

    public int getUpdateDelay(Bitmap bitmap){
        return update(mGifAddress, bitmap);
    }

    private native long loadPath(String path);

    private native int getWidth(long ndkGif);

    private native int getHeight(long ndkGif);

    //返回每一帧与下一帧之间的延迟
    private native int update(long ndkGif, Bitmap bitmap);
}
