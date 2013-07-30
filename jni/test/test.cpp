#include <gtest/gtest.h>
#include "../message/message.h"
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>
using namespace std;
TEST(macro_test, high_low_byte){
	MSG_WORD word = 0x1234;
	EXPECT_EQ(0x12, HBYTE(word));
	EXPECT_EQ(0x34, LBYTE(word));

}

TEST(macro_test, high_low_word){
	MSG_DWORD dword = 0x12345678;
	EXPECT_EQ(0x1234, HWORD(dword));
	EXPECT_EQ(0x5678, LWORD(dword));
	EXPECT_EQ(0x12, HBYTE(HWORD(dword)));
	EXPECT_EQ(0x34, LBYTE(HWORD(dword)));
	EXPECT_EQ(0x56, HBYTE(LWORD(dword)));
	EXPECT_EQ(0x78, LBYTE(LWORD(dword)));
}

TEST(macro_test, big_endian){
	MSG_DWORD dword = 0x87654321;
	MSG_WORD	word = 0x1234;
	string be_word = big_endian(word);
	EXPECT_EQ(0x12, be_word.c_str()[0]);
	EXPECT_EQ(0x34, be_word.c_str()[1]);
	string be_dword = big_endian(dword);
	EXPECT_EQ((char)0x87, be_dword.c_str()[0]);
	EXPECT_EQ((char)0x65, be_dword.c_str()[1]);
	EXPECT_EQ((char)0x43, be_dword.c_str()[2]);
	EXPECT_EQ((char)0x21, be_dword.c_str()[3]);

}

extern void escape(string str, string& escaped);
extern  void unescape(string str, string &unescaped);
void print_hex(const char* array, unsigned int size){
	for (unsigned int i = 0; i < size; ++i){
		printf("%02x ", (unsigned char)*(array + i));
	}
	printf("\n");
}	

TEST(pack_test, escape_then_unescape){
	MSG_BYTE data[] = {0x30, 0x7e, 0x08, 0x7d, 0x55};
	MSG_BYTE escaped_data[] = {0x30, 0x7d, 0x02, 0x08, 0x7d, 0x01, 0x55};
	string buf = "";
	string escaped_buf = "";
	string unescaped_buf = "";
	string right = "";
	buf.append((char*)&data[0], sizeof(data)/sizeof(MSG_BYTE));
	right.append((char*)&escaped_data[0], sizeof(escaped_data)/sizeof(MSG_BYTE));
	escape(buf, escaped_buf);
	print_hex(buf.c_str(), buf.size());
	print_hex(escaped_buf.c_str(), escaped_buf.size());
	EXPECT_EQ(right.size(), escaped_buf.size());
	EXPECT_EQ(0, strncmp(escaped_buf.c_str(), right.c_str(), right.size()));
	unescape(escaped_buf, unescaped_buf);
	print_hex(unescaped_buf.c_str(), unescaped_buf.size());
	EXPECT_EQ(buf.size(), unescaped_buf.size());
	EXPECT_EQ(0, strncmp(unescaped_buf.c_str(), buf.c_str(), buf.size()));
}

TEST(pack_test, random_escape){
	srand(time(NULL));
	unsigned int size = rand() % (rand() % 100000 + 2000000);
	char * array = new (nothrow) char[size];
	ASSERT_NE(array, (char*)NULL);
	for (unsigned int i = 0; i < size; ++i){
		char c = (char)rand() % 256;
		array[i] = c;
	}
	string buf = "";
	buf.append(array, size);
	string escaped_buf = "";
	string unescaped_buf = "";
	string right = "";
	escape(buf, escaped_buf);
	//print_hex(buf.c_str(), buf.size());
	//print_hex(escaped_buf.c_str(), escaped_buf.size());
	unescape(escaped_buf, unescaped_buf);
	EXPECT_EQ(buf.size(), unescaped_buf.size());
	EXPECT_EQ(0, strncmp(unescaped_buf.c_str(), buf.c_str(), buf.size()));
}

extern string header2string(const msg_header_t header);

