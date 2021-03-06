
#pragma once

#include "hnn/othello/board_state.hpp"

#include "evaluation_all.h"
#include "ai_base.h"


struct StrategyResult {
	Evaluation eval;
	std::vector<hnn::othello::BoardPosition> way;
};

//何手読みとか枝刈りとかab法とかの戦術部分、局面評価は行わない
class StrategyBase {
public:
	using BoardState = hnn::othello::BoardState;
	//
	//depth:深さ(0〜)　killCalcがtrueになったらすぐに適当な値を返さなければならない
	virtual StrategyResult execute(const BoardState& bs, const AIbase* ai, int depth, const bool& killCalc) = 0;
	//読める最大の深さ
	virtual int getmaxDepth() {
		return 60;
	}
};//class StrategyBase {
