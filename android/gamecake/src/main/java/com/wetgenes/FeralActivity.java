package com.wetgenes;

import android.app.NativeActivity;
import android.app.ActivityManager;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import java.io.File;
import android.view.InputDevice;
//import android.location.Location;
//import android.location.LocationListener;
//import android.location.LocationManager;
import android.os.Bundle;
import android.os.PowerManager;
import android.os.Environment;

public class FeralActivity extends NativeActivity //implements LocationListener
{
	
	public String smell="base";
	
//	public GameStick gamestick;

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

	public int GetMemoryClass()
	{
		ActivityManager am = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
		int memoryClass = am.getMemoryClass();
		return memoryClass;
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

	public String GetExternalStoragePrefix()
	{
		String s=Environment.getExternalStorageDirectory().getAbsolutePath();
		return s;
	}
	
	public String SmellCheck()
	{
/*
		if(smell=="gamestick")
		{
			if(gamestick==null)
			{
				gamestick=new GameStick(this);
			}
		}
*/		
		return smell;
	}

	public String SmellMsg()
	{
/*
		if(smell=="gamestick")
		{
			return gamestick.poll();
		}
*/		
		return null;
	}

	public void SmellSendScore(int n)
	{
/*
		if(smell=="gamestick")
		{
			gamestick.SendScore(n);
		}
*/
	}

	public void SmellRangeScore(int na,int nb)
	{
/*
		if(smell=="gamestick")
		{
			gamestick.RangeScore(na,nb);
		}
*/
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

//where are we?

//	private LocationManager lm;
	private PowerManager.WakeLock wl;
		
/*

@Override
public void onLocationChanged(Location location) {
}

@Override
public void onProviderDisabled(String provider) {
}

@Override
public void onProviderEnabled(String provider) {
}

@Override
public void onStatusChanged(String provider, int status, Bundle extras) {
}
*/

/*
 	private Location location()
	{
		if(lm==null)
		{
			lm = (LocationManager) getApplicationContext().getSystemService(Context.LOCATION_SERVICE);
			runOnUiThread(new Runnable(){
				@Override
				public void run(){
					LocationManager lm = (LocationManager) getApplicationContext().getSystemService(Context.LOCATION_SERVICE);
					lm.requestLocationUpdates(LocationManager.GPS_PROVIDER,0,0, FeralActivity.this);
					lm.requestLocationUpdates(LocationManager.NETWORK_PROVIDER,0,0, FeralActivity.this);
				}
			});
		
			PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            wl = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "FeralActivity");
            wl.acquire();
		}
		Location l = lm.getLastKnownLocation(LocationManager.GPS_PROVIDER);
		if(l==null) { l = lm.getLastKnownLocation(LocationManager.NETWORK_PROVIDER); }
		return l;

	}

	private Location loc;
	public double GetLocation()
	{
		loc=location(); // only update here, so next two lng/lat functions use this data
		if(loc!=null)
		{
			return loc.getAccuracy(); // and we return accuracy here
		}
		return 999.0;
	}
	public double GetLatitude()
	{
		if(loc!=null)
		{
			return loc.getLatitude();
		}
		return 999.0;
	}
	public double GetLongitude()
	{
		if(loc!=null)
		{
			return loc.getLongitude();
		}
		return 999.0;
	}
*/

}
