
#pragma once

#include "ai_base.h"

//���S�ǂ�AI

class AIPerfect final :public AIbase {
public:
	using BoardState = hnn::othello::BoardState;
private:
	Evaluation evalate(const BoardState &state) const override;
};//class AIPerfect
