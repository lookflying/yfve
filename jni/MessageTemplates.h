
#ifndef MESSAGE_TEMPLATES_H_
#define MESSAGE_TEMPLATES_H_

#include <string.h>
#include <arpa/inet.h>
#include <string>

#include "message/message.h"

enum {
	GENERAL_RESPONSE_SUCCEED = 0,
	GENREAL_RESPONSE_FAILED = 1,
	GENREAL_RESPONSE_MSGERROR = 2,
	GENREAL_RESPONSE_UNSUPPORTED = 3
};

class GeneralResponseMessage {
public:
	MSG_WORD serial;
	MSG_WORD id;
	MSG_BYTE result;

	GeneralResponseMessage()
		:serial(0), id(0), result(1)
	{}

	GeneralResponseMessage(const msg_body_t &msg)
		:serial(0), id(0), result(1)
	{
		this->parse(msg);
	}

	/**
	 * return 0 if everything ok
	 * otherwise -1 if length of msg is not correct
	 */
	int parse(const msg_body_t &msg)
	{
		if (msg.length != 5) {
			return -1;
		}
		MSG_SET_WORD(this->serial, msg.content[0], msg.content[1]);
		MSG_SET_WORD(this->serial, msg.content[2], msg.content[3]);
		this->result = msg.content[4];
		return 0;
	}
};

class TerminalRegisterMessage {
public:
	MSG_WORD provinceId;
	MSG_WORD cityId;
	MSG_BYTE manufactory[5];
	MSG_BYTE terminalModel[8];
	MSG_BYTE terminalId[7];
	MSG_BYTE color;
	std::string carPlate;

	char *toBytes() const {
		char *buf = new char[25 + carPlate.length()];

		buf[0] = HBYTE(provinceId);
		buf[1] = LBYTE(provinceId);
		buf[2] = HBYTE(cityId);
		buf[3] = LBYTE(cityId);
		memcpy(buf + 4, manufactory, sizeof(manufactory));
		memcpy(buf + 9, terminalModel, sizeof(terminalModel));
		memcpy(buf + 17, terminalId, sizeof(terminalId));
		buf[24] = color;
		memcpy(buf + 25, carPlate.c_str(), carPlate.length());

		return buf;
	}

	size_t len()const {
		return 25 + carPlate.length();
	}
};

class TerminalRegisterResponseMessage {
public:
	MSG_WORD serial;
	MSG_BYTE result;
	std::string authorizationCode;

	int parse(const msg_body_t &msg) {
		if (msg.length < 3) {
			return -1;
		}
		MSG_SET_WORD(this->serial, msg.content[0], msg.content[1]);
		this->result = msg.content[2];
		authorizationCode.assign(msg.content + 3, msg.content + msg.length);
		return 0;
	}
};

#endif // MESSAGE_TEMPLATES_H_









































