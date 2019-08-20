package com.android.jni.ffmpeg;

import android.Manifest;
import android.content.Intent;
import android.hardware.Camera;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.mylhyl.acp.Acp;
import com.mylhyl.acp.AcpListener;
import com.mylhyl.acp.AcpOptions;

import java.io.File;
import java.util.List;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private FfmpegPlayer FfmpegPlayer;

    private EditText video_input_name;
    private EditText video_onput_name;
    private EditText sound_input_name;
    private EditText sound_output_name;

    private VideoView videoView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Acp.getInstance(this).request(
                new AcpOptions.Builder().setPermissions(
                        Manifest.permission.WRITE_EXTERNAL_STORAGE,
                        Manifest.permission.READ_EXTERNAL_STORAGE,
                        Manifest.permission.MOUNT_UNMOUNT_FILESYSTEMS,
                        Manifest.permission.MOUNT_FORMAT_FILESYSTEMS).build(),
                new AcpListener() {
                    @Override
                    public void onGranted() {

                        // Example of a call to a native method
                        TextView tv = (TextView) findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());

                        tv.setOnClickListener(new View.OnClickListener() {
                            @Override
                            public void onClick(View view) {
                                startActivity(new Intent(MainActivity.this,PushActivity.class));
                            }
                        });

                        video_input_name = findViewById(R.id.video_input_name);
                        video_onput_name = findViewById(R.id.video_output_name);
                        sound_input_name = findViewById(R.id.sound_input_name);
                        sound_output_name = findViewById(R.id.sound_output_name);

                        videoView = findViewById(R.id.videoView);

                        FfmpegPlayer = new FfmpegPlayer(MainActivity.this);
                        tv.setText(FfmpegPlayer.init());

                    }

                    @Override
                    public void onDenied(List<String> permissions) {
                        finish();
                    }
                });

    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public void videoClick(View view){

        String input_name = video_input_name.getText().toString();
        String output_name = video_onput_name.getText().toString();

        if (TextUtils.isEmpty(input_name)|| TextUtils.isEmpty(output_name)){
            Toast.makeText(this,"需要输入名称",Toast.LENGTH_SHORT).show();
            return;
        }

        String input = new File(Environment.getExternalStorageDirectory(),input_name).getAbsolutePath();
        String output = new File(Environment.getExternalStorageDirectory(),output_name).getAbsolutePath();

        FfmpegPlayer.videoDecode(input,output);

//        FfmpegPlayer.videoDecodeDisplay(input,videoView.getHolder().getSurface());

    }

    public void soundClick(View view){

        String input_name = sound_input_name.getText().toString();
        String output_name = sound_output_name.getText().toString();

        String input = new File(Environment.getExternalStorageDirectory(),input_name).getAbsolutePath();
        String output = new File(Environment.getExternalStorageDirectory(),output_name).getAbsolutePath();
        FfmpegPlayer.soundDecode(input,output);
    }

}
