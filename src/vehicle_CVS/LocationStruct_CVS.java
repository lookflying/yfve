package vehicle_CVS;
public class LocationStruct_CVS {
	long time;
	double longitude;
	double latitude;
	float speed;
	float direction;
	float altitude;
	int satnum;
	int alarmstatus;
	
	
	public void setTime(long time)
	{
		this.time = time;
	}
	
	public void setLongitude(double longitude)
	{
		this.longitude = longitude;
	}
	
	public void setLatitude(double latitude)
	{
		this.latitude = latitude;
	}
	
	public void setSpeed(float speed)
	{
		this.speed = speed;
	}
	
	public void setDirection(float direction)
	{
		this.direction = direction;
	}
	
	public void setAltitude(float altitude)
	{
		this.altitude = altitude;
	}
	
	public void setSatnum(int satnum)
	{
		this.satnum = satnum;
	}
	
	public void setAlarmType(int alarmstatus)
	{
		this.alarmstatus = alarmstatus;
	}
}
