package vehicle_CVS;
import java.util.List;


public class WeatherStruct_DSP {
	Long releaseTime;	//发布时间
	double longitude;	//经度
	double latitude;	//维度
	String cityCode;	//城市代码
	String cityName;	//城市名称
	String currCondition;	//当前天气
	int currConditionType;	//当前天气类型
	int currTemp_c;			//当前温度
	float currHumidity;		//当前湿度
	String currWindDirection;	//当前风向，如东风
	int currWindPower;			//当前风速
	List<WeatherIndex> WeatherIndexList;	//天气指数相关
	List<WeatherBrief> WeatherBriefList;	//未来几天天气
	String retain1;	//保留字段1
	String retain2;	//保留字段2
	String retain3;	//保留字段3
	String retain4; //保留字段4
	String retain5;	//保留字段5
	
}
