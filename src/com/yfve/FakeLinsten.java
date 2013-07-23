package com.yfve;

import java.util.List;

import android.util.Log;

import vehicle_CVS.*;

public class FakeLinsten implements VehicleTransitListen_DSP {

	@Override
	public void yz_3_remotedescallback(int poinum, List<POIStruct_DSP> poilist,
			int result) {
		Log.d("call back", "yz_3_remotedescallback called");
		Log.d("call back", String.format(
				"remotecallback poinum = %d result = %d", poinum, result));
	}

	@Override
	public void yz_3_loginstatecallback(String terminalID, long userId,
			String userName, int state) {
		Log.d("call back", "yz_3_loginstatecallback called");

	}

	@Override
	public void yz_3_TMCcallback(TMCStruct_DSP tmcStruct) {
		Log.d("call back", "yz_3_TMCcallback called");

	}

	@Override
	public void yz_3_weathercallback(WeatherStruct_DSP weatherStruct) {
		Log.d("call back", "yz_3_weathercallback called");

	}

}
