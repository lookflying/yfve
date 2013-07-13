#include <gtest/gtest.h>
#include "../message/message.h"
#include <string>
#include <cstring>
using namespace std;
TEST(macro_test, high_low_byte){
	MSG_WORD word = 0x1234;
	EXPECT_EQ(0x12, HBYTE(word));
	EXPECT_EQ(0x34, LBYTE(word));
	
}

extern void escape(string str, string& escaped);
extern  void unescape(string str, string &unescaped);
void print_hex(const char* array, unsigned int size){
	for (unsigned int i = 0; i < size; ++i){
		printf("%02x ", *(array + i));
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
int main(int argc, char** argv){
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
