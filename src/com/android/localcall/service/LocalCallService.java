package com.android.localcall.service;

import java.util.regex.Pattern;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.os.IBinder;
import android.os.RemoteException;

import com.android.localcall.jni.Rtp;
import com.android.localcall.jni.Rtp.IDataCallback;
import com.android.localcall.socket.CtlServerSocket;
import com.android.localcall.socket.CtlServerSocket.CtlServerCallBack;
import com.android.localcall.socket.CtlSocket;
import com.android.localcall.socket.CtlSocket.CtlSocketCallBack;
import com.android.localcall.utils.AudioSync;
import com.android.localcall.utils.DEBUG;
import com.android.localcall.utils.MultiCastReceiver;
import com.android.localcall.utils.MultiCastSender;

public class LocalCallService extends Service
{
	private final String TAG = "LocalCallService";
	private Pattern mPattern = Pattern.compile("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
	/**
	 * rtp本地监听端口
	 */
	private final int RTP_LOCAL_PORT = 40018;
	//返回host ip
	public static final int RETURN_CODE_ADD_HOST_IP = 1001;
	public static final int RETURN_CODE_RECEIVEBROADCAST_OVER = RETURN_CODE_ADD_HOST_IP + 1;
	public static final int RETURN_CODE_RECEIVEBROADCAST_ISRUNNING = RETURN_CODE_RECEIVEBROADCAST_OVER + 1;
	public static final int RETURN_CODE_SENDBROADCAST_OVER = RETURN_CODE_RECEIVEBROADCAST_ISRUNNING + 1;
	public static final int RETURN_CODE_SENDBROADCAST_ISRUNNING = RETURN_CODE_SENDBROADCAST_OVER + 1;
	public static final int RETURN_CODE_SENDBROADCAST_ADDRESSWRONG = RETURN_CODE_SENDBROADCAST_ISRUNNING + 1;
	public static final int RETURN_CODE_SENDBROADCAST_ADDRESSSELF = RETURN_CODE_SENDBROADCAST_ADDRESSWRONG + 1;
	public static final int RETURN_CODE_SENDBROADCAST_ADDRESSSELFWRONG = RETURN_CODE_SENDBROADCAST_ADDRESSSELF + 1;
	public static final int RETURN_CODE_NEW_COMMAND = RETURN_CODE_SENDBROADCAST_ADDRESSSELFWRONG + 1;
	public static final int RETURN_CODE_CTL_PORT = RETURN_CODE_NEW_COMMAND + 1;
	//组播主机的地址
	private static final String MULTICASTHOST = "224.0.0.102";
//	private static final String MULTICASTHOST = "239.255.255.250";
	//组播使用的端口
	private static final int MULTICASTPORT = 40005;
	//远程回调保存
	private ILocalCallServiceCallback mILocalCallServiceCallback = null;
	//控制客户端回调
	private ILocalCallServiceCallback mILocalCallServiceCallbackForClient = null;
	//控制服务端回调
	private ILocalCallServiceCallback mILocalCallServiceCallbackForServer = null;
	//传输数据
	private Rtp mRtp = null;
	//接收组播
	private MultiCastReceiver mMultiCastReceiver = null;
	//发送组播
	private MultiCastSender mMultiCastSender = null;
	//组播申请
	private MulticastLock multicastLock;
	//同步音频
	private AudioSync mAudioSync = new AudioSync();
	//用于停止录音
	private boolean stopRecorder = false;
	//控制的服务端
	private CtlServerSocket mCtlServerSocket = null;
	//控制的client端
	private CtlSocket mCtlSocket = null;
	//控制服务端端口
	private int mCtlPort = 0;
	//控制的client端接到命令
	private CtlSocketCallBack mCtlSocketCallBack = new CtlSocketCallBack()
	{
		@Override
		public void newCommand(String command)
		{
			if (mILocalCallServiceCallback != null)
			{
				try
				{
					mILocalCallServiceCallback.callBack(RETURN_CODE_NEW_COMMAND, command, "");
				}
				catch (RemoteException e)
				{
					DEBUG.log(TAG, "mICallBack e:" + e);
				}
			}
		}
	};
	//控制的服务端接到命令或新的连接
	private CtlServerCallBack mCtlServerCallBack = new CtlServerCallBack()
	{

		@Override
		public void newCommand(String command)
		{
			if (mILocalCallServiceCallback != null)
			{
				try
				{
					mILocalCallServiceCallback.callBack(RETURN_CODE_NEW_COMMAND, command, "");
				}
				catch (RemoteException e)
				{
					DEBUG.log(TAG, "mICallBack e:" + e);
				}
			}
		}
	};
	/**
	 * 封装远程回调
	 */
	private ICallBack mICallBack = new ICallBack()
	{
		@Override
		public void callBack(int code, String arg1, String arg2)
		{
			if (mILocalCallServiceCallback != null)
			{
				try
				{
					mILocalCallServiceCallback.callBack(code, arg1, arg2);
				}
				catch (RemoteException e)
				{
					DEBUG.log(TAG, "mICallBack e:" + e);
				}
			}
		}
	};

	/**
	 * 远程调用代理
	 */
	private ILocalCallService.Stub stub = new ILocalCallService.Stub()
	{
		@Override
		public void registerCallback(ILocalCallServiceCallback callback) throws RemoteException
		{
			mILocalCallServiceCallback = callback;
			DEBUG.log(TAG, "registerCallback mILocalCallServiceCallback：" + mILocalCallServiceCallback);
		}

		@Override
		public boolean startBroadcast() throws RemoteException
		{
			if (mMultiCastSender != null)
			{
				mMultiCastSender.waitSend(false);
			}
			return true;
		}

		@Override
		public boolean stopBroadcast() throws RemoteException
		{
			if (mMultiCastSender != null)
			{
				mMultiCastSender.waitSend(true);
			}
			return true;
		}

		@Override
		public boolean startRecorder(String ip) throws RemoteException
		{
			if (mPattern.matcher(ip).find())
			{
				DEBUG.log(TAG, "startRecorder ip : " + ip);
				mRtp.addRtpDestinationIp(ip);
			}
			else
			{
				DEBUG.log(TAG, "startRecorder error");
				return false;
			}
			stopRecorder = false;
			mAudioSync.startRecorder();
			new Thread(new Runnable()
			{

				@Override
				public void run()
				{
					while (!stopRecorder)
					{
						short[] data = mAudioSync.readDataFromRecorder();
						if (mRtp != null && data != null)
						{
							DEBUG.log(TAG, "write data to rtp data.lenth : " + data.length);
							DEBUG.log(TAG, "write data to rtp data[0] : " + data[0]);
							DEBUG.log(TAG, "write data to rtp data[1] : " + data[1]);
							mRtp.write(data);
						}
					}
				}
			}).start();

			mAudioSync.stopPlayer();
			return true;
		}

		@Override
		public boolean stopRecorder() throws RemoteException
		{
			stopRecorder = true;
			mAudioSync.stopRecorder();

			mAudioSync.startPlayer();
			return true;
		}

		@Override
		public boolean startPlayer() throws RemoteException
		{
			mAudioSync.startPlayer();

			return false;
		}

		@Override
		public boolean stopPlayer() throws RemoteException
		{
			mAudioSync.stopPlayer();

			return false;
		}

		@Override
		public boolean connectServer(String ip, int port) throws RemoteException
		{
			if(mCtlServerSocket != null)
			{
				mCtlServerSocket.close();
				mCtlServerSocket = null;
			}
			
			if (mCtlSocket != null)
			{
				mCtlSocket.closeConnect();
			}
			
			mCtlSocket = new CtlSocket();
			mCtlSocket.connectServer(ip, port, mCtlSocketCallBack);

			return false;
		}

		@Override
		public boolean writeCommandFromClient(String command) throws RemoteException
		{
			if (mCtlSocket != null)
			{
				mCtlSocket.writeCommand(command);
			}
			return false;
		}

		@Override
		public boolean writeCommandFromServer(String command) throws RemoteException
		{
			if (mCtlServerSocket != null)
			{
				mCtlServerSocket.writeCommand(command);
			}
			return false;
		}
		@Override
		public boolean disconnectServer() throws RemoteException
		{
			if(mCtlServerSocket != null)
			{
				mCtlServerSocket.close();
				mCtlServerSocket = null;
			}
			
			mCtlServerSocket = new CtlServerSocket();
			mCtlPort = mCtlServerSocket.open(mCtlServerCallBack);
			
			if (mCtlSocket != null)
			{
				mCtlSocket.closeConnect();
			}
			return false;
		}

		@Override
		public boolean restartCtlServer() throws RemoteException {
			// TODO Auto-generated method stub
			return false;
		}
	};

	@Override
	public IBinder onBind(Intent arg0)
	{
		DEBUG.log(TAG, "onBind");
		return stub;
	}

	@Override
	public void onCreate()
	{
		DEBUG.log(TAG, "onCreate");
		//组播申请
		WifiManager wifiManager = (WifiManager) getSystemService(Context.WIFI_SERVICE);
		multicastLock = wifiManager.createMulticastLock("multicast.test");
		multicastLock.acquire();
		//打开控制服务
		mCtlServerSocket = new CtlServerSocket();
		mCtlPort = mCtlServerSocket.open(mCtlServerCallBack);
		
		if(mICallBack != null)
		{
			mICallBack.callBack(RETURN_CODE_CTL_PORT, mCtlPort+"", "");
		}
		//接收广播
		mMultiCastReceiver = new MultiCastReceiver(this, MULTICASTHOST, MULTICASTHOST, MULTICASTPORT, new ICallBack()
		{
			@Override
			public void callBack(int code, String arg1, String arg2)
			{
				mICallBack.callBack(code, arg1, arg2);
			}
		});
		mMultiCastReceiver.startReceive();

		//发送广播
		mMultiCastSender = new MultiCastSender(this, MULTICASTHOST, MULTICASTPORT, mCtlPort, new ICallBack()
		{
			@Override
			public void callBack(int code, String arg1, String arg2)
			{
				mICallBack.callBack(code, arg1, arg2);
			}
		});
		mMultiCastSender.startSend();

		//开启rtp传输
		mRtp = new Rtp();
		mRtp.openRtp(RTP_LOCAL_PORT);
		mRtp.setCallback(new IDataCallback()
		{
			@Override
			public void dataCallback(short[] data, int size)
			{
				DEBUG.log(TAG, "read data from service callback size:" + size);
				DEBUG.log(TAG, "read data from service callback data[0]:" + data[0]);
				DEBUG.log(TAG, "read data from service callback data[1]:" + data[1]);
				mAudioSync.writeDataToPlayer(data);
			}
		});

		mAudioSync.start();
		mAudioSync.setRtp(mRtp);
		
		

		super.onCreate();
	}

	@Override
	public void onDestroy()
	{
		DEBUG.log(TAG, "onDestroy");
		if (mMultiCastReceiver != null)
		{
			mMultiCastReceiver.stopReceive();
			mMultiCastReceiver = null;
		}
		if (mMultiCastSender != null)
		{
			mMultiCastSender.stopSend();
			mMultiCastSender = null;
		}
		//组播释放
		multicastLock.release();
		multicastLock = null;

		//关闭rtp传输
		mRtp.closeRtp();
		mRtp.native_rease();
		mRtp = null;

		mAudioSync.interrupt();

		if(mCtlServerSocket != null)
		{
			mCtlServerSocket.close();
			mCtlServerSocket = null;
		}
		super.onDestroy();
	}

	@Override
	public void onRebind(Intent intent)
	{
		DEBUG.log(TAG, "onRebind  ");
		super.onRebind(intent);
	}

	@Override
	public void onStart(Intent intent, int startId)
	{
		DEBUG.log(TAG, "onStart startId " + startId);
		super.onStart(intent, startId);
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId)
	{
		DEBUG.log(TAG, "onStartCommand startId " + startId);
		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public boolean onUnbind(Intent intent)
	{
		DEBUG.log(TAG, "onUnbind");
		return super.onUnbind(intent);
	}

	public static int[] rawByteArray2RGBABitmap1(byte[] data, int width, int height)
	{
		int frameSize = width * height;
		int[] rgba = new int[frameSize];
		int y = 0;
		int u = 0;
		int v = 0;
		int r = 0;
		int g = 0;
		int b = 0;
		int temp_index = 0;
		int temp_main_index = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				temp_index = frameSize + (i >> 1) * width + (j & ~1);
				temp_main_index = i * width + j;

				y = (0xff & ((int) data[temp_main_index]));
				u = (0xff & ((int) data[temp_index]));
				v = (0xff & ((int) data[temp_index + 1]));

				y = y < 16 ? 16 : y;
				y = y - 16;
				r = y + v - 128;
				g = y - (((813 * v) - 54016 - (391 * u)) >> 10);

				b = y + u - 128;

				r = r < 0 ? 0 : (r > 255 ? 255 : r);
				g = g < 0 ? 0 : (g > 255 ? 255 : g);
				b = b < 0 ? 0 : (b > 255 ? 255 : b);

				rgba[temp_main_index] = 0xff000000 + (b << 16) + (g << 8) + r;
			}
		}

		DEBUG.log("onPreviewFrame", "onPreviewFrame rgba.length：" + rgba.length);
		return rgba;
	}

