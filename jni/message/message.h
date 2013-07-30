#ifndef MESSAGE_H
#define MESSAGE_H
#include <stdint.h>
#include <vector>
#include <string>
typedef uint8_t MSG_BYTE;
typedef uint16_t MSG_WORD;
typedef uint32_t MSG_DWORD;
typedef uint8_t MSG_BCD;

#define MSG_FLAG	(0x7e)
#define MSG_ESC	(0x7d)
#define MSG_FLAG_ESC (0x02)
#define MSG_ESC_ESC (0x01)

#define MSG_PACK_OPT_SIZE	(2*sizeof(MSG_WORD))
typedef struct msg_pack_opt{
	MSG_WORD pack_count;/*消息包总数*/
	MSG_WORD pack_seq;/*包序号，从1开始*/
} msg_pack_opt_t;

#define MSG_HEADER_MIN_SIZE (3 * sizeof(MSG_WORD)+6 * sizeof(MSG_BCD))
#define MSG_HEADER_MAX_SIZE (MSG_HEADER_MIN_SIZE + 2 * sizeof(MSG_WORD))
#define MSG_HEADER_SIZE (MSG_HEADER_MIN_SIZE)
#define MSG_PHONE_NUM_LEN	(6)
typedef struct msg_header{
	MSG_WORD id;/*消息id*/
	MSG_WORD property;/*消息体属性*/
	MSG_BCD phone_num[MSG_PHONE_NUM_LEN];/*手机号*/
	MSG_WORD seq;/*消息流水号*/
	msg_pack_opt_t pack_opt;/*消息包封装项*/
} msg_header_t;

typedef struct msg_body{
	MSG_BYTE *content;
	unsigned int length;
} msg_body_t;

typedef struct msg_message{
	msg_header_t header;
	msg_body_t body;
	MSG_BYTE chksum;/*校验码,从消息头开始,同后一字节异或直到校验码前一个字节,占用一个字节*/
} msg_message_t;

typedef struct msg_serialized_message{
	MSG_BYTE* data;/*封包后的消息内容*/
	unsigned int length;
} msg_serialized_message_t;

#define HBYTE(w) ((MSG_BYTE)((w)>>8))
#define LBYTE(w) ((MSG_BYTE)((w)&0xff))
#define HWORD(dw)	((MSG_WORD)((dw)>>16))
#define LWORD(dw)	((MSG_WORD)((dw)&0xffff))


#define MSG_PACK_PROPERTY(divided, encrypt, length) \
	((MSG_WORD)(0x00|(divided?0x2000:0x0000)|((encrypt<<10)&0x1c00)|(length&0x03ff)))
#define MSG_LENGTH(msg_property) \
	((unsigned int)(msg_property & 0x03ff))
#define MSG_IS_DIVIDED(msg_property) \
	(msg_property & 0x2000)
#define MSG_IS_ENCRYPT(msg_property) \
	(msg_property & 0x1c00)

#define MSG_BYTE2BCD(byte0, byte1, bcd) \
	((bcd) = ((MSG_BCD)((byte0) & 0x0f) | (MSG_BCD)(((byte1) & 0x0f)<<4)))


/*
 * 辅助函数，将字节流转换为MSG_BCD数组
 */
void bytes2phone_num(MSG_BYTE *bytes, unsigned int len, MSG_BCD *phone_num);

/*
 *辅助函数，生成分包的包头
 */
msg_header_t generate_header(MSG_WORD id, MSG_WORD property, MSG_BCD *phone_num, MSG_WORD seq, msg_pack_opt_t pack_opt);

/*
 *辅助函数，生成包头
 */
msg_header_t generate_header(MSG_WORD id, MSG_WORD property, MSG_BCD *phone_num, MSG_WORD seq);

/*
 *辅助函数，生成封装项字段
 */
msg_pack_opt_t generate_pack_option(MSG_WORD pack_count, MSG_WORD pack_seq); 


/*
 *宏，设置WORD的高低位字节
 */
#define MSG_SET_WORD(word, hbyte, lbyte) \
	((word) = (((((MSG_WORD)(hbyte))<<8)&0xff00) | (((MSG_WORD)(lbyte))&0x00ff)))

