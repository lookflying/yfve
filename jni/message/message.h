#ifndef MESSAGE_H
#define MESSAGE_H
#include <stdint.h>
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

#define HBYTE(w) ((MSG_BYTE)(w>>8))
#define LBYTE(w) ((MSG_BYTE)(w&0xff))

bool serialize(msg_message_t message, msg_serialized_message_t &serialized);
bool deserialize(msg_serialized_message_t serialized, msg_message_t &message);

#define PACK_PROPERTY(divided, encrypt, length) \
	(0x00|(divided?0x2000:0x0000)|(encrypt&0x1c00)|(length&0x03ff))
#define MSG_LENGTH(msg_property) \
	(msg_property & 0x03ff)
#define MSG_IS_DIVIDED(msg_property) \
	(msg_property & 0x2000)
#define MSG_IS_ENCRYPT(msg_property) \
	(msg_property & 0x1c00)

msg_header_t generate_header(MSG_WORD id, MSG_WORD property, MSG_BCD *phone_num, MSG_WORD seq, msg_pack_opt_t pack_opt);

msg_pack_opt_t generate_pack_option(MSG_WORD pack_count, MSG_WORD pack_seq); 

#define MSG_SET_WORD(word, hbyte, lbyte) \
	((word) = (((((MSG_WORD)(hbyte))<<8)&0xff00) | (((MSG_WORD)(lbyte))&0x00ff)))

#endif
