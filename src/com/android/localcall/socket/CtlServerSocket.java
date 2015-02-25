package com.android.localcall.socket;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.net.ServerSocket;
import java.net.Socket;

import android.os.Process;

import com.android.localcall.utils.DEBUG;

public class CtlServerSocket extends Thread
{
	private static final String TAG = "CtlServerSocket";

	private ServerSocket mServerSocket = null;
	private Socket mSocket = null;
	private BufferedWriter mOutputStream = null;
	private BufferedReader mInputStream = null;
	private int mPort = 0;
	private boolean mStop = false;

	private CtlServerCallBack mCtlServerCallBack = null;

	public CtlServerSocket()
	{
		super("CtlServerSocket");
		DEBUG.log(TAG, "CtlServerSocket ");
	}

	/**
	 * 打开连接
	 * 
	 * @return
	 */
	public int open(CtlServerCallBack callback)
	{
		if (mServerSocket != null)
		{
			DEBUG.log(TAG, "CtlServerSocket open failed because has been opened");
			return -1;
		}

		mCtlServerCallBack = callback;

		try
		{
			mServerSocket = new ServerSocket(0);
			mServerSocket.setSoTimeout(10000);
			mPort = mServerSocket.getLocalPort();
		}
		catch (IOException e)
		{
			DEBUG.log(TAG, "CtlServerSocket open e:" + e);
		}

		if (mPort > 0)
		{
			mStop = false;
			start();
			return mPort;
		}
		else
		{
			mServerSocket = null;
			return -1;
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
				DEBUG.log(TAG, "writeCommand command:" + command);

				mOutputStream.write(command + "\r\n");
				mOutputStream.flush();
			}
			catch (IOException e)
			{
				e.printStackTrace();
			}
		}
	}

	/**
	 * 打开下一个连接
	 */
	public void openNext()
	{
		DEBUG.log(TAG, "openNext");

		closeStream();
	}

	/**
	 * 关闭服务端
	 */
	public void close()
	{
		DEBUG.log(TAG, "close");
		mStop = true;
		closeSocket();
	}

	/**
	 * 返回服务器端口
	 * 
	 * @return
	 */
	public int getPort()
	{
		return mPort;
	}

	@Override
	public void run()
	{
		process();
	}

	private void process()
	{

		Process.setThreadPriority(NORM_PRIORITY);

		while (!mStop)
		{
			DEBUG.log(TAG, "process start mPort:" + mPort);

			try
			{
				mSocket = mServerSocket.accept();

				DEBUG.log(TAG, "process accept new socket:" + mSocket);

				mOutputStream = new BufferedWriter(new OutputStreamWriter(mSocket.getOutputStream()));
				mInputStream = new BufferedReader(new InputStreamReader(mSocket.getInputStream()));

				if (mCtlServerCallBack != null)
				{
					mCtlServerCallBack.newCommand("server socket connected");
				}

				DEBUG.log(TAG, "process stream over:");

				while (true)
				{
					String ret = mInputStream.readLine();

					DEBUG.log(TAG, "process readline:" + ret);

					if (ret == null)
					{
						if (mCtlServerCallBack != null)
						{
							mCtlServerCallBack.newCommand("server socket disconnected");
						}
						break;
					}

					if (mCtlServerCallBack != null && ret != null)
					{
						mCtlServerCallBack.newCommand(ret);
					}
				}
			}
			catch (Exception e)
			{
				DEBUG.log(TAG, "process e1:" + e);
			}
			finally
			{
				closeSocket();
			}
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
				DEBUG.log(TAG, "closeSocket e:" + e);
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
				DEBUG.log(TAG, "closeStream e2:" + e);
			}
			finally
			{
				mInputStream = null;
			}
		}
	}

	/**
	 * @author dragon 状态回调
	 */
	public static interface CtlServerCallBack
	{
		void newCommand(String command);
	}
}
