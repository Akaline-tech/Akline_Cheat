#pragma once
#include <cstdint>

enum  CmdButtons : uint64_t {
	IN_ATTACK = (1<<0),
	IN_JUMP = (1<<1)
};

struct CInButtonState {
public:
	char pad[0X8];
	std::uint64_t nValue;
	std::uint64_t nValueChanged;
	std::uint64_t nValueScroll;
};

class CUserCMD {
public:
	char pad[0X58];
	CInButtonState nButtons;
	char pad_2[0X20];
};