package com.android.localcall.utils;

import java.util.concurrent.LinkedBlockingQueue;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;

import com.android.localcall.data.BufferData;
import com.android.localcall.data.BufferDataExt;
import com.android.localcall.data.FrameBufferQueue;
import com.android.localcall.data.FrameBufferQueueNew;
import com.android.localcall.jni.Rtp;

public class AudioSync extends Thread
{
	private final String TAG = "AudioSync";
	//播放声音用
	private AudioTrack mAudioTrack = null;
	//录音用
	private AudioRecord mAudioRecord = null;
	//音频帧大小
	private int FRAME_SIZE_AUDIO = 160;
	//命令队列
	private LinkedBlockingQueue<Command> mCommandQueue = null;
	//输入流buffer，播放数据
	private FrameBufferQueueNew mInputFrameBufferQueue = null;
	//输出流buffer，录音数据
	private FrameBufferQueueNew mOutputFrameBufferQueue = null;
	
	private Rtp mRtp = null;
	
	short [] datadefault = new short[160];
	//命令字定义
	enum Command
	{
		START_PLAYER, STOP_PLAYER, START_RECORDER, STOP_RECORDER, DEFAULT
	};

	/**
	 * 构造函数
	 */
	public AudioSync()
	{
		init();
	}
	
	public void setRtp(Rtp rtp)
	{
		mRtp = rtp;
	}
	/**
	 * 启动播放
	 */
	public void startPlayer()
	{
		if(mCommandQueue != null)
		{
			mCommandQueue.offer(Command.START_PLAYER);
		}
	}
	
	/**
	 * 停止播放
	 */
	public void stopPlayer()
	{
		if(mCommandQueue != null)
		{
			mCommandQueue.offer(Command.STOP_PLAYER);
		}
	}
	
	/**
	 * 开始录音
	 */
	public void startRecorder()
	{

		if(mCommandQueue != null)
		{
			mCommandQueue.offer(Command.START_RECORDER);
		}
	}
	/**
	 * 停止录音
	 */
	public void stopRecorder()
	{
		if(mCommandQueue != null)
		{
			mCommandQueue.offer(Command.STOP_RECORDER);
		}
	}

	/**
	 * 写播放数据
	 */
	public void writeDataToPlayer(short[] data)
	{
		if (data != null && mInputFrameBufferQueue != null && data.length == FRAME_SIZE_AUDIO)
		{
			BufferData bd = mInputFrameBufferQueue.generateBufferData();
			if (bd != null)
			{
				BufferDataExt bde = (BufferDataExt) bd;
				if (bde.mBufferExt != null)
				{
					System.arraycopy(data, 0, bde.mBufferExt, 0, FRAME_SIZE_AUDIO);
					mInputFrameBufferQueue.offerCustom(bd);
					return;
				}
				mInputFrameBufferQueue.recycleBufferData(bd);
			}
		}
	}

	/**
	 * 读录音数据
	 * 
	 * @return
	 */
	public short[] readDataFromRecorder()
	{
		short[] buffer = null;

		if (mOutputFrameBufferQueue != null)
		{
			BufferData bd = mOutputFrameBufferQueue.pollCustom();
			if (bd != null)
			{
				BufferDataExt bde = (BufferDataExt) bd;
				if (bde.mBufferExt != null)
				{
					buffer = new short[FRAME_SIZE_AUDIO];
					System.arraycopy(bde.mBufferExt, 0, buffer, 0, FRAME_SIZE_AUDIO);
				}
				mOutputFrameBufferQueue.recycleBufferData(bd);
			}
		}
		return buffer;
	}

	private void init()
	{
		for(int i = 0; i < datadefault.length; i++)
		{
			datadefault[i] = 0;
		}
		mCommandQueue = new LinkedBlockingQueue<Command>();
		mInputFrameBufferQueue = new FrameBufferQueueNew(100, FRAME_SIZE_AUDIO, true);
		mOutputFrameBufferQueue = new FrameBufferQueueNew(100, FRAME_SIZE_AUDIO, true);
	}

