package vehicle_CVS;

import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.List;

public class YZ_VehicleTransit_CVS {
	static {
		System.loadLibrary("vehicle_CVS");
	}

	/**
	 * 登录（鉴权） 函数类型：同步函数
	 * 
	 * @param simcardnum
	 *            Sim卡对应的电话号码
	 * @param username
	 *            (用作鉴权码） 工号
	 * @param password
	 *            密码
	 * @param person
	 *            用户真实名称 （调用后取第一个下标：person[0]）
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public static native int yz_2_userlogin(String simcardnum, String username,
			String password, String[] person);

	/**
	 * 注销（断开连接） 函数类型：同步函数
	 * 
	 * @param simcardnum
	 *            Sim卡对应的电话号码
	 * @param username
	 *            工号
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public static native int yz_2_userlogout(String simcardnum, String username);

	/**
	 * GPS上传 函数类型：同步函数
	 * 
	 * @param simcardnum
	 *            Sim卡对应的电话号码
	 * @param currentlocation
	 *            当前的位置信息
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public static native int yz_3_gpsupload(String simcardnum,
			LocationStruct_CVS currentlocation);

	/**
	 * 车辆状态上传 函数类型：同步函数
	 * 
	 * @param simcardnum
	 *            （忽略） 卡对应的电话号码
	 * @param vehiclestatus
	 *            车辆状态， 0为空闲中，1为工作中，2为保养中
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误则返回最新的一个错误值
	 */
	public static native int yz_2_sendvehiclestatus(String simcardnum,
			int vehiclestatus);

	/**
	 * 根据经纬度获取天气预报 函数类型：同步函数
	 * 
	 * @param simcardnum
	 *            卡对应的电话号码（忽略）
	 * @param longtitude
	 *            经度
	 * @param latitude
	 *            纬度
	 * @param PredictDays
	 *            未来几天天气，最小两天，最多四天
	 * @param weatherStruct
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public static native int yz_3_getweather(String simcardnum,
			double longitude, double latitude, int PredictDays,
			WeatherStruct_DSP weatherStruct);

	/**
	 * 根据城市代码获取天气预报 函数类型：同步函数
	 * 
	 * @param simcardnum
	 *            卡对应的电话号码
	 * @param citycode
	 *            城市编号
	 * @param PredictDays
	 *            未来几天天气，默认两天，最多四天
	 * @param weatherinfo
	 * @return 0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public static native int yz_3_getweather(String simcardnum,
			String citycode, int PredictDays, WeatherStruct_DSP weatherinfo);

	/**
	 * 远程目的地设置 函数类型：异步函数
	 * 
	 * @param termialId
	 *            终端ID
	 * @param cityCode
	 *            城市代码
	 * @param megId
	 *            消息编号
	 * @return 获取结果，0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public static native int yz_3_gettmc(String termialId, String cityCode,
			int megId);

	/**
	 * 车况信息上传 函数类型：同步函数
	 * 
	 * @param simcardnum
	 *            卡对应的电话号码（忽略）
	 * @param vehicledata
	 *            车辆信息
	 * @return 获取结果，0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public static native int yz_3_sendvehicledata(String simcardnum,
			VehicleDataStruct_CVS vehicledata);

	/**
	 * 中间件初始化 函数类型：同步
	 * 
	 * @param terminalID
	 *            终端ID
	 * @param cvsIp
	 *            消息服务器IP
	 * @param cvsPort
	 *            消息服务器端口
	 * @param vehicleTransitListen
	 *            事件监听
	 */
	public static native void yz_2_init(String terminalID, String cvsIp,
			int cvsPort, VehicleTransitListen_DSP vehicleTransitListen);

	/**
	 * 初始化派发服务器必要的服务连接
	 * 
	 * @param terminalID
	 *            终端ID
	 * @param cvsIp
	 *            派发服务器IP
	 * @param mediaPort
	 *            派发服务器端口 9008
	 * @param wsIp
	 *            天气预报服务器IP
	 * @param wsPort
	 *            天气预报端口
	 * @param mediaFilePath
	 *            文件存储地址
	 * @param vehicleTransitListen
	 *            派发服务监听
	 */
	public static native void yz_2_init(String terminalID, String cvsIp,
			int mediaPort, String wsIp, int wsPort, String mediaFilePath,
			VehicleTransitListen_DSP vehicleTransitListen);

	/**
	 * 销毁服务连接
	 */
	public static native void yz_2_destroy();

	/**
	 * 修改文件存储地址 函数类型：同步函数
	 * 
	 * @param path
	 *            文件存储目录
	 * @return ？？？
	 */
	public static native int yz_3_modifyMediaPath(String path);

	/**
	 * 终端注册 函数类型：同步
	 * 
	 * @param simcardnum
	 *            手机号
	 * @param provinceId
	 *            省域编号
	 * @param cityId
	 *            市县域编号
	 * @param makerid
	 *            制造商ID
	 * @param terminalModel
	 *            终端型号
	 * @param terminalId
	 *            终端ID
	 * @param plateColor
	 *            车牌颜色
	 * @param plateNum
	 *            车牌号码
	 * @param authCode
	 *            注册成功返回得授权码
	 * @return 成功返回0，失败返回错误码
	 */
	public static int yz_2_register(String simcardnum, int provinceId,
			int cityId, String makerId, String terminalModel,
			String terminalId, int plateColor, String plateNum) {
		byte[] plateNumArray;
		try {
			plateNumArray = plateNum.getBytes("GBK");
		} catch (UnsupportedEncodingException e) {
			return 9997;
		}
		return do_register(simcardnum, provinceId, cityId, makerId,
				terminalModel, terminalId, plateColor, plateNumArray);

	}

	private static native int do_register(String simcardnum, int provinceId,
			int cityId, String makerId, String terminalModel,
			String terminalId, int plateColor, byte[] plateNum);

	/**
	 * 获取授权码， 须在鉴权后调用。
	 * 
	 * @return 返回授权码
	 */
	public static native String yz_2_getAuthCode();

	/**
	 * 终端注销，须在鉴权后调用
	 */
	public static native int yz_2_deregister();

	/**
	 * 辅助函数， 为解决findclass返回NULL的问题，在init前需要调用此函数，将回调函数中将会用到的java类传入
	 */
	public static void yz_2_prepare_class() {
		List<POIStruct_DSP> poiList = new ArrayList<POIStruct_DSP>();
		TMCStruct_DSP tmc_struct = new TMCStruct_DSP();
		WeatherStruct_DSP weather_struct = new WeatherStruct_DSP();
		prepare_class(poiList, tmc_struct, weather_struct);
	}

	/**
	 * prepare call back class
	 * 
	 * @param poilist
	 * @param tmc_struct
	 * @param weather_struct
	 */
	private static native void prepare_class(List<POIStruct_DSP> poilist,
			TMCStruct_DSP tmc_struct, WeatherStruct_DSP weather_struct);

}