#define MSG_SET_DWORD(dword, hhbyte, hlbyte, lhbyte, llbyte) \
	((dword) = (((((MSG_DWORD)(hhbyte))<<24)&0xff000000) \
			  | ((((MSG_DWORD)(hlbyte))<<16)&0x00ff0000) \
			  |	((((MSG_DWORD)(lhbyte))<<8 )&0x0000ff00) \
			  |	((((MSG_DWORD)(llbyte))    ))))
/*
 *单包最大消息尺寸
 */
extern unsigned int msg_g_max_pack_size;
#define MSG_MAX_PACK_SIZE	(msg_g_max_pack_size) /*每个数据包中最大能够pack的内容的尺寸,以字节为单位,若为0则为无限制*/
#define MSG_PACK_SIZE_LIMIT	(0x03ff) /*包头可描述最大包尺寸*/

/*全局sim卡电话号码*/
extern MSG_BCD msg_g_phone_num[MSG_PHONE_NUM_LEN];
/*全局消息流水号，在pack_msg中维护*/
extern MSG_WORD msg_g_msg_seq;

/*
 * 获取消息的流水号
 */
#define MSG_SEQ(message) \
				(message.header.seq)
/*
 *序列化消息（一般无需额外调用，pack_msg会自行序列化）
 *message 待序列化的消息
 *serialized 序列化后的消息
 *返回值，序列化成功返回true，失败返回false
 */
bool serialize(const msg_message_t &message, msg_serialized_message_t &serialized);

/*
 *反序列化消息（主要对外接口）
 *serialized 待反序列化的消息
 *message 序列化后的消息（动态申请内存，使用后需清理）
 *返回值，反序列化成功返回true，失败返回false
 */
bool deserialize(const msg_serialized_message_t &serialized, msg_message_t &message);

/*
 *设置全局sim卡电话号码，接受char数组，每个字节表示一位号码，按字节序，长度不满12位，自动在前面补零，超过12位，只取前12位
 */
#define set_global_phone_num(phone_num,len) \
	(bytes2phone_num((phone_num), (len), &msg_g_phone_num[0]))
/*
 *设置全局单包最大尺寸，以字节位单位
 */
#define set_global_max_pack_size(size) \
	 (msg_g_max_pack_size = (unsigned int)(size))
/*
 *封装消息，自动分包（主要对外接口）
 *id 消息ID
 *msg_data 消息的内容
 *encrypt 消息的加密类型（取低3位，格式定义如消息体属性格式结构的10到12位
 *msg_len 消息的长度
 *packed 封装后的消息（动态申请内存，使用后需要清理）
 *msg_seq 封装后的消息对应的流水号
 *返回值，成功封装返回true，失败返回false
 * */
bool pack_msg(const MSG_WORD id, const char* msg_data, const unsigned char encrypt, const unsigned int msg_len, std::vector<msg_serialized_message_t> &packed, MSG_WORD &msg_seq);

/*
 *解包消息（主要对外接口）
 *利用全局缓存，每次接受一个反序列化后的消息，若与缓存中的消息组合能够成功解包，则返回true，并将消息内容拷贝至msg_data（动态申请内存，使用后需要清理）
 *msg 当前输入的消息
 *msg_id 解包成功后，获得的消息的消息ID
 *msg_seq 解包成功后，获得的消息的流水号
 *msg_data 指向消息数据的指针
 *len 消息的长度
 *返回值，成功解包返回true，尚未能够成功解包或者解包失败返回false
 */
bool unpack_msg(const msg_message_t &msg, MSG_WORD &msg_id, MSG_WORD &msg_seq, char** msg_data, unsigned int &len);

/*
 *清理消息体
 */
void clear_msg(msg_message_t &msg);
/*
 *清理序列化的消息体
 */
void clear_serialized_msg(msg_serialized_message_t &serialized);

/**
 * 将WORD转换为大端字节流
 */
std::string big_endian(MSG_WORD word);

/**
 * 将DWORD转换为大端字符流
 */
std::string big_endian(MSG_DWORD dword);

/*
 * 字符串转换为数字字节数组
 */
void string2num(std::string str, MSG_BYTE** numbers);


#endif
