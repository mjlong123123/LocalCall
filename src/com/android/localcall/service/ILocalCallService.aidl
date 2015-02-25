package com.android.localcall.service;
import com.android.localcall.service.ILocalCallServiceCallback;

interface ILocalCallService
{
	//注册回调
	void registerCallback(ILocalCallServiceCallback callback);
	//发送组播
	boolean startBroadcast();
	//接收组播
	boolean stopBroadcast();
	//开启语音
	boolean startRecorder(String ip);
	//关闭语音
	boolean stopRecorder();
	//开始播放
	boolean startPlayer();
	//停止播放
	boolean stopPlayer();
	//连接控制服务
	boolean connectServer(String ip, int port);
	//client端发送命令
	boolean writeCommandFromClient(String command);
	//服务端发送命令
	boolean writeCommandFromServer(String command);
	//断开连接控制服务
	boolean disconnectServer();
	//重启控制服务
	boolean restartCtlServer();
}