package com.wetgenes;

import android.app.NativeActivity;
import android.util.Log;
import android.content.Intent;
import android.content.Context;
import java.io.File;

public class FeralActivity extends NativeActivity
{

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

}
