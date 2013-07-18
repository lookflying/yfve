#include "message.h"
#include <cstring>
#include <string>
#include <map>
#include <cstdio>
using namespace std;

/*终端手机号*/
MSG_BCD msg_g_phone_num[MSG_PHONE_NUM_LEN];
/*消息流水号*/
MSG_WORD msg_g_msg_seq = 0;
/*单包最大数据尺寸*/
unsigned int msg_g_max_pack_size = 0;

map<MSG_WORD, map<MSG_WORD, msg_message_t> > msg_g_unpack_cache;
map<MSG_WORD, MSG_WORD> msg_g_unpack_count;
map<MSG_WORD, unsigned int> msg_g_unpack_size;
map<MSG_WORD, MSG_WORD>	msg_g_unpack_id;

void clear_msg(msg_message_t &msg){
	if (msg.body.length > 0){
		delete[] msg.body.content;
	}
	msg.body.content = NULL;
	msg.body.length = 0;
}

void clear_serialized_msg(msg_serialized_message_t &serialized){
	if (serialized.length > 0){
		delete[] serialized.data;
	}
	serialized.data = NULL;
	serialized.length = 0;
}



void clear_unpack_cache(const MSG_WORD msg_seq){
	map<MSG_WORD, map<MSG_WORD, msg_message_t> >::iterator it;
	map<MSG_WORD, msg_message_t>::iterator msg_it;
	if ((it = msg_g_unpack_cache.find(msg_seq)) != msg_g_unpack_cache.end()){
		for (msg_it = it->second.begin(); msg_it != it->second.end(); ++msg_it){
			clear_msg(msg_it->second);
		}
		it->second.clear();
		msg_g_unpack_count[msg_seq] = 0;
		msg_g_unpack_size[msg_seq] = 0;
		msg_g_unpack_id[msg_seq] = 0;
	}
	msg_g_unpack_cache.erase(it);
}



string big_endian(MSG_WORD word){
	string str = "";
	str += (char)HBYTE(word);
	str += (char)LBYTE(word);
	return str;
}

