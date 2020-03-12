
#pragma once

#include "hnn/othello/board_state.hpp"

#include "evaluation_all.h"
#include "ai_base.h"


struct StrategyResult {
	Evaluation eval;
	std::vector<hnn::othello::BoardPosition> way;
};

//����ǂ݂Ƃ��}����Ƃ�ab�@�Ƃ��̐�p�����A�ǖʕ]���͍s��Ȃ�
class StrategyBase {
public:
	using BoardState = hnn::othello::BoardState;
	//
	//depth:�[��(0�`)�@killCalc��true�ɂȂ����炷���ɓK���Ȓl��Ԃ��Ȃ���΂Ȃ�Ȃ�
	virtual StrategyResult execute(const BoardState& bs, const AIbase* ai, int depth, const bool& killCalc) = 0;
	//�ǂ߂�ő�̐[��
	virtual int getmaxDepth() {
		return 60;
	}
};//class StrategyBase {
