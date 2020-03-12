
#pragma once

#include <optional>
#include <string>
#include <random>

#include "hnn/othello/core.hpp"

#include "ai_perfect.h"
#include "strategy_alpha_beta.h"



inline AIPerfect perfect;
inline StrategyAlphaBeta sab;

namespace make_board {
	//
	std::optional<std::string>calc(int nBlank, std::mt19937& rand);
	//
	bool ok(const hnn::othello::BoardState& bs_);
	//
	hnn::othello::BoardState kifToBoard(const std::string& kif);
}//namespace make_board {


