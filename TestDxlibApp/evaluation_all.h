
#pragma once

#include <optional>
#include <vector>
#include <array>
#include "hnn/othello/board_position.hpp"
#include "evaluation.h"

//�]���l�S�}�X������

class EvaluationAll {
	using BoardPosition = hnn::othello::BoardPosition;
	struct OneEval :public std::optional<Evaluation> {
		std::vector<BoardPosition> way;		//�]���l�܂ł̎w����
	};//struct OneEval {
	std::array<OneEval, BoardPosition::Length * BoardPosition::Length> evals;
public:
	const std::optional<Evaluation>& get(BoardPosition pos)const {
		return evals[pos.val];
	}
	const std::vector<BoardPosition>& getWay(BoardPosition pos)const {
		return evals[pos.val].way;
	}
	std::optional<Evaluation>& get(BoardPosition pos) {
		return const_cast<std::optional<Evaluation>&>(std::as_const(*this).get(pos));
	}
	//�ő�l
	std::optional<Evaluation> getMaximum() const {
		auto maxPos = getMaximumPos();
		if (maxPos) {
			return evals[maxPos->val];
		}
		return std::nullopt;
	}
	//rand : �ő�l�𗐐��ł΂������ő�ψʕ�
	std::optional<BoardPosition> getMaximumPos(Evaluation rand = 0) const;
	void set(BoardPosition pos, Evaluation eval, const std::vector<BoardPosition>& way) {
		evals[pos.val].emplace(eval);
		evals[pos.val].way = way;
	}
};//class EvaluationAll {
