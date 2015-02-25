package com.android.localcall.data;

public class BufferData
{
	public BufferData()
	{
	}

	public BufferData(int size)
	{
		if (size != 0)
			mBuffer = new byte[size];
	}

	public byte[] mBuffer = null;
	public int mState = -1;//-1没有使用，0正在使用。
}
