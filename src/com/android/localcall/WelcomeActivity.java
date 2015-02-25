package com.android.localcall;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;

public class WelcomeActivity extends BaseActivity
{
	//延时启动
	private final int DELAY_TIME = 1000;
	//启动主界面事件
	private final int EVENT_START_MAIN = 100;
	private Handler mHandler = new Handler()
	{
		@Override
		public void handleMessage(Message msg)
		{
			switch (msg.what)
			{
				case EVENT_START_MAIN:
				{
					finish();
					Intent intent = new Intent();
					intent.setClass(WelcomeActivity.this, MainActivity.class);
					startActivity(intent);
				}
					break;
				default:
					break;
			}
			super.handleMessage(msg);
		}

	};

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		setContentView(R.layout.activity_welcome);
		super.onCreate(savedInstanceState);

	}

	@Override
	protected void onPause()
	{
		mHandler.removeMessages(EVENT_START_MAIN);
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		//延时启动
		mHandler.sendEmptyMessageDelayed(EVENT_START_MAIN, DELAY_TIME);
		super.onResume();
	}

}
