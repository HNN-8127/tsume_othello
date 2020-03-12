
#include "evaluation.h"

std::string Evaluation::getString() const {
	if (val == 0)return "0";
	if (val > 0) {
		return "+" + std::to_string(getInt());
	}
	else {
		//to_stringを単純に使うと"-0"が表示できないのでabsが必要
		return "-" + std::to_string(std::abs(getInt()));
	}
}
