#include "message.h"
#include <cstring>
#include <string>
using namespace std;
static string big_endian(MSG_WORD word){
	string str = "";
	str += (char)HBYTE(word);
	str += (char)LBYTE(word);
	return str;
}

string header2string(const msg_header_t header){
	string str = "";
	str += big_endian(header.id);
	str += big_endian(header.property);
	for ( unsigned int i = 0; i < sizeof(header.phone_num)/sizeof(MSG_BCD); ++i){
		str += (char)header.phone_num[i];
	}
	str += big_endian(header.seq);
	if (MSG_IS_DIVIDED(header.property)){
		str += big_endian(header.pack_opt.pack_count);
		str += big_endian(header.pack_opt.pack_seq);
	}
	return str;
}

string body2string(msg_body_t body){
	string str = "";
	MSG_BYTE *ptr = body.content;
	for (unsigned int i = 0; i < body.length; ++i){
		str += (char)(*ptr++);
	}
	return str;
}

void escape(string str, string &escaped){
	escaped = "";
	for (string::iterator it = str.begin(); it != str.end(); ++it){
		if (*it == MSG_FLAG){
			escaped += (char)MSG_ESC;
			escaped += (char)MSG_FLAG_ESC;
		}else if (*it == MSG_ESC){
			escaped += (char)MSG_ESC;
			escaped += (char)MSG_ESC_ESC;
		}else{
			escaped += *it;
		}
	}
}

bool serialize(msg_message_t message, msg_serialized_message_t &serialized){
	string serialize_buf = "";
	string escaped_serialize_buf = "";
	MSG_BYTE chksum = 0x00;
	serialize_buf += header2string(message.header);
	serialize_buf += body2string(message.body);
	for (string::iterator it = serialize_buf.begin(); it != serialize_buf.end(); ++it){
		chksum = chksum ^ ((MSG_BYTE)*it);
	}
	serialize_buf += (char)chksum;
	escape(serialize_buf, escaped_serialize_buf);
	escaped_serialize_buf.insert(escaped_serialize_buf.begin(), (char)MSG_FLAG);
	escaped_serialize_buf += (char)MSG_FLAG;
	unsigned int size = escaped_serialize_buf.size();
	serialized.data = new (nothrow) MSG_BYTE[size];
	if (serialized.data == NULL){
		return false;
	}
	strncpy((char*)serialized.data, escaped_serialize_buf.c_str(), size);
	serialized.length = size;
	return true;
}

void unescape(string str, string &unescaped){
	unescaped = "";
	char last;
	if (str.size() >= 2){
		string::iterator it = str.begin();
		last = *it;
		++it;
		if (last != MSG_ESC){
			unescaped += last;
		}
		for (; it != str.end(); ++it){
			if (last == (char)MSG_ESC){
				if (*it == (char)MSG_ESC_ESC){
					unescaped += (char)MSG_ESC;
				}else if (*it == (char)MSG_FLAG_ESC){
					unescaped += (char)MSG_FLAG;
				}else{
					//TODO
				}
			}else if (*it == (char)MSG_ESC){
				/*do nothing*/
			}else{
				unescaped += *it;
			}
			last = *it;
		}
	}
}


bool deserialize_header(string str, msg_header_t &header){
	if (str.size() < MSG_HEADER_MIN_SIZE){
		return false;
	}else{
		MSG_SET_WORD(header.id, (MSG_BYTE)str.at(0), (MSG_BYTE)str.at(1));
		MSG_SET_WORD(header.property, (MSG_BYTE)str.at(2), (MSG_BYTE)str.at(3));
		for (unsigned int i = 0; i < MSG_PHONE_NUM_LEN; ++i){
			header.phone_num[i] = (MSG_BCD)str.at(4+i);
		}	
		MSG_SET_WORD(header.seq, (MSG_BYTE)str.at(10), (MSG_BYTE)str.at(11));
		if (MSG_IS_DIVIDED(header.property) && str.size() >= MSG_HEADER_MAX_SIZE){
			MSG_SET_WORD(header.pack_opt.pack_count, (MSG_BYTE)str.at(12), (MSG_BYTE)str.at(13));
			MSG_SET_WORD(header.pack_opt.pack_seq, (MSG_BYTE)str.at(14), (MSG_BYTE)str.at(15));
		}else{
			return false;
		}
	}
	return true;
}
bool deserialize(msg_serialized_message_t serialized, msg_message_t &message)
{
	string deserialize_buf = "";
	string unescaped_deserialize_buf = "";
	deserialize_buf.append((char*)serialized.data, serialized.length);
	unescape(deserialize_buf, unescaped_deserialize_buf);
	
}

msg_header_t generate_header(MSG_WORD id, MSG_WORD property, MSG_BCD *phone_num, MSG_WORD seq, msg_pack_opt_t pack_opt){
	msg_header_t header;
	header.id = id;
	header.property = property;
	memcpy(&header.phone_num[0], phone_num, MSG_PHONE_NUM_LEN);
	header.seq = seq;
	header.pack_opt = pack_opt;
	return header;
}


msg_pack_opt_t generate_pack_option(MSG_WORD pack_count, MSG_WORD pack_seq){
	msg_pack_opt_t pack_opt;
	pack_opt.pack_count = pack_count;
	pack_opt.pack_seq = pack_seq;
	return pack_opt;
}