string big_endian(MSG_DWORD dword){
	string str = "";
	str += big_endian(HWORD(dword));
	str += big_endian(LWORD(dword));
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
MSG_BYTE checksum(string str){
	MSG_BYTE chksum = 0x0000;	
	for (string::iterator it = str.begin(); it != str.end(); ++it){
		chksum = chksum ^ ((MSG_BYTE)*it);
	}
	return chksum;
}
/*根据header和body内容生成校验码, 并序列化*/
bool serialize(const msg_message_t &message, msg_serialized_message_t &serialized){
	string serialize_buf = "";
	string escaped_serialize_buf = "";
	serialize_buf += header2string(message.header);
	serialize_buf += body2string(message.body);
	MSG_BYTE chksum = checksum(serialize_buf);
	serialize_buf += (char)chksum;
	escape(serialize_buf, escaped_serialize_buf);
	escaped_serialize_buf.insert(escaped_serialize_buf.begin(), (char)MSG_FLAG);
	escaped_serialize_buf += (char)MSG_FLAG;
	unsigned int size = escaped_serialize_buf.size();
	serialized.data = new (nothrow) MSG_BYTE[size];
	if (serialized.data == NULL){
		return false;
	}
	memcpy((char*)serialized.data, escaped_serialize_buf.c_str(), size);
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


bool deserialize_header(string &str, msg_header_t &header){
	if (str.size() < MSG_HEADER_MIN_SIZE){
		return false;
	}else{
		MSG_SET_WORD(header.id, (MSG_BYTE)str.at(0), (MSG_BYTE)str.at(1));
		MSG_SET_WORD(header.property, (MSG_BYTE)str.at(2), (MSG_BYTE)str.at(3));
		for (unsigned int i = 0; i < MSG_PHONE_NUM_LEN; ++i){
			header.phone_num[i] = (MSG_BCD)str.at(4+i);
		}	
		MSG_SET_WORD(header.seq, (MSG_BYTE)str.at(10), (MSG_BYTE)str.at(11));
		str.erase(0, 12);
		if (MSG_IS_DIVIDED(header.property)){
			if (str.size() >= MSG_HEADER_MAX_SIZE - MSG_HEADER_MIN_SIZE){
				MSG_SET_WORD(header.pack_opt.pack_count, (MSG_BYTE)str.at(0), (MSG_BYTE)str.at(1));
				MSG_SET_WORD(header.pack_opt.pack_seq, (MSG_BYTE)str.at(2), (MSG_BYTE)str.at(3));
				str.erase(0, 4);
			}else{
				return false;
			}
		}
	}
	return true;
}
bool deserialize(const msg_serialized_message_t &serialized, msg_message_t &message){
	string deserialize_buf = "";
	string unescaped_deserialize_buf = "";
	if ((char)MSG_FLAG == (char)(*serialized.data) && (char)MSG_FLAG == (char)(*(serialized.data + serialized.length - 1))){
		deserialize_buf.append((char*)(serialized.data + 1), serialized.length - 2);
		unescape(deserialize_buf, unescaped_deserialize_buf);
		MSG_BYTE cal_chksum = checksum(unescaped_deserialize_buf);
		if (cal_chksum != 0x00)
						return false;
		msg_header_t temp_header;
		if (deserialize_header(unescaped_deserialize_buf, temp_header)){
			memcpy(&message.header, &temp_header, sizeof(msg_header_t));
			unsigned int msg_len = MSG_LENGTH(message.header.property);
			if(unescaped_deserialize_buf.size() != msg_len + 1){/*校验码占一个字节*/
				return false;
			}else{
				message.body.content = new (nothrow) MSG_BYTE[msg_len];
				if ( message.body.content == NULL){
					return false;
				}else{
					memcpy(&message.body.content[0], unescaped_deserialize_buf.c_str(), msg_len);
					message.body.length = msg_len;
					return true;
				}
			}
		}else{
			return false;
		}
	}else{
		return false;
	}
}

void bytes2phone_num(MSG_BYTE *bytes, unsigned int len, MSG_BCD *phone_num){
	unsigned int pre_zero = 0;
	if (len < 2 * MSG_PHONE_NUM_LEN){
		pre_zero = 2 * MSG_PHONE_NUM_LEN - len;
	}
	for (unsigned int i = 0; i < 2 * MSG_PHONE_NUM_LEN; ++i){
		MSG_BYTE cur_byte, buf_bytes[2];
		if (i < pre_zero){
			cur_byte = 0;
		}else{
			cur_byte = *(bytes + (i - pre_zero));
		}
		buf_bytes[i & 0x0001] = cur_byte;
		if (1 == (i & 0x0001)){
			MSG_BYTE2BCD(buf_bytes[0], buf_bytes[1], *(phone_num + (i / 2)));
		}
	}
}

msg_header_t generate_header(MSG_WORD id, MSG_WORD property, MSG_BCD *phone_num, MSG_WORD seq, msg_pack_opt_t pack_opt){
	msg_header_t header;
	header.id = id;
	header.property = property;
	memcpy(&header.phone_num[0], &phone_num[0], MSG_PHONE_NUM_LEN);
	header.seq = seq;
	header.pack_opt = pack_opt;
	return header;
}

msg_header_t generate_header(MSG_WORD id, MSG_WORD property, MSG_BCD *phone_num, MSG_WORD seq){
	msg_header_t header;
	header.id = id;
	header.property = property;
	memcpy(&header.phone_num[0], &phone_num[0], MSG_PHONE_NUM_LEN);
	header.seq = seq;
	memset(&header.pack_opt, 0, sizeof(msg_pack_opt_t));
	return header;
}


msg_pack_opt_t generate_pack_option(MSG_WORD pack_count, MSG_WORD pack_seq){
	msg_pack_opt_t pack_opt;
	pack_opt.pack_count = pack_count;
	pack_opt.pack_seq = pack_seq;
	return pack_opt;
}


bool pack_msg(const MSG_WORD id, const char* msg_data, const unsigned char encrypt, const unsigned int msg_len, vector<msg_serialized_message_t> &packed, MSG_WORD &msg_seq){
	packed.resize(0);
	if ((MSG_MAX_PACK_SIZE == 0 || msg_len <= (unsigned int)MSG_MAX_PACK_SIZE) && msg_len <= (unsigned int)MSG_PACK_SIZE_LIMIT){
		MSG_WORD property = MSG_PACK_PROPERTY(false, encrypt, msg_len);
		msg_message_t temp_msg;
		msg_serialized_message_t serialized_msg;
		temp_msg.header = generate_header(id, property, msg_g_phone_num, msg_g_msg_seq); 
		temp_msg.body.content = (MSG_BYTE*) msg_data;
		temp_msg.body.length = msg_len;
		if (serialize(temp_msg, serialized_msg)){
			msg_seq = msg_g_msg_seq;
			msg_g_msg_seq++;			
			packed.push_back(serialized_msg);
			return true;
		}else{
			return false;
		}
	}else{
		unsigned int remain = msg_len;
		unsigned int divided_size = 0;
		if (MSG_MAX_PACK_SIZE == 0){
			divided_size = (unsigned int)MSG_PACK_SIZE_LIMIT;
		}else{
			divided_size = ((unsigned int)MSG_MAX_PACK_SIZE < (unsigned int)MSG_PACK_SIZE_LIMIT)? (unsigned int)MSG_MAX_PACK_SIZE:(unsigned int)MSG_PACK_SIZE_LIMIT;
		}
		unsigned int seq = 0;
		unsigned int count = (remain + divided_size - 1) / divided_size;

		while (remain > 0){
			MSG_WORD cur_size;
			if (remain > divided_size){
				cur_size = divided_size;
				remain -= divided_size;
			}else{
				cur_size = remain;
				remain = 0;
			}
			MSG_WORD property = MSG_PACK_PROPERTY(true, encrypt, cur_size);
			msg_message_t temp_msg;
			msg_serialized_message_t serialized_msg;
			temp_msg.header = generate_header(id, property, msg_g_phone_num, msg_g_msg_seq, generate_pack_option(count, seq + 1));
			temp_msg.body.content = (MSG_BYTE*) (msg_data + (seq * divided_size));
			temp_msg.body.length = cur_size;
			if (!serialize(temp_msg, serialized_msg)){
				for (vector<msg_serialized_message_t>::iterator it = packed.begin(); it != packed.end(); ++it){
					clear_serialized_msg(*it);
				}
				return false;
			}
			packed.push_back(serialized_msg);
			++seq;
		}
		msg_seq = msg_g_msg_seq;
		msg_g_msg_seq++;			
		return true;
	}	
}

bool unpack_msg(const msg_message_t &msg, MSG_WORD &msg_id, MSG_WORD &msg_seq, char** msg_data, unsigned int &msg_len){
	msg_len = 0;
	if (MSG_IS_DIVIDED(msg.header.property)){
		map<MSG_WORD, map<MSG_WORD, msg_message_t> >::iterator it;
		map<MSG_WORD, msg_message_t>::iterator msg_it;
		if ((it = msg_g_unpack_cache.find(msg.header.seq)) != msg_g_unpack_cache.end()){
						if (msg_g_unpack_id[msg.header.seq] != msg.header.id
								||msg_g_unpack_count[msg.header.seq] != msg.header.pack_opt.pack_count){
							clear_unpack_cache(msg.header.seq);							
							msg_g_unpack_id[msg.header.seq] = msg.header.id;
							msg_g_unpack_count[msg.header.seq] = msg.header.pack_opt.pack_count;
						}
		}else{
			msg_g_unpack_cache[msg.header.seq] = map<MSG_WORD, msg_message_t>();
			it = msg_g_unpack_cache.find(msg.header.seq);
			msg_g_unpack_id[msg.header.seq] = msg.header.id;
			msg_g_unpack_count[msg.header.seq] = msg.header.pack_opt.pack_count;
			msg_g_unpack_size[msg.header.seq] = 0;
		}
		it->second[msg.header.pack_opt.pack_seq] = msg;
		msg_g_unpack_size[msg.header.seq] += MSG_LENGTH(msg.header.property);
		if (it->second.size() == msg_g_unpack_count[msg.header.seq]){
			*msg_data = new (nothrow) char[msg_g_unpack_size[msg.header.seq]];
			if (msg_data == NULL)
				return false;
			char* ptr = *msg_data;
			for (MSG_WORD i = 0; i < msg_g_unpack_count[msg.header.seq]; ++i){
				unsigned int len = MSG_LENGTH(it->second[i].header.property);
				memcpy(ptr, it->second[i].body.content, len);
				ptr += len;
			}
			msg_len = msg_g_unpack_size[msg.header.seq];
			msg_id = msg_g_unpack_id[msg.header.seq];
			msg_seq = msg.header.seq;
			return true;
		}
	}else{
		msg_len = MSG_LENGTH(msg.header.property);
		msg_id = msg.header.id;
		msg_seq = msg.header.seq;
		*msg_data = new (nothrow) char[msg_len];
		if (*msg_data == NULL)
			return false;
		memcpy(*msg_data, msg.body.content, msg_len);
		return true;
	}
	return false;
}