	/**
	 * 开始播放声音
	 */
	private void startAudioPlayer()
	{
		DEBUG.log(TAG, "startAudioPlayer start");
		if (mAudioTrack != null && mAudioTrack.getState() != AudioTrack.STATE_UNINITIALIZED)
		{
			DEBUG.log(TAG, "startAudioPlayer start already");
			return;
		}
		// 获得音频缓冲区大小
		int bufferSize = android.media.AudioTrack.getMinBufferSize(8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
		DEBUG.log(TAG, "startAudioPlayer bufferSize:" + bufferSize);
		// 获得音轨对象
		mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
		// 设置喇叭音量
		mAudioTrack.setStereoVolume(1.0f, 1.0f);
		// 开始播放声音
		mAudioTrack.play();
		DEBUG.log(TAG, "startAudioPlayer end");
	}

	/**
	 * 停止播放声音
	 */
	private void stopAudioPlayer()
	{
		DEBUG.log(TAG, "stopAudioPlayer start");
		if (mAudioTrack != null && mAudioTrack.getState() != AudioTrack.STATE_UNINITIALIZED)
		{
			mAudioTrack.stop();
			mAudioTrack.release();
			mAudioTrack = null;
		}
		DEBUG.log(TAG, "stopAudioPlayer end");
	}

	/**
	 * 启动录音
	 */
	private void startAudioRecorder()
	{
		DEBUG.log(TAG, "startAudioRecorder start");
		if (mAudioRecord != null && mAudioRecord.getState() != AudioRecord.STATE_UNINITIALIZED)
		{
			DEBUG.log(TAG, "startAudioRecorder start already");
			return;
		}
		// 获得录音缓冲区大小
		int bufferSize = AudioRecord.getMinBufferSize(8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
		DEBUG.log(TAG, "run  bufferSize:" + bufferSize);
		// 获得录音机对象
		mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.MIC, 8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize);
		// 开始录音
		mAudioRecord.startRecording();
		DEBUG.log(TAG, "startAudioRecorder end");
	}

	/**
	 * 停止录音
	 */
	private void stopAudioRecorder()
	{
		DEBUG.log(TAG, "stopAudioRecorder start");
		if (mAudioRecord != null && mAudioRecord.getState() != AudioRecord.STATE_UNINITIALIZED)
		{
			mAudioRecord.stop();
			mAudioRecord.release();
			mAudioRecord = null;
		}
		DEBUG.log(TAG, "stopAudioRecorder end");
	}

	/**
	 * 处理命令
	 * 
	 * @param state
	 */
	private void processCommand(Command cmd)
	{
		if (cmd == null)
		{
			return;
		}

		DEBUG.log(TAG, "reserveCommand cmd :" + cmd);

		switch (cmd)
		{
			case START_PLAYER:
				startAudioPlayer();
				break;
			case STOP_PLAYER:
				stopAudioPlayer();
				break;
			case START_RECORDER:
				startAudioRecorder();
				break;
			case STOP_RECORDER:
				stopAudioRecorder();
				break;
			default:
				break;
		}
	}

	/**
	 * 处理播放数据
	 */
	private void processPlayerData()
	{
		if (mInputFrameBufferQueue != null)
		{
			BufferData bd = mInputFrameBufferQueue.pollCustom();
			if (bd != null)
			{
				BufferDataExt bde = (BufferDataExt) bd;

				if (mAudioTrack != null && mAudioTrack.getState() != AudioTrack.STATE_UNINITIALIZED && bde.mBufferExt != null)
				{
//					mRtp.playback(bde.mBufferExt, bde.mBufferExt.length);
					mAudioTrack.write(bde.mBufferExt, 0, bde.mBufferExt.length);
					mInputFrameBufferQueue.recycleBufferData(bd);
					return;
				}
				else
				{
					//TODO 记录回音参照
//					mRtp.playback(datadefault, 160);
				}

				mInputFrameBufferQueue.recycleBufferData(bd);
			}
			else
			{
				//TODO 记录回音参照（无声）
//				mRtp.playback(datadefault, 160);
			}
		}
		//TODO 记录回音参照
//		mRtp.playback(datadefault, 160);
	}

	/**
	 * 处理录音
	 */
	private void processRecoderData()
	{
		if (mOutputFrameBufferQueue == null)
		{
			DEBUG.log(TAG, "processRecoderData mOutputFrameBufferQueue == null");
			return;
		}
		if (mAudioRecord != null && mAudioRecord.getState() != AudioRecord.STATE_UNINITIALIZED)
		{
			BufferData bd = mOutputFrameBufferQueue.generateBufferData();
			if (bd != null)
			{
				BufferDataExt bde = (BufferDataExt) bd;
				if (bde.mBufferExt != null)
				{
					int count = mAudioRecord.read(bde.mBufferExt, 0, bde.mBufferExt.length);
					if (count > 0)
					{
						//TODO 消除回声
//						mRtp.capture(bde.mBufferExt, bde.mBufferExt.length);
						mOutputFrameBufferQueue.offerCustom(bd);
						return;
					}
				}
				mOutputFrameBufferQueue.recycleBufferData(bd);
			}
		}
	}

	@Override
	public void run()
	{
		try
		{
			long lasttime = System.currentTimeMillis();
			Command cmd = null;
			while (!isInterrupted())
			{
//				DEBUG.log(TAG, "run time:" + (System.currentTimeMillis() - lasttime));
//				lasttime = System.currentTimeMillis();
				//处理命令
				cmd = mCommandQueue.poll();
				if (cmd != null)
					processCommand(cmd);
				//处理播放
				processPlayerData();
				//处理录音
				processRecoderData();
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
			DEBUG.log(TAG, "run Exception:" + e);
		}
		finally
		{
			DEBUG.log(TAG, "run finally");
			mCommandQueue.clear();
			mInputFrameBufferQueue.reset();
			mInputFrameBufferQueue.clear();
			mOutputFrameBufferQueue.reset();
			mOutputFrameBufferQueue.clear();
			//停止播放
			stopAudioPlayer();
			stopAudioRecorder();
		}
	}

	/**
	 * 录音数据返回
	 * 
	 * @author dragon
	 * 
	 */
	public static interface IRecorderDataCallBack
	{
		public void RecorderDataCallBack(short[] data);
	}
}