
#pragma once

//NŽè“Ç‚ÝAI

#include "strategy_base.h"


class StrategyAlphaBeta  final :public StrategyBase {
	const AIbase* ai = nullptr;
	StrategyResult alphaBeta(const BoardState &bs, int depth, Evaluation alpha, Evaluation beta, bool passFlag, const bool &killCalc);
	static Evaluation getEstimatedError(int depth, double simga);
public:
	using BoardPosition = hnn::othello::BoardPosition;
	//
	StrategyResult execute(const BoardState &bs, const AIbase* ai_, int depth, const bool &killCalc) override;
};//class StrategyAlphaBeta {


