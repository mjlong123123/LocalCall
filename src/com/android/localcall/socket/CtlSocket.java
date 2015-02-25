package com.android.localcall.socket;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;

import android.os.Process;

import com.android.localcall.utils.DEBUG;

public class CtlSocket extends Thread
{
	private static final String TAG = "CtlSocket";

	private Socket mSocket = new Socket();

	private String mHost = "";
	private int mPort = 0;
	private BufferedWriter mOutputStream = null;
	private BufferedReader mInputStream = null;

	private CtlSocketCallBack mCtlSocketCallBack = null;

	public CtlSocket()
	{
		super("CtlSocket");
	}

	public boolean connectServer(String host, int port, CtlSocketCallBack callback)
	{
		mCtlSocketCallBack = callback;
		mHost = host;
		mPort = port;
		start();

		return false;
	}

	public void closeConnect()
	{
		closeStream();
		closeSocket();
	}

	@Override
	public void run()
	{
		process();
	}

	private void process()
	{

		Process.setThreadPriority(NORM_PRIORITY);
		try
		{
			DEBUG.log(TAG, "connect start host:" + mHost + " port:" + mPort);
			mSocket.connect(new InetSocketAddress(mHost, mPort));

			DEBUG.log(TAG, "connect end host:" + mHost + " port:" + mPort);

			try
			{
				mOutputStream = new BufferedWriter(new OutputStreamWriter(mSocket.getOutputStream()));
				mInputStream = new BufferedReader(new InputStreamReader(mSocket.getInputStream()));

				if (mCtlSocketCallBack != null)
				{
					mCtlSocketCallBack.newCommand("socket connected");
				}

				while (true)
				{
					DEBUG.log(TAG, "read command start");

					String ret = mInputStream.readLine();

					DEBUG.log(TAG, "read command end ret:" + ret);

					if (ret == null)
					{
						break;
					}
					if (mCtlSocketCallBack != null)
					{
						mCtlSocketCallBack.newCommand(ret);
					}
				}
			}
			catch (Exception e)
			{
				DEBUG.log(TAG, " e" + e);
			}
			finally
			{
				closeStream();

				if (mCtlSocketCallBack != null)
				{
					mCtlSocketCallBack.newCommand("socket disconnected");
				}
			}
		}
		catch (SocketTimeoutException ee)
		{
			DEBUG.log(TAG, " e11" + ee);
			process();
		}
		catch (IOException e)
		{
			DEBUG.log(TAG, " e1" + e);
		}
		finally
		{
			closeSocket();
		}
	}

	/**
	 * 关闭连接
	 */
	private void closeSocket()
	{
		DEBUG.log(TAG, "closeSocket");
		
		closeStream();

		if (mSocket != null)
		{
			try
			{
				mSocket.close();
				mSocket = null;
			}
			catch (IOException e)
			{
				DEBUG.log(TAG, "closeSocket e1:" + e);
			}
		}
	}

	/**
	 * 关闭数据流
	 */
	private void closeStream()
	{
		DEBUG.log(TAG, "closeStream");
		
		if (mOutputStream != null)
		{
			try
			{
				mOutputStream.close();
			}
			catch (IOException e)
			{
				DEBUG.log(TAG, "closeStream e1:" + e);
			}
			finally
			{
				mOutputStream = null;
			}
		}

		if (mInputStream != null)
		{
			try
			{
				mInputStream.close();
			}
			catch (IOException e)
			{
				DEBUG.log(TAG, "closeStream e:" + e);
			}
			finally
			{
				mInputStream = null;
			}
		}
	}

	/**
	 * 写命令
	 * 
	 * @param command
	 */
	public void writeCommand(String command)
	{
		if (mOutputStream != null && command != null)
		{
			try
			{
				DEBUG.log(TAG, "writeCommand:" + command);

				mOutputStream.write(command + "\r\n");
				mOutputStream.flush();
			}
			catch (IOException e)
			{
				DEBUG.log(TAG, "writeCommand e:" + e);
			}
		}
	}

	/**
	 * @author dragon 状态回调
	 */
	public static interface CtlSocketCallBack
	{
		void newCommand(String command);
	}
}
