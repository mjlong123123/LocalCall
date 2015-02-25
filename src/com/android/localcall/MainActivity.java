package com.android.localcall;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.security.MessageDigest;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.LinkedHashMap;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.net.wifi.WifiConfiguration;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.localcall.service.LocalCallService;
import com.android.localcall.service.LocalCallService.ICallBack;
import com.android.localcall.service.LocalCallServiceMgr;
import com.android.localcall.thirdpart.BaiduStatisticController;
import com.android.localcall.utils.DEBUG;
import com.android.localcall.utils.Utils;
import com.android.localcall.utils.WifiAdmin;
import com.android.localcall.utils.WifiApAdmin;

public class MainActivity extends BaseActivity
{
	private final static String TAG = "MainActivity";
	//用户图片
	private ImageView mMyIcon = null;
	//本地ip
	private TextView mMyText1 = null;
	//本地用户名
	private TextView mMyText2 = null;
	//局域网用户列表
	private ListView mListView = null;
	//用户列表数据
	private UserListAdapter mUserListAdapter = null;
	//过滤ip列表
	private LinkedHashMap<String, String> mIPMap = new LinkedHashMap<String, String>();
	//保存ip 列表
	private ArrayList<String> mIPList = new ArrayList<String>();

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		mIPList.add("通过手动拨号呼叫");

