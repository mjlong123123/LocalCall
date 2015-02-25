package com.android.localcall.socket;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;

import com.android.localcall.data.BufferData;
import com.android.localcall.data.FrameBufferQueue;
import com.android.localcall.utils.DEBUG;

public class SocketExt
{
	private final String TAG = "SocketExt";
	//服务端socket
	private ServerSocket mServerSocket = null;
	//socket
	private Socket mSocket = null;
	//客户端连接的ip地址
	private String mIP = "";
	//socket连接使用的端口号
	private int mPort = 0;
	//数据缓存大小
	private int mFrameSize = 0;
	//缓存数据类型flag
	//	private boolean mTypeFlag = false;
	//输出流
	private OutputStream mOutputStream = null;
	//输入流
	private InputStream mInputStream = null;
	//输入流buffer
	private FrameBufferQueue mInputFrameBufferQueue = null;
	//输出流buffer
	private FrameBufferQueue mOutputFrameBufferQueue = null;
	//停止数据流读写
	private boolean mStreamStopFlag = false;
	//数据回调
	private IDataCallback mIDataCallback = null;
	//标识是否是服务端
	private boolean mIsServerFlag = false;

	/**
	 * @param ip
	 *            网络地址
	 * @param port
	 *            网络端口
	 * @param size
	 *            每一帧大小
	 * @param flag
	 *            true是short类型缓存，false是byte类型缓存
	 */
	public SocketExt(String ip, int port, int size)
	{
		mIP = ip;
		mPort = port;
		mFrameSize = size;
		//		mTypeFlag = flag;
		initFrameBuffer();
		mIsServerFlag = false;
	}

	//生成服务端socket
	public SocketExt(int port, int size)
	{
		mPort = port;
		mFrameSize = size;
		//		mTypeFlag = flag;
		initFrameBuffer();
		mIsServerFlag = true;
	}

	public boolean isServer()
	{
		return mIsServerFlag;
	}
	/**
	 * 设置数据回调
	 * 
	 * @param callback
	 */
	public void setIDataCallback(IDataCallback callback)
	{
		mIDataCallback = callback;
	}

	/**
	 * 取得输出缓存
	 * 
	 * @return
	 */
	public FrameBufferQueue getOutputBuffer()
	{
		return mOutputFrameBufferQueue;
	}

	/**
	 * 初始化缓存区
	 */
	private void initFrameBuffer()
	{
		mInputFrameBufferQueue = new FrameBufferQueue(5, mFrameSize, false);
		mOutputFrameBufferQueue = new FrameBufferQueue(5, mFrameSize, false);
	}

