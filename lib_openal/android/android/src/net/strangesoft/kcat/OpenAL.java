package net.strangesoft.kcat;

import java.io.InputStream;
import java.io.IOException;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

public class OpenAL extends Activity
{
    static
    {
        // make sure openal is loaded before shared library that uses it
        System.loadLibrary("openal");
        System.loadLibrary("example");
    }
    
    static native void run(byte[] data);

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        byte[] data = null;

        try
        {
            InputStream is = getAssets().open("trash80_-_three-four-robot-slojam.ogg");

            int length = is.available();

            data = new byte[length];
            is.read(data);
            is.close();
        }
        catch (java.io.IOException e)
        {
            Log.i("OpenAL", "ogg file not found in assets");
        }

        if (data != null)
        {
            run(data);
        }
        
        finish();
    }
}
