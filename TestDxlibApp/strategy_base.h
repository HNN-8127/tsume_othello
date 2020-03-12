
#pragma once

#include "hnn/othello/board_state.hpp"

#include "evaluation_all.h"
#include "ai_base.h"


struct StrategyResult {
	Evaluation eval;
	std::vector<hnn::othello::BoardPosition> way;
};

//‰½è“Ç‚İ‚Æ‚©}Š ‚è‚Æ‚©ab–@‚Æ‚©‚Ìíp•”•ªA‹Ç–Ê•]‰¿‚Ís‚í‚È‚¢
class StrategyBase {
public:
	using BoardState = hnn::othello::BoardState;
	//
	//depth:[‚³(0`)@killCalc‚ªtrue‚É‚È‚Á‚½‚ç‚·‚®‚É“K“–‚È’l‚ğ•Ô‚³‚È‚¯‚ê‚Î‚È‚ç‚È‚¢
	virtual StrategyResult execute(const BoardState& bs, const AIbase* ai, int depth, const bool& killCalc) = 0;
	//“Ç‚ß‚éÅ‘å‚Ì[‚³
	virtual int getmaxDepth() {
		return 60;
	}
};//class StrategyBase {