	public static Bitmap rawByteArray2RGBABitmap2(byte[] data, int width, int height)
	{
		int frameSize = width * height;
		int[] rgba = new int[frameSize];
		int y = 0;
		int u = 0;
		int v = 0;
		int r = 0;
		int g = 0;
		int b = 0;
		int temp_index = 0;
		int temp_main_index = 0;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				temp_index = frameSize + (i >> 1) * width + (j & ~1);
				temp_main_index = i * width + j;

				y = (0xff & ((int) data[temp_main_index]));
				u = (0xff & ((int) data[temp_index]));
				v = (0xff & ((int) data[temp_index + 1]));

				y = y < 16 ? 16 : y;

				y = y - 16;
				//				v = v - 128;
				//				u = u - 128;

				r = y + v - 128;
				//				g = y - ((813 * (v - 128) - 391 * (u - 128))>>10);
				//				g = y - (((vtable[v]) - 54016 - (utable[u])) >> 10);
				g = y - (((813 * v) - 54016 - (391 * u)) >> 10);

				b = y + u - 128;

				r = r < 0 ? 0 : (r > 255 ? 255 : r);
				g = g < 0 ? 0 : (g > 255 ? 255 : g);
				b = b < 0 ? 0 : (b > 255 ? 255 : b);

				rgba[temp_main_index] = 0xff000000 + (b << 16) + (g << 8) + r;
			}
		}

		DEBUG.log("onPreviewFrame", "onPreviewFrame rgba.length：" + rgba.length);

		Bitmap bmp = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
		bmp.setPixels(rgba, 0, width, 0, 0, width, height);
		DEBUG.log("onPreviewFrame", "onPreviewFrame  bmp.getRowBytes()：" + bmp.getRowBytes());
		return bmp;
	}

	/**
	 * @author dragon 用于对远程回调封装
	 */
	public interface ICallBack
	{
		public void callBack(int code, String arg1, String arg2);
	}
}
