package com.yfve;

import vehicle_CVS.LocationStruct_CVS;
import vehicle_CVS.VehicleDataStruct_CVS;
import vehicle_CVS.YZ_VehicleTransit_CVS;
import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {
	private Button button1;
	private Button button2;
	private Button button3;
	private Button button4;
	private EditText text1;
	private EditText text2;
	private EditText text3;
	private EditText text4;
	private EditText text5;
	private EditText text6;
	private Button button5;
	private Button button6;
	private Button button7;
	private Button button8;
	private Button button9;
	private EditText text7;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		button1 = (Button) findViewById(R.id.button1);
		button2 = (Button) findViewById(R.id.button2);
		button3 = (Button) findViewById(R.id.button3);
		button4 = (Button) findViewById(R.id.button4);
		button5 = (Button) findViewById(R.id.button5);
		button6 = (Button) findViewById(R.id.button6);
		text1 = (EditText) findViewById(R.id.editText1);
		text2 = (EditText) findViewById(R.id.editText2);
		text3 = (EditText) findViewById(R.id.editText3);
		text4 = (EditText) findViewById(R.id.editText4);
		text5 = (EditText) findViewById(R.id.editText5);
		text6 = (EditText) findViewById(R.id.editText6);
		button7 = (Button) findViewById(R.id.button7);
		button8 = (Button) findViewById(R.id.button8);
		button9 = (Button) findViewById(R.id.button9);
		text7 = (EditText) findViewById(R.id.editText7);

		button1.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				int ret = YZ_VehicleTransit_CVS.yz_2_register("13816429129", 0,
						0, "", "", "", 0, "苏C0001");
				text1.setText(String.format("register return value = %d", ret));
				String authCode = YZ_VehicleTransit_CVS.yz_2_getAuthCode();
				text2.setText(String.format("authcode = %s", authCode));
			}

		});

		button2.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				int ret = YZ_VehicleTransit_CVS.yz_2_deregister();
				text1.setText(String
						.format("deregister return value = %d", ret));
			}

		});

		button3.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				int ret = YZ_VehicleTransit_CVS.yz_2_userlogin("13816429129",
						YZ_VehicleTransit_CVS.yz_2_getAuthCode(), "", null);
				text1.setText(String.format("login return value = %d", ret));
			}

		});

		button4.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				int ret = YZ_VehicleTransit_CVS.yz_2_userlogout("12345678901",
						"");
				text1.setText(String.format("logout return value = %d", ret));
			}

		});

		button5.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				LocationStruct_CVS loc = new LocationStruct_CVS();
				// loc.setLatitude(31.177326);
				// loc.setLongitude(121.455665);
				loc.setLatitude(30.87638);
				loc.setLongitude(121.38088);
				loc.setTime(System.currentTimeMillis());
				int ret = YZ_VehicleTransit_CVS.yz_3_gpsupload("", loc);
				text1.setText(String
						.format("gps upload return value = %d", ret));
			}

		});

		button6.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				VehicleDataStruct_CVS vdata = new VehicleDataStruct_CVS();
				vdata.setLocated(1);
				vdata.setSpeed(5);
				vdata.setOilLevel(200);
				vdata.setACCSwitch(1);
				vdata.setLocated(0xffff);
				vdata.setOilCircuit(1);
				vdata.setElectricCircuit(1);
				vdata.setDoorLocked(1);
				vdata.setOperating(1);
				vdata.setMileage(600);
				vdata.setLatitude(31.152993);
				vdata.setLongitude(121.805543);
				vdata.setTime(System.currentTimeMillis());
				int ret = YZ_VehicleTransit_CVS.yz_3_sendvehicledata(
						"12345678901", vdata);
				text1.setText(String.format(
						"send vehicle data return value = %d", ret));
			}

		});

		button7.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				YZ_VehicleTransit_CVS.yz_2_prepare_class();
				YZ_VehicleTransit_CVS.yz_2_init("", text5.getText().toString(),
						Integer.valueOf(text6.getText().toString()),
						new FakeListener(uiHandler));

				text3.setText("Middleware started");
			}

		});

		button8.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				YZ_VehicleTransit_CVS.yz_2_destroy();
				text3.setText("Middleware stopped");
			}
		});

		button9.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				YZ_VehicleTransit_CVS.yz_3_test();
			}
		});

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	public static final int LOGIN_ID = 0;
	public static final String LOGIN_CONTENT = "login";
	public static final int TEST_ID = 1;
	public static final String TEST_CONTENT = "test";
	private final Handler uiHandler = new Handler() {
		public void handleMessage(Message msg) {
			super.handleMessage(msg);
			switch (msg.what) {
			case LOGIN_ID:
				text4.append(msg.getData().getString(LOGIN_CONTENT));
				break;
			case TEST_ID:
			default:
				text7.append(msg.getData().getString(TEST_CONTENT));
				break;
			}
		}
	};
}
