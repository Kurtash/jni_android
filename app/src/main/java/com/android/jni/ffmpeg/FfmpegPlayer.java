package com.android.jni.ffmpeg;

import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;
import android.view.Surface;
import android.widget.Toast;

/**
 * Created by DELL on 2019/7/16.
 */

public class FfmpegPlayer {

    private Context mContxt;

    static {

        System.loadLibrary("avutil-54");
        System.loadLibrary("avformat-56");
        System.loadLibrary("avcodec-56");
        System.loadLibrary("swscale-3");
        System.loadLibrary("swresample-1");
        System.loadLibrary("avfilter-5");
        System.loadLibrary("avdevice-56");
        System.loadLibrary("postproc-53");
        System.loadLibrary("ffmpegplayer");

    }

    public FfmpegPlayer(Context mContxt) {
        this.mContxt = mContxt;
    }

    public native String init();

    public native void videoDecode(String input,String output);

    public native void videoDecodeDisplay(String input, Surface surface);

    public native void soundDecode(String input,String output);

    public void showToast(String msg){
        Toast.makeText(mContxt,msg,Toast.LENGTH_SHORT).show();
    }

    public native int cameraLiveInit(int width,int height,String url);

    public native int cameraLiveStart(byte[] cameraData);

    public native int cameraLiveStop();

    public native int cameraLiveClose();

    public native int cameraLiveJason(String input, String output);

    /**
     * 创建一个AudioTrack对象，用于播放
     * @param nb_channels
     * @return
     */
    public AudioTrack createAudioTrack(int sampleRateInHz, int nb_channels){
        //固定格式的音频码流
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        Log.i("jason", "nb_channels:" + nb_channels);
        //声道布局
        int channelConfig;
        if(nb_channels == 1){
            channelConfig = android.media.AudioFormat.CHANNEL_OUT_MONO;
        }else if(nb_channels == 2){
            channelConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;
        }else{
            channelConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;
        }

        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);

        AudioTrack audioTrack = new AudioTrack(
                AudioManager.STREAM_MUSIC,
                sampleRateInHz, channelConfig,
                audioFormat,
                bufferSizeInBytes, AudioTrack.MODE_STREAM);
        //播放
        //audioTrack.play();
        //写入PCM
        //audioTrack.write(audioData, offsetInBytes, sizeInBytes);
        return audioTrack;
    }

}
