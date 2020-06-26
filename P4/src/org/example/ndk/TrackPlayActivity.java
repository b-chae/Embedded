package org.example.ndk;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;

public class TrackPlayActivity extends Activity {

	public static TrackPlayActivity mContext;
	TextView textView;
	Button backButton;
	Intent intent;
	Intent sendIntent;
	
	public void EndSong() {
		textView.setText("Done!");
	}
	
	private void setTypeface(){
		Typeface myTypeFace = Typeface.createFromAsset(getAssets(), "fonts/jalnan.ttf");
	    backButton = (Button)findViewById(R.id.playStop);
	    backButton.setTypeface(myTypeFace);
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.track_play_activity);
        setTypeface();
        mContext = this;
        
        textView = (TextView)findViewById(R.id.textView10);
        
        intent = getIntent();
        sendIntent = new Intent();
        
        int length = intent.getExtras().getInt("str");
        if(length == 0){
        	textView.setText("Not recorded yet!");
        }
        else{
        	textView.setText("Playing..");
        }
        
        backButton.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				sendIntent.putExtra("back", 1);
				setResult(RESULT_OK, sendIntent);
				finish();
			}
		});
	
	}
	
}
