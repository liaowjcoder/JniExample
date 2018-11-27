package example.com.play;

import android.os.Environment;

/**
 * Created by zeal on 2018/11/26.
 */

public class PCMPlayer {
    static {
        System.loadLibrary("opensles_lib");
    }

    private String source = Environment.getExternalStorageDirectory()+"/test.pcm";

    public void play() {
        _play();
    }

    private native void _play();

}
