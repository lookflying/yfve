package vehicle_CVS;
public class LocationStruct_CVS {
	long time;// 单位 毫秒
	double longitude;// 经度,单位：度
	double latitude;// 纬度，单位：度
	float speed;// 速度 单位：km/h
	float direction;// 方向 单位度，正北为0，顺时针
	float altitude;// 高度 单位：米
	int satnum;// 卫星数量
	int alarmstatus;// 报警状态，0为正常，1为紧急 报警，2掉电报警，3为超速报警
}
