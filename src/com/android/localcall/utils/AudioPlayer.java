package com.android.localcall.utils;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

public class AudioPlayer
{
	private final String TAG = "AudioPlayer";
	
	private AudioTrack mAudioTrack = null;

	public AudioPlayer()
	{
	}

	/**
	 * 开始播放声音
	 */
	public void startAudioPlayer()
	{
		DEBUG.log(TAG, "startAudioPlayer start");
		// 获得音频缓冲区大小
		int bufferSize = android.media.AudioTrack.getMinBufferSize(8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT);
		DEBUG.log(TAG, "startAudioPlayer bufferSize:"+bufferSize);
		// 获得音轨对象
		mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, 8000, AudioFormat.CHANNEL_CONFIGURATION_MONO, AudioFormat.ENCODING_PCM_16BIT, bufferSize, AudioTrack.MODE_STREAM);
		// 设置喇叭音量
		mAudioTrack.setStereoVolume(0.8f, 0.8f);
		// 开始播放声音
		mAudioTrack.play();
		DEBUG.log(TAG, "startAudioPlayer end");
	}
	/**
	 * 停止播放声音
	 */
	public void stopAudioPlayer()
	{
		DEBUG.log(TAG, "stopAudioPlayer start");
		if (mAudioTrack != null)
		{
			mAudioTrack.stop();
			mAudioTrack.release();
			mAudioTrack = null;
		}
		DEBUG.log(TAG, "stopAudioPlayer end");
	}
	
	/**
	 * 写数据到player
	 * @param data
	 */
	public void writeDataToPlayer(byte [] data)
	{
		if (mAudioTrack != null)
		{
			mAudioTrack.write(data, 0, data.length);
		}
	}
	/**
	 * 
	 */
	public void writeDataToPlayer(short [] data, int size)
	{
		if (mAudioTrack != null)
		{
			mAudioTrack.write(data, 0, size);
		}
	}
}