package org.example.ndk;

//import com.example.androidex.MainActivity.BackThread;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.view.Menu;
import android.widget.TextView;

public class NDKExam extends Activity {
	public native int add(int x, int y);
	public native int driveropen();
	public native int switchdriveropen();
	public native int driverclose(int fd);
	public native int switchdriverclose(int fd);
	public native int driverwrite(int fd, int mode, String value);
	public native int switchread(int fd);
	public native void testString(String str);
	
	TextView tv;
	int switch_value;
	
	Handler mHandler=new Handler(){
		public void handleMessage(Message msg){
			;
			//nothing done
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
				switchdriverclose(switch_fd);
				msg.arg1=switch_value;
				sHandler.sendMessage(msg);
				try{Thread.sleep(100);}catch(InterruptedException e){;}
			}
		}
	}
	
	BackThread mThread;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.main);
        
        tv = new TextView(this);
        
        System.loadLibrary("ndk-exam");
        
        String str = "";
        
        tv.setText("The multiplication of ");
        setContentView(tv);
        
        int fd = driveropen();
        driverwrite(fd, 20, "100");
        driverclose(fd);

        testString("test");
        
        mThread=new BackThread(mHandler);
		mThread.setDaemon(true);
		mThread.start();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
