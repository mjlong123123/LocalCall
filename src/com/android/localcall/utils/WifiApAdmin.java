package com.android.localcall.utils;

import java.lang.reflect.Method;

import android.content.Context;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Looper;

import com.android.localcall.utils.CustomTimer.TimerCallback;

/**
 * 创建热点
 * 
 */
public class WifiApAdmin
{
	private static final String TAG = "WifiApAdmin";

	private CustomTimer mCustomTimer = null;

	//记录原来的wifi状态
	private static boolean mOriginalOpen = false;
	//控制热点开启关闭
	private Method mSetWifiApEnabledMethod = null;
	//获取热点配置
	private Method mGetWifiApConfigurationMethod = null;
	//判断热点是否开启
	private Method mIsWifiApEnabledMethod = null;

	private WifiManager mWifiManager = null;

	public WifiApAdmin(Context context)
	{
		mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
		init();
	}

	/**
	 * 初始化接口
	 */
	private void init()
	{
		try
		{
			mSetWifiApEnabledMethod = mWifiManager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, boolean.class);
			mGetWifiApConfigurationMethod = mWifiManager.getClass().getMethod("getWifiApConfiguration");
			mIsWifiApEnabledMethod = mWifiManager.getClass().getMethod("isWifiApEnabled");
		}
		catch (NoSuchMethodException e)
		{
			DEBUG.log(TAG, "init e：" + e);
		}
	}

	/**
	 * 开启热点
	 * 
	 * @param ssid
	 *            热点名称
	 * @param passwd
	 *            热点密码
	 */
	public void startWifiAp(String ssid, String passwd, final ApStateCallback callback)
	{
		DEBUG.log(TAG, "startWifiAp mSSID:" + ssid + " mPasswd:" + passwd);

		if (mWifiManager.isWifiEnabled())
		{
			mOriginalOpen = true;
			boolean ret = mWifiManager.setWifiEnabled(false);

			DEBUG.log(TAG, "close wifi ret:" + ret);
		}
		else
		{
			mOriginalOpen = false;
		}
		//开启定时器
		if (mCustomTimer != null)
		{
			mCustomTimer.stopTimer();
		}
		mCustomTimer = new CustomTimer(7);
		mCustomTimer.startTimer(new TimerCallback()
		{
			@Override
			public boolean checkBreakFlag()
			{
				DEBUG.log(TAG, "checkBreakFlag");
				
				if (isWifiApEnabled())
				{
					DEBUG.log(TAG, "checkBreakFlag2");
					
					new Handler(Looper.getMainLooper()).post(new Runnable()
					{
						
						@Override
						public void run()
						{
							if(callback != null)
							{
								callback.onOpen();
							}
						}
					});
					return true;
				}
				return false;
			}

			@Override
			public void checkBreakFlagOver()
			{
				new Handler(Looper.getMainLooper()).post(new Runnable()
				{
					
					@Override
					public void run()
					{
						if(callback != null)
						{
							callback.onOpenError();
						}
					}
				});
			}
			
		});

		stratWifiAp(ssid, passwd);
	}

	/**
	 * 关闭热点
	 */
	public void stopWifiAp()
	{
		DEBUG.log(TAG, "stopWifiAp start");
		//开启定时器
		if (mCustomTimer != null)
		{
			mCustomTimer.stopTimer();
		}
		try
		{
			mGetWifiApConfigurationMethod.setAccessible(true);

			WifiConfiguration config = (WifiConfiguration) mGetWifiApConfigurationMethod.invoke(mWifiManager);

			mSetWifiApEnabledMethod.invoke(mWifiManager, config, false);
		}
		catch (Exception e)
		{
			DEBUG.log(TAG, "stopWifiAp e:" + e);
		}

		if (mOriginalOpen && !mWifiManager.isWifiEnabled())
		{
			mWifiManager.setWifiEnabled(true);
			DEBUG.log(TAG, "stopWifiAp open wifi");
		}
		else
		{
			DEBUG.log(TAG, "stopWifiAp not open wifi");
		}
	}

	/**
	 * 开启热点
	 */
	private void stratWifiAp(String ssid, String passwd)
	{
		if (mSetWifiApEnabledMethod == null || mWifiManager == null)
		{
			DEBUG.log(TAG, "stratWifiAp mSetWifiApEnabledMethod:" + mSetWifiApEnabledMethod + " mWifiManager:" + mWifiManager);
			return;
		}
		try
		{
			WifiConfiguration netConfig = new WifiConfiguration();
			netConfig.SSID = ssid;
			netConfig.preSharedKey = passwd;
			netConfig.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
			netConfig.allowedProtocols.set(WifiConfiguration.Protocol.RSN);
			netConfig.allowedProtocols.set(WifiConfiguration.Protocol.WPA);
			netConfig.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
			netConfig.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
			netConfig.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
			netConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
			netConfig.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);

			mSetWifiApEnabledMethod.invoke(mWifiManager, netConfig, true);
		}
		catch (Exception e)
		{
			DEBUG.log(TAG, "stratWifiAp e:" + e);
		}
	}

	/**
	 * 判断热点是否打开
	 * 
	 * @return
	 */
	private boolean isWifiApEnabled()
	{
		if (mIsWifiApEnabledMethod == null || mWifiManager == null)
		{
			DEBUG.log(TAG, "isWifiApEnabled mSetWifiApEnabledMethod:" + mSetWifiApEnabledMethod + " mWifiManager:" + mWifiManager);
			return false;
		}

		try
		{
			mIsWifiApEnabledMethod.setAccessible(true);
			return (Boolean) mIsWifiApEnabledMethod.invoke(mWifiManager);
		}
		catch (Exception e)
		{
			DEBUG.log(TAG, "isWifiApEnabled e:" + e);
		}
		return false;
	}

	/**
	 * @author chenjiulong 热点开启回调
	 */
	public static interface ApStateCallback
	{
		void onOpen();
		void onOpenError();
	}

}
