#include "ai_perfect.h"

Evaluation AIPerfect::evalate(const BoardState & state) const {
	if (auto winStone = state.checkEndGame(); winStone) {
		return Evaluation(*winStone * 100);
	}
	return Evaluation(-10000 * (state.blackTurn ? 1 : -1));
}
