package com.android.localcall.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.View;

public class VolumeView extends View
{
	private float mPercent = 0.5f;

	public VolumeView(Context context, AttributeSet attrs, int defStyleAttr)
	{
		super(context, attrs, defStyleAttr);
		// TODO Auto-generated constructor stub
	}

	public VolumeView(Context context, AttributeSet attrs)
	{
		super(context, attrs);
		// TODO Auto-generated constructor stub
	}

	public VolumeView(Context context)
	{
		super(context);
		// TODO Auto-generated constructor stub
	}

	@Override
	public void draw(Canvas canvas)
	{
		//描画背景
		Rect rec = new Rect(0, 0, getWidth(), getHeight());
		canvas.clipRect(rec);
		canvas.drawColor(Color.parseColor("#b9b9b9"));

		int w = getWidth();
		w = (int) ((w * mPercent) / 2);

		rec = new Rect(w, 0, getWidth() - w, getHeight());
		canvas.clipRect(rec);
		canvas.drawColor(Color.parseColor("#474747"));
		
		super.draw(canvas);
	}

}
