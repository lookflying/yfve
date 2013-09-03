package com.yfve;

import java.util.Date;
import java.text.SimpleDateFormat;
import java.util.List;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.format.DateFormat;
import android.util.Log;

import vehicle_CVS.*;

public class FakeListener implements VehicleTransitListen_DSP {
	FakeListener(Handler handler) {
		mHandler = handler;
	}

	Handler mHandler;

	@Override
	public void yz_3_remotedescallback(int poinum, List<POIStruct_DSP> poilist,
			int result) {

		Log.d("call back", "yz_3_remotedescallback called");
		Log.d("call back", String.format(
				"remotecallback poinum = %d result = %d", poinum, result));
		int count = poilist.size();
		if (count > 0) {
			Log.d("call back", String.format(
					"latitude = %f\t longtitude = %f\t name = %s",
					poilist.get(0).getLatitude(), poilist.get(0)
							.getLongtitude(), poilist.get(0).getPoiname()));
		}

	}

	@Override
	public void yz_3_loginstatecallback(String terminalID, long userId,
			String userName, int state) {
		Log.d("call back", "yz_3_loginstatecallback");
		Log.d("call back", String.format(
				"tid = %s, userId = %d, userName = %s, state = %d", terminalID,
				userId, userName, state));
		if (mHandler != null) {
			SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
			Bundle bundle = new Bundle();
			bundle.putString(
					MainActivity.LOGIN_CONTENT,
					String.format(
							"%s tid = %s, userId = %d, userName = %s, state = %d\n",
							sdf.format(new Date()), terminalID, userId,
							userName, state));
			Message msg = mHandler.obtainMessage(MainActivity.LOGIN_ID);
			msg.setData(bundle);
			mHandler.sendMessage(msg);
		}

	}

	@Override
	public void yz_3_TMCcallback(TMCStruct_DSP tmcStruct) {

		Log.d("call back", "yz_3_TMCcallback called");

	}

	@Override
	public void yz_3_weathercallback(WeatherStruct_DSP weatherStruct) {
		try {
			Log.d("call back", "yz_3_weathercallback called");

		} catch (Exception e) {
			Log.d("Exception", "in weather" + e.getMessage());
		}

	}

}
