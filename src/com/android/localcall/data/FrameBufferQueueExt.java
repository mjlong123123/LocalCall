package com.android.localcall.data;

import java.util.LinkedList;

/**
 * @author dragon 现生成缓存，没有缓存池
 */
public class FrameBufferQueueExt extends LinkedList<BufferData>
{
	private static final long serialVersionUID = 1L;
	//默认大小
	private static final int SIZE = 3;
	//数据个数
	private int mCount = 0;
	//buffer上线个数
	private int mBufferCount = 0;
	//buffer size
	private int mBufferSize = 0;

	/**
	 * @param datasize
	 * @param flag
	 *            true 是扩展的short类型，false是byte类型
	 */
	public FrameBufferQueueExt(int datasize)
	{
		super();
		mBufferCount = SIZE;
		mBufferSize = datasize;
	}

	/**
	 * @param size
	 * @param datasize
	 * @param flag
	 *            true 是扩展的short类型，false是byte类型
	 */
	public FrameBufferQueueExt(int size, int datasize)
	{
		super();
		if (size > SIZE)
			mBufferCount = size;
		else
			mBufferCount = SIZE;
		mBufferSize = datasize;
	}

	/**
	 * 清除数据
	 */
	public void clearCustom()
	{
		mCount = 0;
		clear();
	}

	/**
	 * 弹出数据
	 * 
	 * @return
	 */
	public BufferData pollCustom()
	{
		if (mCount == 0)
		{
			return null;
		}

		mCount--;
		return poll();
	}

	/**
	 * 弹出数据
	 * 
	 * @return
	 */
	public BufferDataExt pollCustomShort()
	{
		if (mCount == 0)
		{
			return null;
		}
		mCount--;
		BufferData temp = poll();
		
		if (temp instanceof BufferDataExt)
			return (BufferDataExt) temp;
		else
			return null;
	}

	/**
	 * 加入数据
	 */
	public void offerCustom(BufferData data)
	{
		if (data != null)
		{
			mCount++;
			offer(data);
		}
	}

	/**
	 * 产生buffer数据
	 * 
	 * @return
	 */
	public BufferData generateBufferData()
	{
		BufferData ret = null;
		if (mCount >= mBufferCount)
		{
			ret = pollCustom();
		}
		else
		{
			ret = new BufferData(mBufferSize);
		}
		return ret;
	}

	/**
	 * 产生buffer数据
	 * 
	 * @return
	 */
	public BufferDataExt generateBufferDataShort()
	{
		BufferDataExt ret = null;
		if (mCount >= mBufferCount)
		{
			ret = pollCustomShort();
		}
		else
		{
			ret = new BufferDataExt(mBufferSize);
		}
		return ret;
	}
}
