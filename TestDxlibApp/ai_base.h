
#pragma once

#include <vector>
#include <string>

#include "hnn/othello/board_position.hpp"
#include "hnn/othello/board_state.hpp"

#include "evaluation.h"

class AIbase {
public:
	//¡‚Ì‹Ç–Ê‚Ì0è“Ç‚İ•]‰¿’l(+‚Åè”Ô‘¤‚ª—L—˜)
	virtual Evaluation evalate(const hnn::othello::BoardState& state)const = 0;
};//class AIbase {
