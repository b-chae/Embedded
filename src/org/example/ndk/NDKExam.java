package org.example.ndk;

//import com.example.androidex.MainActivity.BackThread;

import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.graphics.Color;
import android.graphics.Typeface;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.text.style.StyleSpan;
import android.view.Menu;
import android.view.View;
import android.view.animation.AlphaAnimation;
import android.view.animation.Animation;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;

public class NDKExam extends Activity {
	public native int switchdriveropen();
	public native int switchdriverclose(int fd);
	public native int driverwrite(int value);
	public native int fndwrite(int value);
	public native int switchread(int fd);
	
	TextView tv;
	TextView feverTextView;
	Button airplaneButton;
	TextView airplaneScoreText;
	Button beautyButton;
	TextView beautyScoreText;
	Button appleButton;
	TextView appleScoreText;
	Button backButton;
	int switch_value;
	int currentIndex = 0;
	int myBestScore[] = {0, 0, 0, 0};
	int score = 0;
	int currentSong = 0;
	int musicSpeed = 500;
	String str = "";
	String interval = "";
	String[] intervalColor = {"#ffffff", "#ff0000", "#ffa500", "#ffff00", "#008000", "#0000ff", "#4b0082", "#ee82ee", "#f25278", "#ffa500"};
	MediaPlayer[] Doremi;
	boolean feverTime = false;
	int consecutive = 0;
	int feverConsecutive = 0;
	
