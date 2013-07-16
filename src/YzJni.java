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
	
	static
	{
		System.loadLibrary("yz_comm");
	}
	
}
