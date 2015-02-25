package com.android.localcall.utils;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.MulticastSocket;
import java.net.NetworkInterface;

import android.content.Context;
import android.os.Process;

import com.android.localcall.service.LocalCallService;
import com.android.localcall.service.LocalCallService.ICallBack;

public class MultiCastReceiver extends Thread
{
	private final String TAG = "MultiCastReceiver";
	//组播ip
	private String mHost = "";
	private String mBind = "";
	//组播端口
	private int mPort = 0;
	//本机ip地址保存
	private String mSelfIp = "";
	//回调使用
	private ICallBack mICallBack = null;
	//接收组播的套接字
	private MulticastSocket mReceiveMulticastSocket = null;
	
	private NetworkInterface mNetworkInterface = null;
	
	private InetSocketAddress mInetSocketAddressGroup = null;
	//停止
	private boolean mStop = true;

	private InetAddress address = null;

	private Context mContext = null;

	public MultiCastReceiver(Context mContext, String host, String bind, int port, ICallBack cal)
	{
		mHost = host;
		mPort = port;
		mBind = bind;
		mICallBack = cal;
		this.mContext = mContext;
	}

	/**
	 * 开始接收组播
	 * 
	 * @return
	 */
	public boolean startReceive()
	{
		DEBUG.log(TAG, "startReceive");
		if (!mStop)
		{
			DEBUG.log(TAG, "startReceive is running");
			return false;
		}
		start();
		return true;
	}

	/**
	 * 结束接收组播
	 * 
	 * @return
	 */
	public boolean stopReceive()
	{
		DEBUG.log(TAG, "stopReceive");
		mStop = true;

		if (mReceiveMulticastSocket != null && !mReceiveMulticastSocket.isClosed())
		{
			DEBUG.log(TAG, "finally mReceiveMulticastSocket.close()");
			try
			{
				mReceiveMulticastSocket.leaveGroup(mInetSocketAddressGroup, mNetworkInterface);
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
			mReceiveMulticastSocket.close();
			mReceiveMulticastSocket = null;
		}
		return true;
	}

	@Override
	public void run()
	{
		Process.setThreadPriority(Process.THREAD_PRIORITY_BACKGROUND);

		mStop = false;

		try
		{
			mReceiveMulticastSocket = new MulticastSocket(null);
			mReceiveMulticastSocket.setReuseAddress(true);
			InetSocketAddress adbind = new InetSocketAddress(mPort);
			mReceiveMulticastSocket.bind(adbind);
			
			mInetSocketAddressGroup = new InetSocketAddress(InetAddress.getByName(mHost), mPort);
			
			mNetworkInterface = NetworkInterface.getByInetAddress(InetAddress.getByName(mBind));

			mReceiveMulticastSocket.setLoopbackMode(false);
			mReceiveMulticastSocket.joinGroup(mInetSocketAddressGroup, mNetworkInterface);
			
//			mReceiveMulticastSocket = new MulticastSocket(mPort);
//			address = InetAddress.getByName(mHost);
//			mReceiveMulticastSocket.setLoopbackMode(false);
//			mReceiveMulticastSocket.joinGroup(address);

			byte buf[] = new byte[50];
			DatagramPacket dp = new DatagramPacket(buf, 50);

			while (!mStop)
			{
				mReceiveMulticastSocket.receive(dp);

				String temp = new String(buf, 0, dp.getLength());

				DEBUG.log(TAG, "receive temp:" + temp);

				if (mSelfIp == null || mSelfIp.equals(""))
				{
					mSelfIp = Utils.getLocalIpAddress(mContext);
				}

				if (temp.equals(mSelfIp))
				{
					DEBUG.log(TAG, "receive continue");
					continue;
				}
				callback(LocalCallService.RETURN_CODE_ADD_HOST_IP, temp, "");
			}
		}
		catch (Exception e)
		{
			DEBUG.log(TAG, "receive e:" + e);
		}
		finally
		{
			DEBUG.log(TAG, "finally");
			if (mReceiveMulticastSocket != null && !mReceiveMulticastSocket.isClosed())
			{
				DEBUG.log(TAG, "finally mReceiveMulticastSocket.close()");

				try
				{
					mReceiveMulticastSocket.leaveGroup(mInetSocketAddressGroup, mNetworkInterface);
				}
				catch (IOException e)
				{
					e.printStackTrace();
				}
				mReceiveMulticastSocket.close();
				mReceiveMulticastSocket = null;
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