	Handler mHandler=new Handler(){
		public void handleMessage(Message msg){
			if(str.length() == 0){
			
			}
			else if(currentIndex >= str.length()){
				backButton.setVisibility(View.VISIBLE);
			}
			else{
				
				for(int i=0; i<10; i++)
					if(Doremi[i].isPlaying()){
						Doremi[i].pause();
						if(interval.charAt(currentIndex) != i + '0'){
							Doremi[i].seekTo(300);
						}
					}
				
				if(interval.charAt(currentIndex) != '0'){
					Doremi[interval.charAt(currentIndex) - '0'].start();
				}
				
				try{Thread.sleep(70);}
				catch(InterruptedException e){
				}
				
				SpannableStringBuilder ssb = new SpannableStringBuilder(str);
				if(currentIndex != 0)
					ssb.setSpan(new ForegroundColorSpan(Color.parseColor("#00777777")), 0, currentIndex, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				
				for(int i = currentIndex+1; i< str.length(); i++){
					ssb.setSpan(new ForegroundColorSpan(Color.parseColor("#55" + intervalColor[interval.charAt(i)-'0'].substring(1))), i, i+1, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				}
				
				ssb.setSpan(new ForegroundColorSpan(Color.parseColor(intervalColor[msg.arg1])), currentIndex, currentIndex+1, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				ssb.setSpan(new StyleSpan(Typeface.BOLD), currentIndex, currentIndex+1, Spannable.SPAN_EXCLUSIVE_EXCLUSIVE);
				tv.setText(ssb);
				
				if(interval.substring(currentIndex, currentIndex+1).equals(Integer.toString(msg.arg1))){
					if(!feverTime){
						if(interval.charAt(currentIndex) != '0') consecutive += 1;
						if(msg.arg1 != 0) score += 1;
					}
					else{
						consecutive = 8;
						score += 5;
						if(interval.charAt(currentIndex) != '0') feverConsecutive += 1;
					}
					
					if(!feverTime && consecutive >= 8){
						tv.setBackgroundColor(0x55fbf9c2);
						feverTime = true;
						feverTextView.setText("FEVER TIME X2");
					}
					fndwrite(score);
				}
				else{
					consecutive = 0;
					tv.setBackgroundColor(0x00fafafa);
					feverTime = false;
					feverTextView.setText("");
				}
				
				driverwrite(consecutive);
				currentIndex += 1;
				
				if(feverConsecutive == 5){
					tv.setBackgroundColor(0x00fafafa);
					feverTime = false;
					feverConsecutive = 0;
					consecutive = 0;
					feverTextView.setText("");
				}
			}
		}
	};
	class BackThread extends Thread{
		Handler sHandler;
		
		BackThread(Handler handler){
			sHandler=handler;
		}
		public void run(){
			while(true){
				Message msg=Message.obtain();
				msg.what=0;
				
				int switch_fd = switchdriveropen();
				switch_value = switchread(switch_fd);
				
				msg.arg1=switch_value;
				switchdriverclose(switch_fd);
				sHandler.sendMessage(msg);
				try{Thread.sleep(musicSpeed - 70);}
				catch(InterruptedException e){
				}
			}
		}
	}
	
	BackThread mThread;
	Typeface myTypeFace;
	
	private void beforeStartGame(){
		currentIndex = 0;
		
		tv.setText(str);
        feverTextView.setText("");
        
        airplaneButton.setVisibility(View.INVISIBLE);
        beautyButton.setVisibility(View.INVISIBLE);
        appleButton.setVisibility(View.INVISIBLE);
        backButton.setVisibility(View.INVISIBLE);
	}
	
	private void airplaneInit(){
		currentSong = 1;
		str = "준비시작미미미레도도레레미쉼미쉼미쉼레쉼레쉼레쉼미쉼솔쉼솔쉼미미미레도도레레미쉼미쉼미쉼레레쉼레미미레레도도";
        interval = "000033321122303030202020305050333211223030302202332211";
	
        beforeStartGame();
	}
	
	private void beautyInit(){
		currentSong = 2;
		str = "준비시작라라도도라라솔솔라라솔솔파파파쉼파파파레도레파솔라라솔솔파파파쉼파파파레도레파솔라라솔솔라라라쉼라라도도도도쉼쉼쉼쉼쉼쉼쉼쉼라라도도라라솔솔라라솔솔파파";
        interval = "00006688665566554440444212456655444044421245665566606688880000000066886655665544";
        
        beforeStartGame();
	}
	
	private void appleInit(){
		currentSong = 3;
		
		str = "준비시작도레미쉼미쉼파미레레레쉼레미파쉼파쉼솔파미미미쉼미파솔쉼솔쉼됴라솔쉼솔쉼도레미쉼미쉼레레도도도쉼";
		interval = "0000123030432220234040543330345050865050123030221110";
		
		beforeStartGame();
	}
	
	private void init(){
        tv.setText("");
        feverTextView.setText("");
        airplaneScoreText.setText("My best score : 0");
        beautyScoreText.setText("My best score : 0");
        appleScoreText.setText("My best score : 0");
        
        Animation anim = new AlphaAnimation(0.0f, 1.0f);
        anim.setDuration(100);
        anim.setStartOffset(20);
        anim.setRepeatMode(Animation.REVERSE);
        anim.setRepeatCount(Animation.INFINITE);
        feverTextView.startAnimation(anim);
        
        System.loadLibrary("ndk-exam");
    
        driverwrite(0);
        fndwrite(0);
        
        Doremi = new MediaPlayer[10];
        Doremi[0] = MediaPlayer.create(this, R.raw.re);
        Doremi[1] = MediaPlayer.create(this, R.raw.dol);
        Doremi[2] = MediaPlayer.create(this, R.raw.re);
        Doremi[3] = MediaPlayer.create(this, R.raw.mi);
        Doremi[4] = MediaPlayer.create(this, R.raw.fa);
        Doremi[5] = MediaPlayer.create(this, R.raw.sol);
        Doremi[6] = MediaPlayer.create(this, R.raw.la);
        Doremi[7] = MediaPlayer.create(this, R.raw.si);
        Doremi[8] = MediaPlayer.create(this, R.raw.high_dol);
        Doremi[9] = MediaPlayer.create(this, R.raw.high_re);
	
        myTypeFace = Typeface.createFromAsset(getAssets(), "fonts/jalnan.ttf");
        
        TextView title = (TextView)findViewById(R.id.textView1);
        title.setTypeface(myTypeFace);
        airplaneButton.setTypeface(myTypeFace);
        appleButton.setTypeface(myTypeFace);
        beautyButton.setTypeface(myTypeFace);
        backButton.setTypeface(myTypeFace);
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        tv = (TextView)findViewById(R.id.str_text);
        feverTextView = (TextView)findViewById(R.id.fever_text);
        airplaneScoreText = (TextView)findViewById(R.id.airplaneScoreText);
        beautyScoreText = (TextView)findViewById(R.id.beautyScore);
        appleScoreText = (TextView)findViewById(R.id.appleScore);
        
        backButton = (Button)findViewById(R.id.backButton);
        airplaneButton = (Button)findViewById(R.id.airplaneButton);
        beautyButton = (Button)findViewById(R.id.beautyButton);
        appleButton = (Button)findViewById(R.id.appleButton);        
        init();
        
        mThread=new BackThread(mHandler);
		mThread.setDaemon(true);
		mThread.start();
        
        airplaneButton.setOnClickListener(new View.OnClickListener() {
        	
			@Override
			public void onClick(View v) {
				airplaneInit();
			}
		});
        
        beautyButton.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				beautyInit();
			}
		});
        
        appleButton.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				appleInit();
			}
		});

        
        backButton.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				feverTime = false;
				consecutive = 0;
				feverConsecutive = 0;
				driverwrite(0);
				for(int i=0; i<10; i++){
					if(Doremi[i].isPlaying())
						Doremi[i].pause();
				}
				fndwrite(0);
				str = "";
				tv.setBackgroundColor(0x00fafafa);
				backButton.setVisibility(View.INVISIBLE);
				airplaneButton.setVisibility(View.VISIBLE);
				beautyButton.setVisibility(View.VISIBLE);
				appleButton.setVisibility(View.VISIBLE);
				tv.setText("");
				feverTextView.setText("");
				if(score > myBestScore[currentSong]){
					myBestScore[currentSong] = score;
					if(currentSong == 1) airplaneScoreText.setText("My best score : "+myBestScore[1]);
					else if(currentSong == 2) beautyScoreText.setText("My best score : "+myBestScore[2]);
					else if(currentSong == 3) appleScoreText.setText("My best score : "+myBestScore[3]);
				}
				score = 0;
			}
		});
        
        SeekBar speedBar = (SeekBar)findViewById(R.id.seekBar);
        speedBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
			
			@Override
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			@Override
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				// TODO Auto-generated method stub
				musicSpeed = 400 + progress;
			}
		});
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	
	@Override
	protected void onDestroy(){
		for(int i=0; i<10; i++)
			if(Doremi[i].isPlaying())
				Doremi[i].pause();
		str="";
		super.onDestroy();
	}

}