		initView();
		//连接服务
		LocalCallServiceMgr.bindILocalCallService(this);
	}

	@Override
	protected void onDestroy()
	{
		super.onDestroy();

		LocalCallServiceMgr.unbindILocalCallService(this);

		WifiApAdmin mWifiApAdmin = new WifiApAdmin(this);
		mWifiApAdmin.stopWifiAp();

		WifiAdmin mWifiAdmin = new WifiAdmin(this);
		mWifiAdmin.disconnectWifi();
	}

	@Override
	protected void onPause()
	{
		//断开服务
		LocalCallServiceMgr.registerCallback(null);
		//停止发送组播
		LocalCallServiceMgr.stopBroadcast();

		super.onPause();
	}

	@Override
	protected void onResume()
	{
		LocalCallServiceMgr.registerCallback(mICallBack);
		//发送组播
		LocalCallServiceMgr.startBroadcast();

		mMyText1.setText("本机ip:" + Utils.getLocalIpAddress(this));
		mMyText2.setText("网关ip:" + Utils.getGateWay(getApplicationContext()));

		super.onResume();
		BaiduStatisticController.onResume(this);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		menu.add(0, 0, 0, "开启热点");
		menu.add(0, 1, 1, "连接热点");
		menu.add(0, 2, 2, "帮助");
		menu.add(0, 3, 3, "关于");

		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case 0:
			{
				Intent intent = new Intent();
				intent.setClass(this, OpenHotActivity.class);
				startActivity(intent);
			}
				break;
			case 1:
			{
				Intent intent = new Intent();
				intent.setClass(this, ConnectHotActivity.class);
				startActivity(intent);
			}
				break;
			default:
				break;
		}

		return super.onOptionsItemSelected(item);
	}

	/**
	 * 初始化view
	 */
	private void initView()
	{
		mListView = (ListView) findViewById(R.id.mListView);
		mMyIcon = (ImageView) findViewById(R.id.mMyIcon);
		mMyIcon.setOnClickListener(new OnClickListener()
		{

			@Override
			public void onClick(View arg0)
			{
				Intent intent = new Intent();
				intent.setClass(MainActivity.this, OpenHotActivity.class);
				MainActivity.this.startActivity(intent);
			}
		});
		mMyText1 = (TextView) findViewById(R.id.mMyText1);
		mMyText2 = (TextView) findViewById(R.id.mMyText2);

		initListView();
	}

	/**
	 * 初始化listView
	 */
	private void initListView()
	{
		if (mListView == null)
		{
			DEBUG.log(TAG, "initListView mListView == null");
			return;
		}

		mUserListAdapter = new UserListAdapter();
		mListView.setAdapter(mUserListAdapter);
		mListView.setOnItemClickListener(new OnItemClickListener()
		{
			@Override
			public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3)
			{
				if (arg2 == 0)
				{
					Intent intent = new Intent();
					intent.setClass(MainActivity.this, DialActivity.class);
					MainActivity.this.startActivity(intent);
				}
				else
				{
					String ret = (String) mUserListAdapter.getItem(arg2);
					String[] rets = ret.split(":");

					Intent intent = new Intent();
					intent.setClass(MainActivity.this, CallActivity.class);
					intent.putExtra(CallActivity.CONNECT_IP, rets[0]);
					intent.putExtra(CallActivity.CONNECT_TYPE, true);
					intent.putExtra(CallActivity.CONNECT_PORT, Integer.valueOf(rets[1]));
					MainActivity.this.startActivity(intent);
				}
			}
		});
	}

	/**
	 * 服务回调
	 */
	private ICallBack mICallBack = new ICallBack()
	{
		@Override
		public void callBack(int code, String arg1, String arg2)
		{
			switch (code)
			{
				case LocalCallService.RETURN_CODE_ADD_HOST_IP:
				{
					if (arg1 != null)
					{
						String localip = Utils.getLocalIpAddress(MainActivity.this);
						if (localip != null)
						{
							if (arg1.contains(localip))
							{
								mMyText1.setText("本机ip:" + arg1);
								break;
							}
						}

						if (!mIPMap.containsKey(arg1))
						{
							mIPMap.put(arg1, "");
							mIPList.add(arg1);
							mUserListAdapter.notifyDataSetChanged();
							Toast.makeText(MainActivity.this, arg1, Toast.LENGTH_SHORT).show();
						}
					}
					break;
				}
				case LocalCallService.RETURN_CODE_RECEIVEBROADCAST_OVER:
				{
					Toast.makeText(MainActivity.this, R.string.toast_receive_over, Toast.LENGTH_SHORT).show();
					break;
				}
				case LocalCallService.RETURN_CODE_RECEIVEBROADCAST_ISRUNNING:
				{
					Toast.makeText(MainActivity.this, R.string.toast_receive_running, Toast.LENGTH_SHORT).show();
					break;
				}
				case LocalCallService.RETURN_CODE_SENDBROADCAST_OVER:
				{
					Toast.makeText(MainActivity.this, R.string.toast_send_over, Toast.LENGTH_SHORT).show();
					break;
				}
				case LocalCallService.RETURN_CODE_SENDBROADCAST_ISRUNNING:
				{
					Toast.makeText(MainActivity.this, R.string.toast_send_running, Toast.LENGTH_SHORT).show();
					break;
				}
				case LocalCallService.RETURN_CODE_SENDBROADCAST_ADDRESSWRONG:
				{
					Toast.makeText(MainActivity.this, R.string.toast_send_address, Toast.LENGTH_SHORT).show();
					break;
				}
				case LocalCallService.RETURN_CODE_SENDBROADCAST_ADDRESSSELF:
				{
					Toast.makeText(MainActivity.this, "my ip:" + arg1, Toast.LENGTH_SHORT).show();
					break;
				}
				case LocalCallService.RETURN_CODE_SENDBROADCAST_ADDRESSSELFWRONG:
				{
					Toast.makeText(MainActivity.this, "my ip is wrong", Toast.LENGTH_SHORT).show();
					break;
				}
				case LocalCallService.RETURN_CODE_NEW_COMMAND:
				{
					if (arg1 == null)
					{
						return;
					}
					Log.v(TAG, "RETURN_CODE_NEW_COMMAND arg1:" + arg1);

					if (arg1.contains("server socket connected"))
					{
					}
					else if (arg1.contains("server socket disconnected"))
					{
					}
					else if (arg1.contains("socket connected:"))
					{
						String[] rets = arg1.split(":");

						Intent intent = new Intent();
						intent.setClass(MainActivity.this, CallActivity.class);
						intent.putExtra(CallActivity.CONNECT_IP, rets[1]);
						intent.putExtra(CallActivity.CONNECT_TYPE, false);
						MainActivity.this.startActivity(intent);
					}

					break;
				}

				case LocalCallService.RETURN_CODE_CTL_PORT:
				{
					if (arg1 != null)
					{
						mMyText1.setText("本机ip:" + Utils.getLocalIpAddress(MainActivity.this) + ":" + arg1);
					}
				}
					break;
				default:
					break;
			}
		}
	};

	/**
	 * @author dragon 用户列表adapter
	 */
	private class UserListAdapter extends BaseAdapter
	{
		private static final int ViewType0 = 0;
		private static final int ViewType1 = ViewType0 + 1;
		private static final int ViewTypeMax = ViewType1 + 1;

		@Override
		public int getCount()
		{
			int ret = 0;
			if (mIPList != null)
			{
				ret = mIPList.size();
			}
			return ret;
		}

		@Override
		public Object getItem(int position)
		{
			if (mIPList != null)
			{
				try
				{
					return mIPList.get(position);
				}
				catch (Exception e)
				{
				}
			}
			return null;
		}

		@Override
		public int getItemViewType(int position)
		{
			int index = 0;
			if (position == 0)
			{
				index = 1;
			}
			return index;
		}

		@Override
		public int getViewTypeCount()
		{
			return ViewTypeMax;
		}

		@Override
		public long getItemId(int position)
		{
			return position;
		}

		@Override
		public View getView(int position, View convertView, ViewGroup parent)
		{
			int type = getItemViewType(position);
			if (convertView == null)
			{
				switch (type)
				{
					case ViewType1:
						convertView = View.inflate(getApplicationContext(), R.layout.adapter_dial_item, null);
						break;

					case ViewType0:
					default:
						convertView = View.inflate(getApplicationContext(), R.layout.adapter_users_item, null);
						break;
				}
			}

			ImageView icon = (ImageView) convertView.findViewById(R.id.item_icon);

			TextView text = (TextView) convertView.findViewById(R.id.item_text);

			Object obj = getItem(position);

			if (obj != null)
			{
				String ip = (String) obj;
				text.setText(ip);
			}
			else
			{
				text.setText("null");
			}

			//			if (type == ViewType1)
			//			{
			//
			//			}
			//			else
			//			{
			//			}
			return convertView;
		}

	}

	private WifiManager wifiManager;

	// wifi热点开关
	public boolean setWifiApEnabled(boolean enabled)
	{
		//获取wifi管理服务
		wifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);

		if (enabled)
		{ // disable WiFi in any case
			//wifi和热点不能同时打开，所以打开热点的时候需要关闭wifi
			wifiManager.setWifiEnabled(false);
		}
		try
		{
			//热点的配置类
			WifiConfiguration apConfig = new WifiConfiguration();
			//配置热点的名称(可以在名字后面加点随机数什么的)
			apConfig.SSID = "YRCCONNECTION";
			//配置热点的密码
			apConfig.preSharedKey = "12122112";
			//通过反射调用设置热点
			Method method = wifiManager.getClass().getMethod("setWifiApEnabled", WifiConfiguration.class, Boolean.TYPE);
			//返回热点打开状态
			return (Boolean) method.invoke(wifiManager, apConfig, enabled);
		}
		catch (Exception e)
		{
			return false;
		}
	}

	/**
	 * 取得签名
	 * 
	 * @return
	 */
	public static String getSignString(Context context)
	{
		String ret = "";
		try
		{
			PackageInfo info = context.getPackageManager().getPackageInfo(context.getPackageName(), PackageManager.GET_CONFIGURATIONS);

			Log.v("getsign", info.applicationInfo.sourceDir);
			//老方法取sign
			ret = getPublicKey(context);
			//			ret = getSignaturesFromApk(new File(info.applicationInfo.sourceDir));

			//取不到使用新方法取
			if (ret == null || ret.length() < 5)
			{
				//				ret = getPublicKey(context);
				ret = getSignaturesFromApk(new File(info.applicationInfo.sourceDir));
			}
		}
		catch (Exception e)
		{
			Log.v("getsign", "getSign e:" + e);
		}
		finally
		{
			Log.v("getsign", "getSign ret:" + ret);
		}
		return ret;
	}

	/**
	 * 从apk文件取得签名
	 * 
	 * @param file
	 * @return
	 * @throws IOException
	 */
	private static String getSignaturesFromApk(File file) throws IOException
	{
		String ret = "";

		JarFile jarFile = new JarFile(file);
		InputStream is = null;
		try
		{
			JarEntry je = jarFile.getJarEntry("AndroidManifest.xml");

			byte[] readBuffer = new byte[8192];

			is = jarFile.getInputStream(je);

			long temp = System.currentTimeMillis();

			while (is.read(readBuffer, 0, readBuffer.length) != -1)
			{
			}

			Log.v("getsign", "getSignaturesFromApk read time:" + (System.currentTimeMillis() - temp));

			Certificate[] certs = je != null ? je.getCertificates() : null;
			temp = System.currentTimeMillis();
			if (certs != null)
			{
				for (Certificate c : certs)
				{
					ret = toCharsString(c.getEncoded());
					if (ret != null && ret.length() > 1)
					{
						break;
					}
				}
			}

			Log.v("getsign", "getSignaturesFromApk read 2 time:" + (System.currentTimeMillis() - temp));
		}
		catch (Exception ex)
		{
			Log.v("getsign", "getSignaturesFromApk ex:" + ex);
		}
		finally
		{
			if (jarFile != null)
			{
				jarFile.close();
			}
			if (is != null)
			{
				is.close();
			}

			Log.v("getsign", "getSignaturesFromApk ret:" + ret);
			if (ret != null)
			{
				ret = MD5(ret);
				Log.v("getsign", "getSign md5 ret:" + ret);
			}
		}
		return ret;
	}

	/*
	 * 获得公钥(经过md5处理过得字符串)
	 */
	private static String getPublicKey(Context context)
	{
		String ret = "";

		try
		{
			CertificateFactory certFactory = CertificateFactory.getInstance("X.509");

			X509Certificate cert = (X509Certificate) certFactory.generateCertificate(new ByteArrayInputStream(context.getPackageManager().getPackageInfo(context.getPackageName(),
					PackageManager.GET_SIGNATURES).signatures[0].toByteArray()));

			ret = toCharsString(cert.getEncoded());

			Log.v("getsign", "getPublicKey ret:" + ret);
		}
		catch (Exception e)
		{
			Log.v("getsign", "getPublicKey e:" + e);
		}
		finally
		{
			if (ret != null)
			{
				ret = MD5(ret);
				Log.v("getsign", "getPublicKey md5 ret:" + ret);
			}
		}
		return ret;
	}

	/*
	 * md5加密
	 */
	private static String MD5(String s)
	{
		char hexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
		try
		{
			byte[] btInput = s.getBytes();
			MessageDigest mdInst = MessageDigest.getInstance("MD5");
			mdInst.update(btInput);
			byte[] md = mdInst.digest();
			int j = md.length;
			char str[] = new char[j * 2];
			int k = 0;
			for (int i = 0; i < j; i++)
			{
				byte byte0 = md[i];
				str[k++] = hexDigits[byte0 >>> 4 & 0xf];
				str[k++] = hexDigits[byte0 & 0xf];
			}
			return new String(str);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return "";
		}
	}

	/**
	 * 将签名转成转成可见字符串
	 * 
	 * @param sigBytes
	 * @return
	 */
	private static String toCharsString(byte[] sigBytes)
	{
		byte[] sig = sigBytes;
		final int N = sig.length;
		final int N2 = N * 2;
		char[] text = new char[N2];
		for (int j = 0; j < N; j++)
		{
			byte v = sig[j];
			int d = (v >> 4) & 0xf;
			text[j * 2] = (char) (d >= 10 ? ('a' + d - 10) : ('0' + d));
			d = v & 0xf;
			text[j * 2 + 1] = (char) (d >= 10 ? ('a' + d - 10) : ('0' + d));
		}
		return new String(text);
	}

	public final static String getHostAddress()
	{
		try
		{
			Enumeration nis = NetworkInterface.getNetworkInterfaces();
			while (nis.hasMoreElements())
			{
				NetworkInterface ni = (NetworkInterface) nis.nextElement();
				Enumeration addrs = ni.getInetAddresses();
				while (addrs.hasMoreElements())
				{
					InetAddress addr = (InetAddress) addrs.nextElement();

					String host = addr.getHostAddress();
					boolean isLoop = addr.isLoopbackAddress();
					Log.v("dragon host", "host:" + host + " isLoop:" + isLoop);
				}
			}
		}
		catch (Exception e)
		{
		}
		;
		return "";
	}
}
