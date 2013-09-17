package com.wetgenes;

import android.app.NativeActivity;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import java.io.File;
import android.view.InputDevice;

public class FeralActivity extends NativeActivity
{
	
	public String smell="base";
	
	public GameStick gamestick;

	private static String FA = "FeralActivity";
	public static void Log(String s)
	{
		android.util.Log.v(FA,s);
	}

	public void SendIntent(String txt)
	{
//		Log(txt);

		Intent i=new Intent(android.content.Intent.ACTION_SEND);

		i.setType("text/plain");
//		i.putExtra(Intent.EXTRA_SUBJECT, R.string.share_subject);
		i.putExtra(Intent.EXTRA_TEXT, txt );

		startActivity(Intent.createChooser(i,"Where shall we post?"));

	}
	
	public void TaskToBack()
	{
		moveTaskToBack(true);
	}

	public String GetFilesPrefix()
	{
		File f=getFilesDir();
		String s=f.getAbsolutePath();
		return s;
	}
	public String GetCachePrefix()
	{
		File f=getCacheDir();
		String s=f.getAbsolutePath();
		return s;
	}

	public String SmellCheck()
	{
		if(smell=="gamestick")
		{
			if(gamestick==null)
			{
				gamestick=new GameStick(this);
			}
		}
		
		return smell;
	}

	public String SmellMsg()
	{
		if(smell=="gamestick")
		{
			return gamestick.poll();
		}
		
		return null;
	}

	public void SmellSendScore(int n)
	{
		if(smell=="gamestick")
		{
			gamestick.SendScore(n);
		}
	}

	public void SmellRangeScore(int na,int nb)
	{
		if(smell=="gamestick")
		{
			gamestick.RangeScore(na,nb);
		}
	}

// tell app to quit
    public void FinishMe() {
        this.runOnUiThread(new Runnable() {
            public void run() {
                finish();
            }
        });
    }
    
//dumb way to see if a device disconnects (zee number she go down senor)
	public int CountInputDevices()
	{
		return InputDevice.getDeviceIds().length;
	}

}
