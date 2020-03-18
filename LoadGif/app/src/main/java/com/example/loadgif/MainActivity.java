package com.example.loadgif;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

import java.io.File;

public class MainActivity extends AppCompatActivity {
//    一个对象
    private Bitmap bitmap;
    private ImageView imageView;
    private GifProcessor gifHandler;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageView = findViewById(R.id.imageView);

        if(ContextCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE)
            != PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE}, 0);
        }
    }


    public void loading(View view) {

        File file=new File(Environment.getExternalStorageDirectory(),"demo.gif");
        if(file.exists()) {
            gifHandler = new GifProcessor(file.getAbsolutePath());
            int width = gifHandler.getWidth();
            int height = gifHandler.getHeight();
            bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);

//        发送下一帧的刷新事件
            int nextFrame = gifHandler.getUpdateDelay(bitmap);
            handler.sendEmptyMessageDelayed(1, nextFrame);
        }else{
            Toast.makeText(this,"没有找到demo", Toast.LENGTH_SHORT).show();
        }
    }

    @SuppressLint("HandlerLeak")
    Handler handler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            //获取下一帧
            imageView.setImageBitmap(bitmap);
            int mNextFrame=gifHandler.getUpdateDelay(bitmap);
            handler.sendEmptyMessageDelayed(1, mNextFrame);

        }
    };
}
