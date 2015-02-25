package com.android.localcall;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.android.localcall.service.LocalCallService;
import com.android.localcall.service.LocalCallService.ICallBack;
import com.android.localcall.service.LocalCallServiceMgr;
import com.android.localcall.utils.Utils;

public class CallActivity extends BaseActivity
{
	private final static String TAG = "CallActivity";

	public final static String CONNECT_TYPE = "CONNECT_TYPE";
	public final static String CONNECT_IP = "CONNECT_IP";
	public final static String CONNECT_PORT = "CONNECT_PORT";
	//控制的客户端
	private boolean mIsClient = false;

	private String mCtlIp = "";
	private String mConnectIp = "";
	private int mPort = 0;

	private ViewGroup mRootView = null;

	private TextView mMsg = null;

	private View mTalking = null;
	
	private boolean mCanTalk = true;

	private ICallBack mICallBack = new ICallBack()
	{
		@Override
		public void callBack(int code, String arg1, String arg2)
		{
			switch (code)
			{
				case LocalCallService.RETURN_CODE_NEW_COMMAND:
					
					Log.v(TAG, "arg1:" + arg1);

					if (arg1.contains("socket connected"))
					{
						LocalCallServiceMgr.writeCommandFromClient("socket connected:"+Utils.getLocalIpAddress(CallActivity.this) + ":" + mPort);
						
						if(mMsg != null)
						{
							mMsg.setText(R.string.press_talk);
						}
					}
					else if (arg1.contains("socket disconnected"))
					{
						Toast.makeText(CallActivity.this, "Socket connected ", Toast.LENGTH_SHORT).show();
						if(mMsg != null)
						{
							mMsg.setText("已经从服务端断开");
						}
						CallActivity.this.finish();
					}
					else if (arg1.contains(":"))
					{
						String[] rets = arg1.split(":");
						if (rets != null && rets.length == 2)
						{
							String ip = rets[0];
							int port = Integer.valueOf(rets[1]);

						}
					}
					else if(arg1.contains("down"))
					{
						if(mMsg != null)
						{
							mMsg.setText("对方正在讲话");
							mCanTalk = false;
						}
					}
					else if(arg1.contains("up"))
					{
						if(mMsg != null)
						{
							mMsg.setText(R.string.press_talk);
							mCanTalk = true;
						}
					}
					break;
				default:
					break;
			}
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		setContentView(R.layout.activity_call);

		initIntent();

		initView();

		super.onCreate(savedInstanceState);
	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();
	}

	@Override
	protected void onPause()
	{
		LocalCallServiceMgr.stopPlayer();

		if (mIsClient)
		{
			LocalCallServiceMgr.disconnectServer();
		}
		else
		{
			LocalCallServiceMgr.writeCommandFromServer("socket disconnected");
		}
		
		//注销监听
		LocalCallServiceMgr.registerCallback(null);

		super.onPause();
	}

	@Override
	protected void onResume()
	{

		//注册监听
		LocalCallServiceMgr.registerCallback(mICallBack);

		if (mIsClient)
		{
			LocalCallServiceMgr.connectServer(mConnectIp, mPort);
		}
		
		LocalCallServiceMgr.startPlayer();

		super.onResume();
	}

	/**
	 * 获取intent信息
	 */
	private void initIntent()
	{
		Intent intent = getIntent();
		if (intent != null)
		{
			mIsClient = intent.getBooleanExtra(CONNECT_TYPE, false);
			mConnectIp = intent.getStringExtra(CONNECT_IP);
			mPort = intent.getIntExtra(CONNECT_PORT, 0);
		}
	}

	/**
	 * 初始化view
	 */
	private void initView()
	{
		mRootView = (ViewGroup) findViewById(R.id.main);
		mRootView.setOnTouchListener(new OnTouchListener()
		{
			@Override
			public boolean onTouch(View arg0, MotionEvent arg1)
			{
				switch (arg1.getAction())
				{
					case MotionEvent.ACTION_DOWN:
						Log.v(TAG, "ACTION_DOWN mConnectIp:" + mConnectIp);
						if(!mCanTalk)
						{
							return false;
						}
						LocalCallServiceMgr.startRecorder(mConnectIp);
						mTalking.setVisibility(View.VISIBLE);
						mMsg.setText(CallActivity.this.getString(R.string.pressed_talk, mConnectIp));
						if (mIsClient)
							LocalCallServiceMgr.writeCommandFromClient("down");
						else
							LocalCallServiceMgr.writeCommandFromServer("down");
						break;
					case MotionEvent.ACTION_UP:
						Log.v(TAG, "ACTION_UP");
//						LocalCallServiceMgr.stopRecorder();
//						mTalking.setVisibility(View.GONE);
//						mMsg.setText(R.string.press_talk);
//						break;
					case MotionEvent.ACTION_CANCEL:
						Log.v(TAG, "ACTION_CANCEL");
						LocalCallServiceMgr.stopRecorder();
						mTalking.setVisibility(View.GONE);
						mMsg.setText(R.string.press_talk);
						if (mIsClient)
							LocalCallServiceMgr.writeCommandFromClient("up");
						else
							LocalCallServiceMgr.writeCommandFromServer("up");
						break;
				}
				return true;
			}
		});
		mMsg = (TextView) findViewById(R.id.msg);
		mMsg.setText(R.string.press_talk);
		
		if(mMsg != null)
		{
			if(mIsClient)
			{
				mMsg.setText("正在呼叫");
			}
		}
		
		mTalking = findViewById(R.id.talking);
		mTalking.setVisibility(View.GONE);
	}
}
