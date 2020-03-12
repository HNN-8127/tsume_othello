#pragma once

//1�肸�Ō�܂œǂ�AI

#include "strategy_base.h"


class StrategyDepthSearch  final :public StrategyBase {
public:
	using BoardPosition = hnn::othello::BoardPosition;
	StrategyResult execute(const BoardState &bs, const AIbase* ai_, int depth, const bool &killCalc) override;
};//class StrategyDepthSearch {

