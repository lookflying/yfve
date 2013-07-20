package vehicle_CVS;

public class VehicleDataStruct_CVS {
	int rotationSpeed;	//转速，单位（rpm）	
	int speed;	//车速，单位（kmh）
	int oilLevel;	//油位，0为E，1为1/4,2为3/4,3为F
	int coolantTemperature;	//冷却液温度， 0为cool，1为3/8,2为1/2,3为hot
	int engineOilPressure;	//机油压力， 0为low，1为normal
	int absState;	//abs状态，0为abnormal，1为normal
	int tc;	//牵引力控制（TC）， 0为off，1为on
	int esc;//电子防滑ESC， 0为0ff,1为on
	int gear;//档位， 0为P，1为R，2为N，3为D，4为2(L),5为1,6为M1,7为M2,8为M3,9为M4,10为M5,11为M6
	int cruise;//定速巡航，0为off，1为on
	int antiTheft;//整车防盗，0为off，1为on
	int tirePressure;//胎压，单位pa
	int driveMode;//驾驶模式，0为ECO，1为Sport
	
}
