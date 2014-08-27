package icefox.youproxy;

import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InterruptedIOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.SocketException;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.Toast;

public class MainActivity extends Activity{
	private String basedir=null;  
	private DatagramSocket serviceSocket;
	static CheckBox cbEnable;
	

	private static Handler handler = new Handler() {
		@Override
		public synchronized void handleMessage(Message msg) {
			if(msg.what == 0){
				cbEnable.setChecked(false);
			}else if(msg.what == 1){
				cbEnable.setChecked(true);
			}
			super.handleMessage(msg);
		}
	};
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);
		Log.d("Y", "YouProxy 1.0");
		cbEnable = (CheckBox)this.findViewById(R.id.cbEnable);
   
		try { 
			basedir = getBaseContext().getFilesDir().getAbsolutePath();
		} catch (Exception e) {} 

		copyfile("redsocks");
		copyfile("redsocks.conf");
		copyfile("iptables");
		copyfile("youagent");
		 

		this.startService(new Intent(this, BGService.class));
		try { 
			serviceSocket = new DatagramSocket();
		} catch (SocketException e) {  
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		
		cbEnable.setOnClickListener(new OnClickListener(){
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				if(cbEnable.isChecked()){
					startServer();
				}else{
					try {
						sendQuit();
					} catch (Exception e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				} 
			} 
			
		});
	}
	 
	public final String getBaseDir(){
		return this.basedir;
	}
	
	public void startServer(){
		try{
			if(this.checkProcessAlive("youagent")){
				this.sendQuit();
				Thread.sleep(1000);
			}
			
			Log.d("Y", "Starting youagent process...");
			Runtime rt = Runtime.getRuntime();
			Process serviceProcess = rt.exec(new String[]{"su","-c", basedir + "/youagent"});
			Thread.sleep(300);
			try{
				serviceProcess.exitValue();
				if(!this.checkProcessAlive("youagent")){
					cbEnable.setChecked(false);
					warn("无法成功启动YouAgent进程。");
				}
			}catch(Exception e){
				// process is still running
			}
			info("已开启YouAgent进程。");
		}catch(Exception e){
			e.printStackTrace();
		}
	}

	public void copyfile(String file) {
		String of = file;
		File f = new File(of);

		if (!f.exists()) {
			try {
				InputStream in = getAssets().open(file);
				
				FileOutputStream out = getBaseContext().openFileOutput(of, MODE_PRIVATE);

				byte[] buf = new byte[1024];
				int len; 
				while ((len = in.read(buf)) > 0) {
					out.write(buf, 0, len);
				}
				out.close();
				in.close();
				Runtime.getRuntime().exec("chmod 777 " + basedir + "/" + of);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}


	private void warn(String message){
		AlertDialog.Builder builder = new Builder(this);
		builder.setMessage(message);
		builder.setTitle(getText(R.string.app_name));
		builder.setPositiveButton(
				"Warn",
				new android.content.DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int which) {
						dialog.dismiss();
					}
				});
		builder.create().show();
	}
	
	private void info(String info){
		Toast t2 = Toast.makeText(getApplicationContext(),
				info, Toast.LENGTH_SHORT);
		t2.setGravity(Gravity.CENTER, 0, 0);
		t2.show();
	}
	  
	private void sendQuit() throws Exception{
		final String data = "quit";
		new Thread(){
			@Override
			public void run(){
				DatagramPacket pack = new DatagramPacket(data.getBytes(), data.length());
				try {
					pack.setAddress(InetAddress.getByName("127.0.0.1"));
					pack.setPort(0x1998);
					serviceSocket.send(pack);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}.start();
	}
	  
	private void sendNoop() throws Exception{
		final String data = "noop";
		new Thread(){
			@Override
			public void run(){
				DatagramPacket pack = new DatagramPacket(data.getBytes(), data.length());
				try {
					pack.setAddress(InetAddress.getByName("127.0.0.1"));
					pack.setPort(0x1998);
					serviceSocket.send(pack);
					serviceSocket.setSoTimeout(2000);
					try{
						serviceSocket.receive(pack);
						Log.d("Y", "Receive packet from service");
						handler.sendEmptyMessage(1);
					}catch(InterruptedIOException ee){
						handler.sendEmptyMessage(0);
					}
				} catch (Exception e) {
					// TODO Auto-generated catch block
					handler.sendEmptyMessage(0);
					e.printStackTrace();
				}
			}
		}.start();
	}

	public boolean checkProcessAlive(String name) {
		File[] list = new File("/proc").listFiles();
		for(File sub : list){
			if(!sub.isDirectory())
				continue;
			File cmdline = new File(sub.getAbsolutePath() + "/cmdline");
			if(cmdline == null || !cmdline.exists())
				continue;
			try {
				FileInputStream fis = new FileInputStream(cmdline);
				DataInputStream dis = new DataInputStream(fis);
				String line = dis.readLine();
				if(line != null){
					if(line.contains(name))
						return true;
				}
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
		}
		return false;
	}
	
	@Override
	public void onStart(){
		super.onStart();
		try {
			this.sendNoop();
		} catch (Exception e) { 
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	// Back button
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		//
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			if(!cbEnable.isChecked()){
				stopService(new Intent(MainActivity.this, BGService.class));
				MainActivity.this.finish();
				return true;
			}
			
			AlertDialog.Builder builder = new Builder(MainActivity.this);
			builder.setMessage(getText(R.string.close_alert));
			builder.setTitle(getText(R.string.app_name));
			builder.setPositiveButton("Yes",
					new android.content.DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					//android.os.Process.killProcess(android.os.Process.myPid());
					dialog.dismiss();
					stopService(new Intent(MainActivity.this, BGService.class));
					MainActivity.this.finish();
				}
			});
			builder.setNegativeButton("No",
					new android.content.DialogInterface.OnClickListener() {
				public void onClick(DialogInterface dialog, int which) {
					dialog.dismiss();
				}
			});
			builder.create().show();
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
}
