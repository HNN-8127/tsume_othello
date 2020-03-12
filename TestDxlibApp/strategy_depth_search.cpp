
#include "strategy_depth_search.h"

StrategyResult StrategyDepthSearch::execute(const BoardState & bs, const AIbase * ai_, int depth, const bool & killCalc) {
	StrategyResult res;
	BoardState state = bs;
	//1éËêiÇﬂÇÈ
	bool passFlag = false;
	while (true) {
		Evaluation maxEval = Evaluation::min();
		BoardPosition maxPos;
		for (int i = 0; i < BoardPosition::Length; ++i) {
			for (int j = 0; j < BoardPosition::Length; ++j) {
				BoardPosition pos = { i,j };
				if (auto child = state.put(pos); child) {
					auto eval = ai_->evalate(*child).inverse();
					if (maxEval < eval) {
						maxEval = eval;
						maxPos = pos;
					}
				}
			}
		}
		if (!(maxEval == Evaluation::min())) {
			state = *state.put(maxPos);
			passFlag = false;
			res.way.push_back(maxPos);
		}
		else {
			if (passFlag) {
				res.eval = *state.checkEndGame() * 100 * (bs.blackTurn == state.blackTurn ? -1 : 1);
				return res;
			}
			state.pass();
			passFlag = true;
		}
	}
}
