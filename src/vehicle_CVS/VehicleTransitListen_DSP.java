package vehicle_CVS;

import java.util.List;

public interface VehicleTransitListen_DSP {
	/**
	 * 远程目的地设置 函数类型：回调函数
	 * 
	 * @param poinum
	 *            发送的poi个数，暂定最多5个
	 * @param poilist
	 *            poi信息列表
	 * @param result
	 *            获取结果，0为成功，非0为失败，根据具体值对应错误类型，如果有多个错误，则返回最新的一个错误值
	 */
	public void yz_3_remotedescallback(int poinum, List<POIStruct_DSP> poilist, int result);
	/**
	 * 用户登陆状态 回调函数
	 * 
	 * @param terminalID
	 *            终端ID
	 * @param userId
	 *            用户ID
	 * @param userName
	 *            用户名称
	 * @param state
	 *            用户状态 0在线 1异常退出 2正常注销
	 */
	public void yz_3_loginstatecallback(String terminalID,
			long userId, String userName, int state);

	/**
	 * 获取事实交通信息 函数类型：回调函数
	 * 
	 * @param tmcStruct
	 */
	public void yz_3_TMCcallback(TMCStruct_DSP tmcStruct);
	

	/**
	 * 服务器主推天气信息 函数类型：异步函数
	 * 
	 * @param weatherSturt
	 */
	public void yz_3_weathercallback(
			WeatherStruct_DSP weatherStruct);
	
}