	/**
	 * 在子线程中执行启动服务端监听和传输。
	 */
	public void startServerSocket()
	{
		new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				try
				{
					mServerSocket = new ServerSocket(mPort);
					DEBUG.log(TAG, "processServerSocket accept start");
					mSocket = mServerSocket.accept();
					DEBUG.log(TAG, "processServerSocket accept end");
					DEBUG.log(TAG, "processServerSocket get stream over");
					startStream(mSocket);
				}
				catch (IOException e)
				{
					DEBUG.log(TAG, "processServerSocket e:" + e);
					try
					{
						if (mServerSocket != null && !mServerSocket.isClosed())
						{
							mServerSocket.close();
							mServerSocket = null;
						}
						if (mSocket != null && !mSocket.isClosed())
						{
							mSocket.close();
							mSocket = null;
						}
					}
					catch (IOException e1)
					{
						DEBUG.log(TAG, "processServerSocket e1:" + e1);
					}
				}
			}

		}).start();
	}

	/**
	 * 停止服务端监听和数据传输
	 */
	public void stopServerSocket()
	{
		try
		{
			if (mServerSocket != null && !mServerSocket.isClosed())
			{
				mServerSocket.close();
				mServerSocket = null;
			}
			if (mSocket != null && !mSocket.isClosed())
			{
				mSocket.close();
				mSocket = null;
			}
		}
		catch (IOException e1)
		{
			DEBUG.log(TAG, "stopServerSocket e1:" + e1);
		}
	}

	/**
	 * 连接服务端
	 */
	public void startSocket()
	{
		new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				try
				{
					DEBUG.log(TAG, "startSocket new Socket start");
					mSocket = new Socket(mIP, mPort);
					DEBUG.log(TAG, "startSocket new Socket end");
					startStream(mSocket);
					DEBUG.log(TAG, "startSocket new Socket over");
				}
				catch (IOException e)
				{
					DEBUG.log(TAG, "processServerSocket e:" + e);
					try
					{
						if (mSocket != null && !mSocket.isClosed())
						{
							mSocket.close();
							mSocket = null;
						}
					}
					catch (IOException e1)
					{
						DEBUG.log(TAG, "processServerSocket e1:" + e1);
					}
				}
			}

		}).start();
	}

	/**
	 * 断开服务端连接
	 */
	public void stopSocket()
	{
		try
		{
			if (mSocket != null && !mSocket.isClosed())
			{
				mSocket.close();
				mSocket = null;
			}
		}
		catch (IOException e1)
		{
			DEBUG.log(TAG, "stopSocket e1:" + e1);
		}

	}

	/**
	 * 建立数据交换线程，分数据输入和输出两个线程
	 */
	private void startStream(Socket socket)
	{
		if (socket == null)
		{
			DEBUG.log(TAG, "startStream socket == null");
			return;
		}

		//获取输入输出流
		try
		{
			mSocket.setKeepAlive(true);
			mSocket.setSoTimeout(5000);
			mOutputStream = mSocket.getOutputStream();
			mInputStream = mSocket.getInputStream();
		}
		catch (IOException e)
		{
			DEBUG.log(TAG, "startStream e:" + e);
			try
			{
				if (mOutputStream != null)
				{
					mOutputStream.close();
					mOutputStream = null;
				}
				if (mInputStream != null)
				{
					mInputStream.close();
					mInputStream = null;
				}
				if (mServerSocket != null && !mServerSocket.isClosed())
				{
					mServerSocket.close();
					mServerSocket = null;
				}
				if (mSocket != null && !mSocket.isClosed())
				{
					mSocket.close();
					mSocket = null;
				}
			}
			catch (IOException e1)
			{
				DEBUG.log(TAG, "startStream e1:" + e1);
			}
			return;
		}
		//输入流
		new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				//读取钱清空数据
				mInputFrameBufferQueue.reset();

				BufferData tempBufferData = null;
				byte[] tempbuffer = new byte[mFrameSize];
				byte[] buffer = null;
				int index = 0;
				int count = 0;
				while (!mStreamStopFlag && mInputStream != null)
				{
					try
					{
						//生成缓存buffer
						if (index == 0)
						{
							buffer = null;
							tempBufferData = mInputFrameBufferQueue.generateBufferData();
							if (tempBufferData != null)
							{
								buffer = tempBufferData.mBuffer;
							}
						}
						//空值保护
						if (buffer == null)
						{
							buffer = tempbuffer;
						}

						//读取网络数据
						count = mInputStream.read(buffer, index, buffer.length - index);

						if (count > 0)
						{
							index += count;
						}
						//判断是否写满一帧
						if (index == mFrameSize)
						{
							//写满一帧后换buffer写
							index = 0;
							if (tempBufferData != null)
							{
								mInputFrameBufferQueue.offerCustom(tempBufferData);
							}
						}
						else if(index == 0)
						{
							if (tempBufferData != null)
							{
								mInputFrameBufferQueue.recycleBufferData(tempBufferData);
							}
						}
					}
					catch (Exception e)
					{
						mStreamStopFlag = true;
						DEBUG.log(TAG, "startStream mInputStream:" + e);
						DEBUG.log(TAG, "startStream index:" + index);
						DEBUG.log(TAG, "startStream buffer.length:" + buffer.length);

						try
						{
							if (mOutputStream != null)
							{
								mOutputStream.close();
								mOutputStream = null;
							}
							if (mInputStream != null)
							{
								mInputStream.close();
								mInputStream = null;
							}
							if (mServerSocket != null && !mServerSocket.isClosed())
							{
								mServerSocket.close();
								mServerSocket = null;
							}
							if (mSocket != null && !mSocket.isClosed())
							{
								mSocket.close();
								mSocket = null;
							}
						}
						catch (Exception e11)
						{
							DEBUG.log(TAG, "startStream e1:" + e11);
						}
					}
				}
			}

		}).start();

		//输出流
		new Thread(new Runnable()
		{

			@Override
			public void run()
			{
				mOutputFrameBufferQueue.reset();

				BufferData tempBufferData = null;

				while (!mStreamStopFlag && mOutputStream != null)
				{
					try
					{
						tempBufferData = mOutputFrameBufferQueue.pollCustom();
						if (tempBufferData == null)
						{
							Thread.sleep(20);
							continue;
						}
						mOutputStream.write(tempBufferData.mBuffer);
						mOutputFrameBufferQueue.recycleBufferData(tempBufferData);
					}
					catch (Exception e)
					{
						mStreamStopFlag = true;
						DEBUG.log(TAG, "startStream mOutputStream:" + e);
						try
						{
							if (mOutputStream != null)
							{
								mOutputStream.close();
								mOutputStream = null;
							}
							if (mInputStream != null)
							{
								mInputStream.close();
								mInputStream = null;
							}
							if (mServerSocket != null && !mServerSocket.isClosed())
							{
								mServerSocket.close();
								mServerSocket = null;
							}
							if (mSocket != null && !mSocket.isClosed())
							{
								mSocket.close();
								mSocket = null;
							}
						}
						catch (Exception e11)
						{
							DEBUG.log(TAG, "startStream e1:" + e11);
						}
					}
				}
			}

		}).start();

		//启动读取数据buffer线程
		new Thread(new Runnable()
		{
			@Override
			public void run()
			{
				BufferData temp = null;
				while (!mStreamStopFlag)
				{
					try
					{
						temp = mInputFrameBufferQueue.pollCustom();
						if (temp == null)
						{
							Thread.sleep(20);
							continue;
						}

						if (mIDataCallback != null)
						{
							mIDataCallback.dataCallback(temp.mBuffer);
						}
						mInputFrameBufferQueue.recycleBufferData(temp);
					}
					catch (InterruptedException e)
					{
						DEBUG.log(TAG, "startStream e:" + e);
					}
				}
			}
		}).start();
	}

	/**
	 * 数据回调
	 * 
	 * @author dragon
	 * 
	 */
	public static interface IDataCallback
	{
		void dataCallback(byte[] buffer);
	}
}