extern bool deserialize_header(string &str, msg_header_t &header);
TEST(pack_test, serialize_header){
	MSG_BYTE phone_num[] = {1,2,3,4,5,6,7,8,9,0,};
	MSG_BCD real_phone_num[MSG_PHONE_NUM_LEN];
	bytes2phone_num(&phone_num[0], 10, &real_phone_num[0]);

	msg_header_t header = generate_header(0x0001, MSG_PACK_PROPERTY(true, 0x07, 128), &real_phone_num[0], 2, generate_pack_option(2, 1));
	string str = header2string(header);
	EXPECT_EQ(16, str.size());
	msg_header_t deserialized; 
	EXPECT_EQ(true, deserialize_header(str, deserialized));

	EXPECT_EQ(0, strncmp((char*)&header, (char*)&deserialized, sizeof(msg_header_t)));
	header = generate_header(0x01, MSG_PACK_PROPERTY(false, 0x08, 64), &real_phone_num[0], 3, generate_pack_option(2, 1));
	str = header2string(header);
	EXPECT_EQ(12, str.size());
	EXPECT_EQ(true, deserialize_header(str, deserialized));
	EXPECT_EQ(0, strncmp((char*)&header, (char*)&deserialized, sizeof(msg_header_t) - sizeof(msg_pack_opt_t)));
	print_hex((char*)&real_phone_num[0], MSG_PHONE_NUM_LEN);
	print_hex((char*)&header.phone_num[0], MSG_PHONE_NUM_LEN);
	print_hex((char*)&deserialized.phone_num[0], MSG_PHONE_NUM_LEN);
}

TEST(pack_test, small_sample){
	char data[] = {0x30, 0x7e, 0x08, 0x7d, 0x55};
	vector<msg_serialized_message_t> sample;
	MSG_WORD msg_seq;
	msg_message_t message;
	char* unpacked;
	unsigned int len;
	MSG_WORD unpacked_id;
	MSG_WORD unpacked_seq;
	print_hex(data, sizeof(data));
	EXPECT_TRUE(pack_msg(0x0001, &data[0], 0x0000, sizeof(data), sample, msg_seq));
	EXPECT_TRUE(deserialize(sample[0], message));
	EXPECT_TRUE(unpack_msg(message, unpacked_id, unpacked_seq, &unpacked, len));
	EXPECT_EQ(0x0001, unpacked_id);
	EXPECT_EQ(msg_seq, unpacked_seq);
	EXPECT_EQ(sizeof(data), len);
	print_hex(unpacked, sizeof(data));
	EXPECT_EQ(0, strncmp(&data[0], unpacked, len));
}

TEST(pack_test, pack_unpack_message){
	srand(time(NULL));
	unsigned int small_size = (unsigned int)rand() % (unsigned int)0x3ff;
	unsigned int large_size = (unsigned int)rand() % (0x3ff * 100)+ (unsigned int)0x400;
	char* small_msg = new (nothrow) char[small_size];
	ASSERT_TRUE(small_msg != NULL);
	char* large_msg = new (nothrow) char[large_size];
	ASSERT_TRUE(large_msg != NULL);
	vector<msg_serialized_message_t> small, large;
	MSG_WORD small_msg_seq, large_msg_seq;
	set_global_max_pack_size(0);
	EXPECT_TRUE(pack_msg(0x0001, small_msg, 0x0000, small_size, small, small_msg_seq));
	EXPECT_TRUE(pack_msg(0x0002, large_msg, 0x0007, large_size, large, large_msg_seq));
	EXPECT_EQ((small_size + 0x3ff - 1) / 0x3ff, small.size());
	EXPECT_EQ((large_size + 0x3ff - 1) / 0x3ff, large.size());
	char * unpacked;
	unsigned int len;
	MSG_WORD unpacked_id;
	MSG_WORD unpacked_seq;
	msg_message_t message;
	EXPECT_TRUE(deserialize(small[0], message));
	EXPECT_TRUE(unpack_msg(message, unpacked_id, unpacked_seq, &unpacked, len));
	EXPECT_EQ(0x0001, unpacked_id);
	EXPECT_EQ(small_msg_seq, unpacked_seq);
	for (vector<msg_serialized_message_t>::reverse_iterator it = large.rbegin();
			it != large.rend();
			++it){
		EXPECT_TRUE(deserialize(*it, message));
		if (unpack_msg(message, unpacked_id, unpacked_seq, &unpacked, len)){
			break;
		}
	}
	EXPECT_EQ(large_size, len);
	EXPECT_EQ(0, strncmp(large_msg, unpacked, large_size));
	EXPECT_EQ(0x0002, unpacked_id);
	EXPECT_EQ(large_msg_seq, unpacked_seq);
}

TEST(jni_test, string2num){
	string a = "0123456";
	MSG_BYTE* ptr;
	string2num(a, &ptr);
	for (unsigned int i = 0; i < a.size(); ++i){
		EXPECT_EQ(i, ptr[i]);
	}

}
/*
TEST(jni_test, string_char_converter){
	char* cptr = new (nothrow) char[(int)'z' - (int)'A' + 1];
	ASSERT_FLASE(cptr == NULL);
	char* ptr = cptr;
	for (char c = 'A'; c < 'z'; ++c){
		*ptr++ = c;
	}
	*ptr = (char)0;
	jstring jstr = string2jstring(
}
*/

int main(int argc, char** argv){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
