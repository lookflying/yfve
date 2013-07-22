package vehicle_CVS;
public class LocationStruct_CVS {
	long mtime;
	double mlongitude;
	double mlatitude;
	float mspeed;
	float mdirection;
	float maltitude;
	int msatnum;
	int malarmstatus;
	
	
	public void setTime(long time)
	{
		mtime = time;
	}
	
	public void setLongitude(double longitude)
	{
		mlongitude = longitude;
	}
	
	public void setLatitude(double latitude)
	{
		mlatitude = latitude;
	}
	
	public void setSpeed(float speed)
	{
		mspeed = speed;
	}
	
	public void setDirection(float direction)
	{
		mdirection = direction;
	}
	
	public void setAltitude(float altitude)
	{
		maltitude = altitude;
	}
	
	public void setSatnum(int satnum)
	{
		msatnum = satnum;
	}
	
	public void setAlarmType(int alarmstatus)
	{
		malarmstatus = alarmstatus;
	}
}
