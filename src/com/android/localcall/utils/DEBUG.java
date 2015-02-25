package com.android.localcall.utils;

import android.util.Log;

public class DEBUG
{
	private static boolean mIsDebug = true;

	public static void setDebug(boolean flag)
	{
		mIsDebug = flag;
	}

	public static void log(String Tag, String msg)
	{
		if (mIsDebug)
		{
			Log.v(Tag, msg);
		}
	}
}
