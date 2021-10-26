package com.wetgenes.gamecake;

import org.libsdl.app.SDLActivity;

public class CakeAct extends SDLActivity {

	protected String[] getArguments() {
		String[] ss = { getContext().getPackageCodePath() , "--" , "--logs" , "--fullscreen" };
		return ss;
	}

}



