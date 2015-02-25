package com.android.localcall;

import com.android.localcall.thirdpart.BaiduStatisticController;

import android.app.Activity;

public class BaseActivity extends Activity
{

	@Override
	protected void onPause()
	{
		BaiduStatisticController.onPause(this);
		super.onPause();
	}

	@Override
	protected void onResume()
	{
		BaiduStatisticController.onResume(this);
		super.onResume();
	}

}
