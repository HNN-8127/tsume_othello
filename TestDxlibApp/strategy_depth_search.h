#pragma once

//1手ずつ最後まで読むAI

#include "strategy_base.h"


class StrategyDepthSearch  final :public StrategyBase {
public:
	using BoardPosition = hnn::othello::BoardPosition;
	StrategyResult execute(const BoardState &bs, const AIbase* ai_, int depth, const bool &killCalc) override;
};//class StrategyDepthSearch {

