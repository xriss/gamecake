package {package};

import org.libsdl.app.SDLActivity;

public class CakeAct extends SDLActivity {

	protected String[] getArguments() {
		String[] ss = { "-landroid" , getContext().getPackageCodePath() , {commandline} };
		return ss;
	}

}



