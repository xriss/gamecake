package com.wetgenes;

import android.app.NativeActivity;
import android.util.Log;
import android.content.Intent;

public class FeralActivity extends NativeActivity
{

	private static String FA = "FeralActivity";
	public static void Log(String s)
	{
		android.util.Log.v(FA,s);
	}

	public void SendIntent(String txt)
	{
		Log(txt);

		Intent i=new Intent(android.content.Intent.ACTION_SEND);

		i.setType("text/plain");
//		i.putExtra(Intent.EXTRA_SUBJECT, R.string.share_subject);
		i.putExtra(Intent.EXTRA_TEXT, txt );

//		moveTaskToBack(true);
		startActivity(Intent.createChooser(i,"Where shall we post?"));

	}

}
