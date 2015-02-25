package com.android.localcall.service;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;

import com.android.localcall.service.LocalCallService.ICallBack;
import com.android.localcall.utils.DEBUG;

public class LocalCallServiceMgr
{
	private static final String TAG = "LocalCallServiceMgr";

	private static ILocalCallService mILocalCallService = null;

	private static ICallBack mICallBack = null;

	/**
	 * 服务回调函数
	 */
	private static ILocalCallServiceCallback.Stub mCallback = new ILocalCallServiceCallback.Stub()
	{
		@Override
		public void callBack(final int returnCode, final String arg1, final String arg2) throws RemoteException
		{
			Log.v("SocketService", " callBack returnCode :" + returnCode);
			Log.v("SocketService", " callBack arg1 :" + arg1);
			Log.v("SocketService", " callBack arg2 :" + arg2);
			if (mICallBack != null)
			{
				new Handler(Looper.getMainLooper()).post(new Runnable()
				{
					@Override
					public void run()
					{
						if (mICallBack != null)
							mICallBack.callBack(returnCode, arg1, arg2);
					}
				});
			}
		}
	};
	/**
	 * 服务联接
	 */
	private static ServiceConnection mServiceConnection = new ServiceConnection()
	{

		@Override
		public void onServiceDisconnected(ComponentName name)
		{
			Log.v("SocketService", "bind onServiceDisconnected name :" + name.toString());
			try
			{
				if (mILocalCallService != null)
					mILocalCallService.registerCallback(null);
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
			mILocalCallService = null;
		}

		@Override
		public void onServiceConnected(ComponentName name, IBinder service)
		{
			Log.v("SocketService", "bind onServiceConnected name :" + name.toString());
			mILocalCallService = ILocalCallService.Stub.asInterface(service);
			if (mILocalCallService != null)
			{
				try
				{
					mILocalCallService.registerCallback(mCallback);
				}
				catch (RemoteException e)
				{
					e.printStackTrace();
				}
			}
		}
	};

	/**
	 * 绑定服务
	 * 
	 * @param context
	 */
	public static void bindILocalCallService(Context context)
	{
		if (context != null)
		{
			Intent intent = new Intent();
			intent.setClass(context, LocalCallService.class);
			context.bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
		}
	}

	/**
	 * 解绑服务
	 * 
	 * @param context
	 */
	public static void unbindILocalCallService(Context context)
	{
		if (context != null && mServiceConnection != null)
		{
			context.unbindService(mServiceConnection);
		}
	}

	/**
	 * 注册回调
	 * 
	 * @param callback
	 */
	public static void registerCallback(ICallBack callback)
	{
		mICallBack = callback;
	}

	/**
	 * 发送组播
	 */
	public static boolean startBroadcast()
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.startBroadcast();
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	/**
	 * 接收组播
	 */
	public static boolean stopBroadcast()
	{
		DEBUG.log(TAG, "stopBroadcast");

		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.stopBroadcast();
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	/**
	 * 开启语音
	 */
	public static boolean startRecorder(String ip)
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.startRecorder(ip);
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	/**
	 * 关闭语音
	 */
	public static boolean stopRecorder()
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.stopRecorder();
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	/**
	 * 开启播放
	 */
	public static boolean startPlayer()
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.startPlayer();
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	/**
	 * 关闭播放
	 */
	public static boolean stopPlayer()
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.stopPlayer();
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	//连接控制服务
	public static boolean connectServer(String ip, int port)
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.connectServer(ip, port);
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	//client端发送命令
	public static boolean writeCommandFromClient(String command)
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.writeCommandFromClient(command);
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	//服务端发送命令
	public static boolean writeCommandFromServer(String command)
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.writeCommandFromServer(command);
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}
	//断开连接控制服务
	public static boolean disconnectServer()
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.disconnectServer();
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}

	//重启控制服务
	public static boolean restartCtlServer()
	{
		if (mILocalCallService != null)
		{
			try
			{
				return mILocalCallService.restartCtlServer();
			}
			catch (RemoteException e)
			{
				e.printStackTrace();
			}
		}
		return false;
	}
}
