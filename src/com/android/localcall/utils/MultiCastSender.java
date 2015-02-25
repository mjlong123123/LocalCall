package com.android.localcall.utils;

import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.MulticastSocket;

import android.content.Context;
import android.os.Process;

import com.android.localcall.service.LocalCallService.ICallBack;

public class MultiCastSender extends Thread
{
	private final String TAG = "MultiCastSender";
	//发送组播地址
	private String mHost = "";
	//发送组播端口
	private int mPort = 0;
	//控制端口
	private int mCtlPort = 0;
	//组播套接字
	private MulticastSocket mMulticastSocket = null;
	//本机ip地址保存
	private String mSelfIp = "";
	//重复发送时间间隔
	private static final int RETRY_TIME = 1000;
	//回调使用
	private ICallBack mICallBack = null;
	//是否停止执行
	private boolean mStop = true;
	//挂起lock
	private Object mLock = new Object();
	//等待flag
	private boolean mWait = false;

	private Context mContext = null;

	public MultiCastSender(Context mContext, String host, int port, int ctl, ICallBack cal)
	{
		mHost = host;
		mPort = port;
		mICallBack = cal;
		mCtlPort = ctl;
		this.mContext = mContext;
	}

	/**
	 * 启动发送线程
	 * 
	 * @return
	 */
	public boolean startSend()
	{
		DEBUG.log(TAG, "startSend");
		if (!mStop)
		{
			DEBUG.log(TAG, "startSend is running");
			return false;
		}
		start();
		return true;
	}

	/**
	 * 停止发送线程
	 * 
	 * @return
	 */
	public boolean stopSend()
	{
		DEBUG.log(TAG, "stopSend");

		mStop = true;
		synchronized (mLock)
		{
			synchronized (mLock)
			{
				mLock.notifyAll();
			}
		}
		return true;
	}

	/**
	 * 挂起
	 * 
	 * @param flag
	 */
	public void waitSend(boolean flag)
	{
		mWait = flag;

		if (!flag)
		{
			synchronized (mLock)
			{
				mLock.notifyAll();
			}
		}
	}

	@Override
	public void run()
	{
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		mStop = false;

		try
		{
			mMulticastSocket = new MulticastSocket();

			mMulticastSocket.setTimeToLive(10);

			InetAddress address = InetAddress.getByName(mHost);

			int count = 0;
			while (!mStop)
			{
				//取自己ip
				if (mSelfIp == null || mSelfIp.equals(""))
				{
					mSelfIp = Utils.getLocalIpAddress(mContext)+":"+mCtlPort;
					DEBUG.log(TAG, "sentBroadcast send mSelfIp :" + mSelfIp);
				}
				//发送自己ip
				if (mSelfIp != null && !mSelfIp.equals(""))
				{
					DEBUG.log(TAG, "sentBroadcast send count :" + count);
					DEBUG.log(TAG, "sentBroadcast send mSelfIp :" + mSelfIp);
					count++;
					byte[] data = mSelfIp.getBytes();

					DatagramPacket dataPacket = new DatagramPacket(data, data.length, address, mPort);

					mMulticastSocket.send(dataPacket);
				}
				//休眠
				Thread.sleep(RETRY_TIME);

				if (mWait)
				{
					synchronized (mLock)
					{
						DEBUG.log(TAG, "sentBroadcast send wait start");
						mLock.wait();
						DEBUG.log(TAG, "sentBroadcast send wait end :");
					}
				}
			}

		}
		catch (Exception e)
		{
			DEBUG.log(TAG, "sentBroadcast e : " + e);
		}
		finally
		{
			DEBUG.log(TAG, "sentBroadcast finally");
			if (mMulticastSocket != null && !mMulticastSocket.isClosed())
			{
				DEBUG.log(TAG, "sentBroadcast finally multicastSocket.close()");
				mMulticastSocket.close();
				mMulticastSocket = null;
			}
		}
	}

	/**
	 * 回调封装
	 * 
	 * @param code
	 * @param arg1
	 * @param arg2
	 */
	private void callback(int code, String arg1, String arg2)
	{
		if (mICallBack != null)
		{
			mICallBack.callBack(code, arg1, arg2);
		}
	}
}
