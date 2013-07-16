import java.util.List;

public class YzJni {
	/**
	 * 登录
	 * 函数类型：同步函数
	 * @param simcardnum Sim卡对应的电话号码
	 * @param username 工号
	 * @param password 密码
	 * @param person 用户真实名称 （调用后取第一个下标：person[0]）
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native int yz_2_userlogin(String simcardnum, String username,
			String password, String[] person);
	
	/**
	 * 注销
	 * 函数类型：同步函数
	 * @param simcardnum Sim卡对应的电话号码
	 * @param username 工号
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native int yz_2_userlogout(String simcardnum, String username);
	
	/**
	 * GPS上传
	 * 函数类型：同步函数
	 * @param simcardnum Sim卡对应的电话号码
	 * @param currentlocation 当前的位置信息
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native int yz_3_gpsupload(String simcardnum, LocationSturt_CVS currentlocation);
	
	/**
	 * 车辆状态上传
	 * 函数类型：同步函数
	 * @param simcardnum 卡对应的电话号码
	 * @param vehiclestatus 车辆状态， 0为空闲中，1为工作中，2为保养中
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误则返回最新的一个错误值
	 */
	public native int yz_2_sendvehiclestatus(String simcardnum, int vehiclestatus);
	
	/**
	 * 根据经纬度获取天气预报
	 * 函数类型：同步函数
	 * @param simcardnum	卡对应的电话号码
	 * @param longtitude	经度
	 * @param latitude		纬度
	 * @param PredictDays	未来几天天气，最小两天，最多四天
	 * @param weatherStruct	
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native int yz_3_getweather(String simcardnum, double longtitude, double latitude, int PredictDays, WeatherSturt_DSP weatherStruct);
	
	/**
	 * 服务器主推天气信息
	 * 函数类型：异步函数
	 * @param weatherSturt
	 */
	public native void yz_3_weathercallback(WeatherSturt_DSP weatherSturt);
	
	/**
	 * 根据城市代码获取天气预报
	 * 函数类型：同步函数
	 * @param simcardnum 卡对应的电话号码
	 * @param citycode 城市编号
	 * @param PredictDays 未来几天天气，默认两天，最多四天
	 * @param weatherinfo
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native int yz_3_getweather(String simcardnum, String citycode, int PredictDays, WeatherSturt_DSP weatherinfo);
	
	/**
	 * 远程目的地设置
	 * 函数类型：回调函数
	 * @param poinum 发送的poi个数，暂定最多5个
	 * @param poilist poi信息列表 
	 * @param result 获取结果，0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native void yz_3_remotedescallback(int poinum, List<POIStruct_DSP> poilist, int result);
	
	/**
	 * 远程目的地设置
	 * 函数类型：异步函数
	 * @param termialId 终端ID
	 * @param cityCode 城市代码
	 * @param megId 消息编号
	 * @return 获取结果，0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native int yz_3_gettmc(String termialId, String cityCode, int megId);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	static
	{
		System.loadLibrary("yz_comm");
	}
	
}
