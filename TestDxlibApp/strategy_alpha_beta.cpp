
#include "strategy_alpha_beta.h"

StrategyResult StrategyAlphaBeta::execute(const BoardState& bs, const AIbase* ai_, int depth, const bool& killCalc) {
	ai = ai_;
	auto res = alphaBeta(bs, depth, minEvaluationValue, maxEvaluationValue, false, killCalc);
	res.eval = res.eval.inverse();
	std::vector<BoardPosition> wayNew(res.way.size());
	for (size_t i = 0; i < res.way.size(); ++i) {
		wayNew[i] = res.way[res.way.size() - 1 - i];
	}
	res.way = std::move(wayNew);
	return res;
}

StrategyResult StrategyAlphaBeta::alphaBeta(const BoardState& bs, int depth
	, Evaluation alpha, Evaluation beta, bool passFlag, const bool& killCalc) {
	if (killCalc) {
		return {};
	}
	//1手進める
	using TupleType = std::tuple<BoardPosition, BoardState, Evaluation>;
	std::vector<TupleType> children;
	children.reserve(60);
	for (int i = 0; i < BoardPosition::Length; ++i) {
		for (int j = 0; j < BoardPosition::Length; ++j) {
			BoardPosition pos = { i,j };
			if (auto child = bs.put(pos); child) {
				children.push_back({ pos,*child ,ai->evalate(*child) });
			}
		}
	}
	if (children.size() == 0) {
		if (passFlag) {
			//二連続passは終局
			return { ai->evalate(bs),{} };
		}
		BoardState bsNew = bs;
		const auto p = bsNew.pass();
		assert(p);
		//パスの場合
		auto res = alphaBeta(bsNew, depth, beta.inverse(), alpha.inverse(), true, killCalc);
		res.eval = res.eval.inverse();
		return res;
	}
	//評価値で並び替えておくことでabカットされる可能性を上げる
	std::sort(children.begin(), children.end(), [](const TupleType& left, const TupleType& right) {
		return std::get<2>(left) < std::get<2>(right);
		});
	if (depth <= 1) {
		return { std::get<2>(children[0]).inverse(),{std::get<0>(children[0])} };
	}
	std::vector<BoardPosition> way_;
	for (const auto& child : children) {
#if 0
		//前向き枝刈り(機能してないし完全読みの時パスがあるとバグるのの原因)
		if (std::get<2>(child).inverse() + getEstimatedError(depth, 0) < std::get<2>(children[0]).inverse()) {
			break;
		}
#endif
		if (alpha >= beta) {
			assert(way_.size() != 0);
			break;
		}
		auto res = alphaBeta(std::get<1>(child), depth - 1, beta.inverse(), alpha.inverse(), false, killCalc);
		res.eval = res.eval.inverse();
		if (alpha < res.eval) {
			alpha = res.eval;
			way_ = res.way;
			way_.push_back(std::get<0>(child));
		}
	}
	return { alpha, way_ };
}

Evaluation StrategyAlphaBeta::getEstimatedError(int depth, double simga) {
	double error;
	switch (depth) {
	case 0:
		error = 0;
		break;
	case 1:
		error = 5;
		break;
	case 2:
		error = 6;
		break;
	case 3:
	case 4:
		error = 7;
		break;
	case 5:
	case 6:
		error = 8;
		break;
	case 7:
	case 8:
		error = 9;
		break;
	default:
		error = 10;
	}
	if (depth % 2) {
		error += 5;
	}
	return Evaluation(error * simga * 100);
}
