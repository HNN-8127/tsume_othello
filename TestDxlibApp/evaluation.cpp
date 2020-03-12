
#include "evaluation.h"

std::string Evaluation::getString() const {
	if (val == 0)return "0";
	if (val > 0) {
		return "+" + std::to_string(getInt());
	}
	else {
		//to_string��P���Ɏg����"-0"���\���ł��Ȃ��̂�abs���K�v
		return "-" + std::to_string(std::abs(getInt()));
	}
}
