/*
 * Yarn javastubs
 */
package com.wetgenes;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.widget.TextView;
import android.graphics.Typeface;
import android.content.pm.ActivityInfo;
import android.view.Window;
import android.view.WindowManager;
import android.util.DisplayMetrics;
import android.view.KeyEvent;
import android.graphics.Bitmap;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;


public class Yarn extends Activity {

    private static native void setup();
    private static native void clean();
    private static native void update();
    private static native void draw(int[] bmap);
    private static native String getstring();
    private static native void keypress(String ascii,String key,String act);

	int bdat[]=new int[320*240];;
	Bitmap bmap;
	TextView tv;
    DrawView dv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, 
			WindowManager.LayoutParams.FLAG_FULLSCREEN);

		requestWindowFeature(Window.FEATURE_NO_TITLE);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
       
		setup();

		bmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888);

        dv = new DrawView(this);
        setContentView(dv);
        dv.requestFocus();

		updraw();
	 }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }
    
    @Override
    protected void onDestroy() {
        super.onDestroy();
        clean();
    }

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		 switch (keyCode) {
			case KeyEvent.KEYCODE_DPAD_LEFT:
				keypress(" ","left","down");
				updraw();
			return true;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				keypress(" ","right","down");
				updraw();
			return true;
			case KeyEvent.KEYCODE_DPAD_UP:
				keypress(" ","up","down");
				updraw();
			return true;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				keypress(" ","down","down");
				updraw();
			return true;
			case KeyEvent.KEYCODE_DPAD_CENTER:
				keypress(" ","space","down");
				updraw();
			return true;
		 }
		 return false;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event) {
		 switch (keyCode) {
			case KeyEvent.KEYCODE_DPAD_LEFT:
				keypress(" ","left","up");
				update();
			return true;
			case KeyEvent.KEYCODE_DPAD_RIGHT:
				keypress(" ","right","up");
				update();
			return true;
			case KeyEvent.KEYCODE_DPAD_UP:
				keypress(" ","up","up");
				update();
			return true;
			case KeyEvent.KEYCODE_DPAD_DOWN:
				keypress(" ","down","up");
				update();
			return true;
			case KeyEvent.KEYCODE_DPAD_CENTER:
				keypress(" ","space","up");
				update();
			return true;
		 }
		 return false;
	} 

    void updraw()
    {
		update();
		draw(bdat);
		bmap.setPixels(bdat, 0, 320, 0, 0, 320, 240);
        dv.invalidate();

	}

    static {
        System.loadLibrary("lua");
    }
}

class DrawView extends View implements OnTouchListener {
    private static final String TAG = "DrawView";

	Yarn yarn;

	DisplayMetrics metrics;
	
    public DrawView(Context context) {
        super(context);
		yarn=(Yarn)context;

		metrics = new DisplayMetrics();
		yarn.getWindowManager().getDefaultDisplay().getMetrics(metrics);

        setFocusable(true);
        setFocusableInTouchMode(true);

        this.setOnTouchListener(this);

    }

    @Override
    public void onDraw(Canvas canvas) {

		canvas.scale(metrics.widthPixels/320, metrics.heightPixels/240);
		canvas.drawBitmap(yarn.bmap, 0, 0, null);

    }

    public boolean onTouch(View view, MotionEvent event) {
        return true;
    }
}

