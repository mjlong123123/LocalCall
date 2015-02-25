package com.android.localcall.utils;

import java.util.List;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.NetworkInfo.DetailedState;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.Looper;

import com.android.localcall.utils.CustomTimer.TimerCallback;

public class WifiAdmin
{
	private static final String TAG = "WifiAdmin";

	private static final int TYPE_NO_PASSWD = 0x11;
	private static final int TYPE_WEP = 0x12;
	private static final int TYPE_WPA = 0x13;

	//记录原来的wifi状态
	private static boolean mOriginalOpen = false;

	private WifiManager mWifiManager;

	private ConnectivityManager mConnectivityManager;

	private static int mWcgID = -1;

	private CustomTimer mCustomTimer = null;

	public WifiAdmin(Context context)
	{
		// 取得WifiManager对象  
		mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);

		mConnectivityManager = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
	}

	/**
	 * 连接热点
	 * 
	 * @param ssid
	 *            用户名
	 * @param pass
	 *            密码
	 * @return
	 */
	public void connectWifi(String ssid, String pass, final ConnectStateCallback callback)
	{
		DEBUG.log(TAG, "connectWifi ssid:" + ssid + " pass:" + pass);

		if (!mWifiManager.isWifiEnabled())
		{
			mOriginalOpen = false;

			mWifiManager.setWifiEnabled(true);
		}
		else
		{
			mOriginalOpen = true;
		}

		WifiConfiguration wc = createWifiInfo(ssid, pass, TYPE_WPA);

		mWcgID = mWifiManager.addNetwork(wc);

		boolean ret = mWifiManager.enableNetwork(mWcgID, true);

		if (ret)
		{
			DEBUG.log(TAG, "connectWifi success");
		}
		else
		{
			DEBUG.log(TAG, "connectWifi failed");
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

				if (isWifiContected() == WIFI_CONNECTED)
				{
					DEBUG.log(TAG, "checkBreakFlag2");

					new Handler(Looper.getMainLooper()).post(new Runnable()
					{

						@Override
						public void run()
						{
							if (callback != null)
							{
								callback.onConnectApSuccess();
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
						if (callback != null)
						{
							callback.onConnectApFailed();
						}
					}
				});
			}

		});
	}

	/**
	 * 断开热点连接
	 */
	public void disconnectWifi()
	{
		boolean ret = false;

		ret = mWifiManager.enableNetwork(mWcgID, false);

		if (ret)
		{
			DEBUG.log(TAG, "connectWifi success");
		}
		else
		{
			DEBUG.log(TAG, "connectWifi failed");
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
	 * 根据ssid删除wifi配置
	 * 
	 * @param ssid
	 */
	private void removeWifiConfigurationBySsid(String ssid)
	{
		WifiConfiguration tempConfig = getWifiConfigurationBySsid(ssid);

		if (tempConfig != null)
		{
			mWifiManager.removeNetwork(tempConfig.networkId);
		}
	}

	/**
	 * 创建配置
	 * 
	 * @param SSID
	 * @param password
	 * @param type
	 * @return
	 */
	private WifiConfiguration createWifiInfo(String SSID, String password, int type)
	{

		DEBUG.log(TAG, "createWifiInfo SSID = " + SSID + "## Password = " + password + "## Type = " + type);

		removeWifiConfigurationBySsid(SSID);

		WifiConfiguration config = new WifiConfiguration();
		config.allowedAuthAlgorithms.clear();
		config.allowedGroupCiphers.clear();
		config.allowedKeyManagement.clear();
		config.allowedPairwiseCiphers.clear();
		config.allowedProtocols.clear();
		config.SSID = "\"" + SSID + "\"";

		// 分为三种情况：1没有密码2用wep加密3用wpa加密  
		if (type == TYPE_NO_PASSWD)
		{// WIFICIPHER_NOPASS  
			config.wepKeys[0] = "";
			config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
			config.wepTxKeyIndex = 0;

		}
		else if (type == TYPE_WEP)
		{ //  WIFICIPHER_WEP   
			config.hiddenSSID = true;
			config.wepKeys[0] = "\"" + password + "\"";
			config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.SHARED);
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP40);
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.WEP104);
			config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.NONE);
			config.wepTxKeyIndex = 0;
		}
		else if (type == TYPE_WPA)
		{ // WIFICIPHER_WPA  
			config.preSharedKey = "\"" + password + "\"";
			config.hiddenSSID = true;
			config.allowedAuthAlgorithms.set(WifiConfiguration.AuthAlgorithm.OPEN);
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.TKIP);
			config.allowedKeyManagement.set(WifiConfiguration.KeyMgmt.WPA_PSK);
			config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.TKIP);
			// config.allowedProtocols.set(WifiConfiguration.Protocol.WPA);  
			config.allowedGroupCiphers.set(WifiConfiguration.GroupCipher.CCMP);
			config.allowedPairwiseCiphers.set(WifiConfiguration.PairwiseCipher.CCMP);
			config.status = WifiConfiguration.Status.ENABLED;
		}

		return config;
	}

	public static final int WIFI_CONNECTED = 0x01;
	public static final int WIFI_CONNECT_FAILED = 0x02;
	public static final int WIFI_CONNECTING = 0x03;

	/**
	 * 判断wifi是否连接成功,不是network
	 * 
	 * @param context
	 * @return
	 */
	private int isWifiContected()
	{
		NetworkInfo wifiNetworkInfo = mConnectivityManager.getNetworkInfo(ConnectivityManager.TYPE_WIFI);

		DEBUG.log(TAG, "isConnectedOrConnecting = " + wifiNetworkInfo.isConnectedOrConnecting());
		DEBUG.log(TAG, "wifiNetworkInfo.getDetailedState() = " + wifiNetworkInfo.getDetailedState());

		if (wifiNetworkInfo.getDetailedState() == DetailedState.OBTAINING_IPADDR || wifiNetworkInfo.getDetailedState() == DetailedState.CONNECTING)
		{
			return WIFI_CONNECTING;
		}
		else if (wifiNetworkInfo.getDetailedState() == DetailedState.CONNECTED)
		{
			return WIFI_CONNECTED;
		}
		else
		{
			return WIFI_CONNECT_FAILED;
		}
	}

	/**
	 * 通过用户名获取配置
	 * 
	 * @param SSID
	 * @return
	 */
	private WifiConfiguration getWifiConfigurationBySsid(String SSID)
	{
		List<WifiConfiguration> existingConfigs = mWifiManager.getConfiguredNetworks();

		for (WifiConfiguration existingConfig : existingConfigs)
		{
			if (existingConfig.SSID.equals("\"" + SSID + "\""))
			{
				return existingConfig;
			}
		}

		return null;
	}

	/**
	 * @author chenjiulong 热点开启回调
	 */
	public static interface ConnectStateCallback
	{
		void onConnectApSuccess();

		void onConnectApFailed();
	}
}
