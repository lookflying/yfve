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
	
	/**
	 * 获取事实交通信息
	 * 函数类型：回调函数
	 * @param tmcSturt
	 */
	public native void yz_3_TMCcallback(TMCStruct_DSP tmcSturt);
	
	/**
	 * 车况信息上传
	 * 函数类型：同步函数
	 * @param simcardnum	卡对应的电话号码
	 * @param vehicledata	车辆信息
	 * @return 获取结果，0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public native int yz_3_sendvehicledata(String simcardnum, VehicleDataStruct_CVS vehicledata);
	
	
	/**
	 * 中间件初始化
	 * 函数类型：同步
	 * @param terminalID 终端ID
	 * @param cvsIp	消息服务器IP
	 * @param cvsPort 消息服务器端口	9001
	 */
	public native void yz_2_init(String terminalID, String cvsIp, int cvsPort);
	
	/**
	 * 初始化派发服务器必要的服务连接
	 * @param terminalID	终端ID
	 * @param cvsIp	派发服务器IP
	 * @param mediaPort	派发服务器端口 9008
	 * @param wsIp	天气预报服务器IP
	 * @param wsPort 天气预报端口
	 * @param mediaFilePath 文件存储地址
	 * @param vehicleTransitListen 派发服务监听
	 */
	public native void yz_2_init(String terminalID, String cvsIp, int mediaPort, String wsIp, int wsPort,  String mediaFilePath, VehicleTransitListen_DSP vehicleTransitListen);
	
	/**
	 * 销毁服务连接
	 */
	public native void yz_2_destory();
	
	/**
	 * 销毁服务连接（拼写正确版）
	 */
	public native void yz_2_destroy();
	
	/**
	 * 用户登陆状态
	 * 回调函数
	 * @param terminalID 终端ID
	 * @param userId 用户ID
	 * @param userName 用户名称
	 * @param state 用户状态 0在线 1异常退出 2正常注销
	 */
	public native void yz_3_loginstatecallback(String terminalID, long userId, String userName, int state);
	
	/**
	 * 修改文件存储地址
	 * 函数类型：同步函数
	 * @param path 文件存储目录 
	 * @return ？？？
	 */
	public native int yz_3_modifyMediaPath(String path);
	
	
	
	static
	{
		System.loadLibrary("yz_comm");
	}
	
}
