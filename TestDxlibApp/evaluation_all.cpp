
#include "hnn/random.hpp"

#include "evaluation_all.h"

using BoardPosition = hnn::othello::BoardPosition;

std::optional<BoardPosition> EvaluationAll::getMaximumPos(Evaluation rand) const {
	std::optional<BoardPosition> res;
	std::optional<Evaluation> eval;
	for (int i = 0; i < BoardPosition::Length; ++i) {
		for (int j = 0; j < BoardPosition::Length; ++j) {
			BoardPosition pos = { i,j };
			int val = hnn::random::getInt(0, rand.getRawValue() + 1);
			auto oneEval = evals[pos.val];
			if (oneEval) {
				//—”‚ð‘«‚·
				*oneEval = Evaluation(val + oneEval->getRawValue());
				if (!eval || *eval < *oneEval) {
					eval = *oneEval;
					res = pos;
				}
			}
		}
	}
	return res;
}
