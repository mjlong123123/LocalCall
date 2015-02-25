package com.android.localcall.utils;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

public class AudioRecorder extends Thread
{
	private final String TAG = "AudioRecorder";
	//音频帧大小
	private int FRAME_SIZE_AUDIO = 160;
	//录音用
	private AudioRecord mAudioRecord = null;
	//录音数据回调
	private IRecorderDataCallBack mIRecorderDataCallBack = null;

	public AudioRecorder(int framesize)
	{
		FRAME_SIZE_AUDIO = framesize;
	}
	
	/**
	 * 设置数据回调
	 * @param callback
	 */
	public void setRecorderDataCallBack(IRecorderDataCallBack callback)
	{
		mIRecorderDataCallBack = callback;
	}

	/**
	 * 开始录音
	 */
	public void startAudioRecorder()
	{
		if (!isAlive())
		{
			DEBUG.log(TAG, "startAudioRecorder");
			start();
		}
	}

	/**
	 * 停止录音
	 */
	public void stopAudioRecorder()
	{
		if (isAlive() && !interrupted())
		{
			DEBUG.log(TAG, "stopAudioRecorder");
			interrupt();
		}
	}

	@Override
	public void run()
	{
		try
		{
			// 获得录音缓冲区大小
			int bufferSize = AudioRecord.getMinBufferSize(8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
			DEBUG.log(TAG, "run  bufferSize:"+bufferSize);
			// 获得录音机对象
			mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize);
			// 开始录音
			mAudioRecord.startRecording();
			//数据buffer
			short[] readBuffer = new short[FRAME_SIZE_AUDIO];
			int count = 0;
			int index = 0;

			while (!isInterrupted())
			{
				// 从mic读取音频数据
				count = mAudioRecord.read(readBuffer, index, FRAME_SIZE_AUDIO - index);
				DEBUG.log(TAG, "run  count:"+count);
				if (count > 0)
				{
					index += count;
				}

				if (index == FRAME_SIZE_AUDIO)
				{
					index = 0;
					DEBUG.log(TAG, "run  send data");
					if(mIRecorderDataCallBack != null)
					{
						mIRecorderDataCallBack.RecorderDataCallBack(readBuffer);
					}
				}
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
		finally
		{
			DEBUG.log(TAG, "run finally");
			if (mAudioRecord != null)
			{
				mAudioRecord.stop();
				mAudioRecord.release();
				mAudioRecord = null;
			}
		}

	}
	
	/**
	 * 录音数据返回
	 * @author dragon
	 *
	 */
	public static interface IRecorderDataCallBack
	{
		public void RecorderDataCallBack(short [] data);
	}
}
