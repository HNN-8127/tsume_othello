
#pragma once

#include <vector>
#include <string>

#include "hnn/othello/board_position.hpp"
#include "hnn/othello/board_state.hpp"

#include "evaluation.h"

class AIbase {
public:
	//今の局面の0手読み評価値(+で手番側が有利)
	virtual Evaluation evalate(const hnn::othello::BoardState& state)const = 0;
};//class AIbase {
