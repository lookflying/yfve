package vehicle_CVS;

public class TMCStruct_DSP {
	int msgId;	//消息编号
	long tmcTime;	//路况时间
	String cityCode;	//城市代码
	String tmcContent;	//路况信息
	
	public int getMsgId()
	{
		return msgId;
	}
	
	public long getTmcTime()
	{
		return tmcTime;
	}
	
	public String getCityCode()
	{
		return cityCode;
	}
	
	public String getTmcContent()
	{
		return tmcContent;
	}
}
