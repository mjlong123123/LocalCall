package com.android.localcall.data;

public class BufferDataExt extends BufferData
{
	public BufferDataExt()
	{
	}

	public BufferDataExt(int size)
	{
		if (size != 0)
			mBufferExt = new short[size];
	}

	public short[] mBufferExt = null;
}
