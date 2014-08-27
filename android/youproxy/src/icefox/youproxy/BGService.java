package icefox.youproxy;

import android.app.Notification;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;


public class BGService extends Service {
	int counter = 1;
 
	@Override
	public IBinder onBind(Intent intent) {
		// TODO Auto-generated method stub
		return null;
	}
  
	@Override
	public void onCreate(){
		super.onCreate();
	}
	
	@Override
    public int onStartCommand(Intent intent, int flags, int startId) {
		final int myID = 151;

		//The intent to launch when the user clicks the expanded notification
		Intent intent1 = new Intent(this, MainActivity.class);
		intent1.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP);
		PendingIntent pendIntent = PendingIntent.getActivity(this, 0, intent1, 0);

		//This constructor is deprecated. Use Notification.Builder instead
		Notification notice = new Notification(R.drawable.ic_launcher, getText(R.string.app_name), System.currentTimeMillis());

		//This method is deprecated. Use Notification.Builder instead.
		notice.setLatestEventInfo(this, getText(R.string.app_name), getText(R.string.show_running), pendIntent);

		notice.flags |= Notification.FLAG_NO_CLEAR;
		startForeground(myID, notice);

        return super.onStartCommand(intent, flags, startId);
    }
	
	@Override
	public void onStart(Intent intent, int startId){
		super.onStart(intent, startId);
		Log.d("Y", "service onStart times " + counter);
		counter += 1;
	}
	
	@Override
	public void onDestroy(){
		super.onDestroy();
		Log.d("Y", "service onDestory");
	}
}
