
#pragma once

#include <vector>
#include <string>

#include "hnn/othello/board_position.hpp"
#include "hnn/othello/board_state.hpp"

#include "evaluation.h"

class AIbase {
public:
	//���̋ǖʂ�0��ǂݕ]���l(+�Ŏ�ԑ����L��)
	virtual Evaluation evalate(const hnn::othello::BoardState& state)const = 0;
};//class AIbase {
