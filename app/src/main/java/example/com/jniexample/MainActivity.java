package example.com.jniexample;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.example.audioplayer.player.AudioPlayer;
import com.example.audioplayer.player.OnPlayPreparedListener;
import com.example.lib.JniThreadDemo;


public class MainActivity extends AppCompatActivity {
    private static final String TAG = MainActivity.class.getSimpleName();
    private AudioPlayer mAudioPlayer = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mAudioPlayer = new AudioPlayer();
        mAudioPlayer.setOnPlayPreparedListener(new OnPlayPreparedListener() {
            @Override
            public void onPrepared() {
                Log.i(TAG, "onPrepared");
                mAudioPlayer.start();
            }
        });
    }

    public void hello_ffmpeg(View view) {
//        JniDemo jniDemo = new JniDemo();
//        jniDemo.test();
    }

    public void hello_jni_thread(View view) {
        JniThreadDemo jniThreadDemo = new JniThreadDemo();

        jniThreadDemo.createThread();
    }

    public void callJavaMethodOnCppMainThread(View view) {
        JniThreadDemo jniThreadDemo = new JniThreadDemo();

        jniThreadDemo.callJavaMethodOnCPPMainThread();
    }

    public void callJavaMethodOnCppChildThread(View view) {
        JniThreadDemo jniThreadDemo = new JniThreadDemo();

        jniThreadDemo.callJavaMethodOnCppChildThread();
    }

    //生产者与消费者
    public void mutex(View view) {
        JniThreadDemo jniThreadDemo = new JniThreadDemo();

        jniThreadDemo.mutex();
    }


    //生产者与消费者
    public void stopMutex(View view) {
        JniThreadDemo jniThreadDemo = new JniThreadDemo();

        jniThreadDemo.stopMutex();
    }

    public void audio_prepare(View view) {
        mAudioPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        mAudioPlayer.prepare();
    }

    public void audio_start(View view) {

    }
}


