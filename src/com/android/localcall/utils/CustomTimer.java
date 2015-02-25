package com.android.localcall.utils;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;

/**
 * @author chenjiulong 定时器
 */
public class CustomTimer
{
	private static final String TAG = "CustomTimer";

	private static final int MESSAGE_1 = 100;
	
	private static final int DELAY_TIME = 3000;

	private int mTiles = 5;
	
	private TimerCallback mTimerCallback = null;

	private HandlerThread mHandlerThread = null;
	
	private Handler mHandler = null;

	public CustomTimer(int times)
	{
		mTiles = times;
		mHandlerThread = new HandlerThread("CustomTimer");
		mHandlerThread.start();
		mHandler = new Handler(mHandlerThread.getLooper())
		{

			@Override
			public void handleMessage(Message msg)
			{
				if (mTiles <= 0)
				{
					DEBUG.log(TAG, "timer over1");
					if(mTimerCallback != null)
					{
						mTimerCallback.checkBreakFlagOver();
					}
					mHandlerThread.quit();
					return;
				}

				mTiles--;

				if (mTimerCallback != null && mTimerCallback.checkBreakFlag())
				{
					DEBUG.log(TAG, "timer over2");
					mHandlerThread.quit();
					return;
				}

				mHandler.sendEmptyMessageDelayed(MESSAGE_1, DELAY_TIME);
			}

		};
	}

	public void startTimer(TimerCallback callback)
	{
		mTimerCallback = callback;
		mHandler.sendEmptyMessageDelayed(MESSAGE_1, DELAY_TIME);
	}

	public void stopTimer()
	{
		if(mTimerCallback != null)
		{
			mTimerCallback.checkBreakFlagOver();
		}
		mHandler.removeMessages(100);
		mHandlerThread.quit();
	}

	public interface TimerCallback
	{
		boolean checkBreakFlag();
		void checkBreakFlagOver();
	}
}
