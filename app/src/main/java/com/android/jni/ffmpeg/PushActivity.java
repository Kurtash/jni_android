package com.android.jni.ffmpeg;

import android.app.Activity;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.IOException;

/**
 * Created by DELL on 2019/8/12.
 */

public class PushActivity extends Activity{

    private static final String TAG= "PushActivity";

    private SurfaceView surfaceview;

    private Camera mCamera;
    private SurfaceHolder holder;

    private FfmpegPlayer FfmpegPlayer;

    private Camera.PreviewCallback mPreviewCallbacx;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_push);
        FfmpegPlayer = new FfmpegPlayer(this);

        mPreviewCallbacx = new Camera.PreviewCallback() {
            @Override
            public void onPreviewFrame(byte[] arg0, Camera arg1) {

//                WSPlayer.start(arg0);
                FfmpegPlayer.cameraLiveStart(arg0);
            }
        };

        surfaceview = findViewById(R.id.surfaceview);

        SurfaceHolder surfaceHolder = surfaceview.getHolder();
        surfaceHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        surfaceHolder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                try{
                    if(mCamera!=null){
                        mCamera.setPreviewDisplay(surfaceHolder);
                        holder=surfaceHolder;
                    }
                }catch(IOException exception){
                    Log.e(TAG, "Error setting up preview display", exception);
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

                if(mCamera==null) return;
                Camera.Parameters parameters=mCamera.getParameters();
                parameters.setPreviewSize(640,480);
                parameters.setPictureSize(640,480);
                mCamera.setParameters(parameters);
                try{
                    mCamera.startPreview();
                    holder=surfaceHolder;
                }catch(Exception e){
                    Log.e(TAG, "could not start preview", e);
                    mCamera.release();
                    mCamera=null;
                }

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                if(mCamera!=null)
                {
                    mCamera.stopPreview();
                    surfaceview = null;
                    holder = null;
                }
            }
        });

    }

    @Override
    protected void onResume(){
        super.onResume();
        if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.GINGERBREAD){
            mCamera=Camera.open(0);
        }else
        {
            mCamera=Camera.open();
        }
        mCamera.setDisplayOrientation(90);

        //        WSPlayer.initialize(mCamera.getParameters().getPreviewSize().width,mCamera.getParameters().getPreviewSize().height,"rtmp://192.168.1.198:1935/live/live");
        FfmpegPlayer.cameraLiveInit(mCamera.getParameters().getPreviewSize().width,mCamera.getParameters().getPreviewSize().height,"rtmp://192.168.1.198:1935/live/live");
        mCamera.setPreviewCallback(mPreviewCallbacx);
    }

    @Override
    protected void onPause(){
        super.onPause();
//        WSPlayer.stop();
//        WSPlayer.close();
        FfmpegPlayer.cameraLiveStop();
        FfmpegPlayer.cameraLiveClose();
        if(mCamera!=null){
            mCamera.release();
            mCamera=null;
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
//        WSPlayer.stop();
//        WSPlayer.close();
        FfmpegPlayer.cameraLiveStop();
        FfmpegPlayer.cameraLiveClose();
        if(mCamera!=null){
            mCamera.release();
            mCamera=null;
        }
    }

}
