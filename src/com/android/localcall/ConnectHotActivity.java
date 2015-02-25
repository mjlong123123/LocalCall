package com.android.localcall;

import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.localcall.service.LocalCallServiceMgr;
import com.android.localcall.utils.DEBUG;
import com.android.localcall.utils.WifiAdmin;
import com.android.localcall.utils.WifiAdmin.ConnectStateCallback;

public class ConnectHotActivity extends BaseActivity implements View.OnClickListener
{
	private static final String TAG = "ConnectHotActivity";

	private WifiAdmin mWifiAdmin = null;

	private boolean mError = false;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_hot);

		mWifiAdmin = new WifiAdmin(ConnectHotActivity.this);

		mWifiAdmin.connectWifi("HotSpot", "hhhhhh123", new ConnectStateCallback()
		{

			@Override
			public void onConnectApSuccess()
			{
				DEBUG.log(TAG, "connect onConnectApSuccess");

				Toast.makeText(ConnectHotActivity.this, "连接成功", Toast.LENGTH_SHORT).show();

				finish();
			}

			@Override
			public void onConnectApFailed()
			{
				DEBUG.log(TAG, "connect onConnectApFailed");

				Toast.makeText(ConnectHotActivity.this, "连接失败", Toast.LENGTH_SHORT).show();

				mError = true;

				finish();
			}
		});

		init();
	}

	@Override
	public void onClick(View v)
	{
		switch (v.getId())
		{
			case R.id.button1:

				mError = true;

				finish();

				break;

			default:
				break;
		}
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		switch (keyCode)
		{
			case KeyEvent.KEYCODE_BACK:

				mError = true;

			default:
				break;
		}
		return super.onKeyDown(keyCode, event);
	}

	private void init()
	{
		Button button = (Button) findViewById(R.id.button1);
		TextView text = (TextView) findViewById(R.id.text1);
		text.setText("正在连接热点");
		button.setOnClickListener(this);
	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();

		if (mWifiAdmin != null && mError)
			mWifiAdmin.disconnectWifi();
	}

	@Override
	protected void onPause()
	{
		LocalCallServiceMgr.restartCtlServer();
		
		super.onPause();
	}

}
