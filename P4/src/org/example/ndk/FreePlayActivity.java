package org.example.ndk;

import android.app.Activity;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

public class FreePlayActivity extends Activity {
	
	Button track1Button;
	Button track2Button;
	Button nopeButton;
	Intent intent;
	
	private void setTypeface(){
		Typeface myTypeFace = Typeface.createFromAsset(getAssets(), "fonts/jalnan.ttf");
	    track1Button = (Button)findViewById(R.id.track1Button);
	    track2Button = (Button)findViewById(R.id.track2Button);
	    nopeButton = (Button)findViewById(R.id.nopeButton);
	    track1Button.setTypeface(myTypeFace);
	    track2Button.setTypeface(myTypeFace);
	    nopeButton.setTypeface(myTypeFace);
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.freeplay_activity);
        setTypeface();
        
        intent = new Intent();
        
        track1Button.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				intent.putExtra("track", 1);
				setResult(RESULT_OK, intent);
				finish();
			}
		});
        
        track2Button.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				intent.putExtra("track", 2);
				setResult(RESULT_OK, intent);
				finish();
			}
		});
        
        nopeButton.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				intent.putExtra("track", 0);
				setResult(RESULT_OK, intent);
				finish();
			}
		});
	}

}
