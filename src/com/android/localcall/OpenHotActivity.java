package com.android.localcall;

import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.android.localcall.service.LocalCallServiceMgr;
import com.android.localcall.utils.DEBUG;
import com.android.localcall.utils.WifiApAdmin;
import com.android.localcall.utils.WifiApAdmin.ApStateCallback;

public class OpenHotActivity extends BaseActivity implements View.OnClickListener
{
	private static final String TAG = "OpenHotActivity";

	private WifiApAdmin mWifiApAdmin = null;

	private boolean mError = false;

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_hot);

		mWifiApAdmin = new WifiApAdmin(this);
		mWifiApAdmin.startWifiAp("HotSpot", "hhhhhh123", new ApStateCallback()
		{
			@Override
			public void onOpen()
			{
				DEBUG.log(TAG, "wifiap open");
				Toast.makeText(OpenHotActivity.this, "开启热点成功", Toast.LENGTH_SHORT).show();
				finish();
			}

			@Override
			public void onOpenError()
			{
				DEBUG.log(TAG, "wifiap onOpenError");

				Toast.makeText(OpenHotActivity.this, "开启热点失败", Toast.LENGTH_SHORT).show();

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
		button.setOnClickListener(this);
	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();

		if (mWifiApAdmin != null && mError)
			mWifiApAdmin.stopWifiAp();
	}

	@Override
	protected void onPause()
	{
		LocalCallServiceMgr.restartCtlServer();
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		// TODO Auto-generated method stub
		super.onResume();
	}

}
