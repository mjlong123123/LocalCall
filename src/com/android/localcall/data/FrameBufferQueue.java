package com.android.localcall.data;

import java.util.LinkedList;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * @author dragon
 * 使用缓存池，recycleBufferData回收前就不能使用。
 */
public class FrameBufferQueue extends ConcurrentLinkedQueue<BufferData>
{
	private static final long serialVersionUID = 1L;

	private static final int SIZE = 2;

	private BufferData[] mDataBuffer = null;

	/**
	 * @param datasize
	 * @param flag
	 *            true 是扩展的short类型，false是byte类型
	 */
	public FrameBufferQueue(int datasize, boolean flag)
	{
		super();
		init(SIZE, datasize, flag);
	}

	/**
	 * @param size
	 * @param datasize
	 * @param flag
	 *            true 是扩展的short类型，false是byte类型
	 */
	public FrameBufferQueue(int size, int datasize, boolean flag)
	{
		super();

		if (size >= 2)
			init(size, datasize, flag);
		else
			init(SIZE, datasize, flag);
	}

	/**
	 * 初始化buffer
	 * 
	 * @param size
	 *            队列大小
	 * @param datasize
	 *            每一个数据单元byte数据大小
	 * @param flag
	 *            true 是扩展的short类型，false是byte类型
	 */
	private void init(int size, int datasize, boolean flag)
	{
		if (flag)
		{
			mDataBuffer = new BufferDataExt[size];
		}
		else
		{
			mDataBuffer = new BufferData[size];
		}
		if (flag)
		{
			for (int i = 0; i < size; i++)
			{
				mDataBuffer[i] = new BufferDataExt(datasize);
			}
		}
		else
		{
			for (int i = 0; i < size; i++)
			{
				mDataBuffer[i] = new BufferData(datasize);
			}
		}
	}

	/**
	 * 弹出数据
	 * 
	 * @return
	 */
	public BufferData pollCustom()
	{
		return poll();
	}

	/**
	 * 回收数据
	 * 
	 * @param data
	 */
	public void recycleBufferData(BufferData data)
	{
		if (data != null)
		{
			data.mState = -1;
		}
	}

	/**
	 * 加入数据
	 */
	public void offerCustom(BufferData data)
	{
		if (data != null)
		{
			offer(data);
		}
	}

	/**
	 * 重设buffer
	 */
	public void reset()
	{
		clear();

		if (mDataBuffer != null)
		{
			//取空闲缓存区
			for (int i = 0; i < mDataBuffer.length; i++)
			{
				mDataBuffer[i].mState = -1;
			}
		}
	}

	/**
	 * 产生buffer数据
	 * 
	 * @return
	 */
	public BufferData generateBufferData()
	{
		if (mDataBuffer == null)
		{
			return null;
		}

		BufferData ret = null;
		//取空闲缓存区
		for (int i = 0; i < mDataBuffer.length; i++)
		{
			if (mDataBuffer[i].mState == -1)
			{
				mDataBuffer[i].mState = 0;
				ret = mDataBuffer[i];
				break;
			}
		}
		//从队列中删除正在使用的数据
		if (ret == null)
		{
			ret = poll();
		}

		return ret;
	}
}
