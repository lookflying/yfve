package vehicle_CVS;

public class VehicleDataStruct_CVS {
	/* int rotationSpeed; //转速，单位（rpm） */
	int speed; // 车速，单位（km/h）
	int oilLevel; // 油位，单位升
	/*
	 * int coolantTemperature; //冷却液温度， 0为cool，1为3/8,2为1/2,3为hot int
	 * engineOilPressure; //机油压力， 0为low，1为normal int absState;
	 * //abs状态，0为abnormal，1为normal int tc; //牵引力控制（TC）， 0为off，1为on int
	 * esc;//电子防滑ESC， 0为0ff,1为on int gear;//档位，
	 * 0为P，1为R，2为N，3为D，4为2(L),5为1,6为M1,7为M2,8为M3,9为M4,10为M5,11为M6 int
	 * cruise;//定速巡航，0为off，1为on int antiTheft;//整车防盗，0为off，1为on int
	 * tirePressure;//胎压，单位pa int driveMode;//驾驶模式，0为ECO，1为Sport
	 */
	int ACCSwitch; // ACC状态，0为关，1为开
	int located; // 定位状态,0未定位，1为定位
	int operating; // 0运营状态， 1停运状态
	int oilCircuit; // 油路，0正常，1断开
	int electricCircuit; // 电路，0正常，1断开
	int doorLocked; // 0车门解锁，1车门加锁

	long mileage;// 里程，对应车上里程表得读数，单位km

	long time;
	double longitude;
	double latitude;

	public void setSpeed(int speed) {
		this.speed = speed;
	}

	public void setOilLevel(int oilLevel) {
		this.oilLevel = oilLevel;
	}

	public void setACCSwitch(int ACCSwitch) {
		this.ACCSwitch = ACCSwitch;
	}

	public void setLocated(int located) {
		this.located = located;
	}

	public void setOperating(int operating) {
		this.operating = operating;
	}

	public void setOilCircuit(int oilCircuit) {
		this.oilCircuit = oilCircuit;
	}

	public void setElectricCircuit(int electricCircuit) {
		this.electricCircuit = electricCircuit;
	}

	public void setDoorLocked(int doorLocked) {
		this.doorLocked = doorLocked;
	}

	public void setMileage(int mileage) {
		this.mileage = mileage;
	}

	public void setTime(long time) {
		this.time = time;
	}

	public void setLongitude(double longitude) {
		this.longitude = longitude;
	}

	public void setLatitude(double latitude) {
		this.latitude = latitude;
	}

}
