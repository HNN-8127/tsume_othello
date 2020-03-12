
#include "make_board.h"
#include "evaluation.h"
#include "ai_edax.h"
#include "strategy_depth_search.h"

namespace make_board {

using namespace hnn::othello;

namespace {
AIEdax edax;
}

std::optional<std::string>calc(int nBlank, std::mt19937& rand) {
	assert(nBlank > 1);
	std::string kif;
	kif.reserve(120);
	//
	hnn::othello::BoardState b = {};
	for (int i = 0; i < 60 - nBlank; ++i) {
		Evaluation ev = Evaluation::max();
		hnn::othello::BoardPosition bp;
		for (int x = 0; x < 8; ++x) {
			for (int y = 0; y < 8; ++y) {
				auto res = b.put({ x,y });
				if (res) {
					auto ev0 = edax.evalate(*res);
					int randWidth = 500 - i * 10;
					if (randWidth < 1)randWidth = 1;
					ev0.set(ev0.getRawValue() + rand() % randWidth);
					if (ev0 < ev) {
						ev = ev0;
						bp = { x,y };
					}
				}
			}
		}
		//
		if (ev == Evaluation::max()) {
			//パスは除外
			return std::nullopt;
		}
		else {
			b = *b.put(bp);
			kif += bp.getName();
		}
	}
	if (b.pass()) {
		//最終手パス
		return std::nullopt;
	}
	return kif;
}

//盤面の候補手で勝てる数と負ける数を数える
struct GameResult {
	int nWin = 0, nDraw = 0, nLose = 0;
	hnn::othello::BoardPosition winPos;		//勝てる手がある場合、そのうちの一つ
};
//置ける場所とその勝敗
GameResult countWinLose(const hnn::othello::BoardState& bs0) {
	GameResult gr;
	Evaluation ev0 = Evaluation::min(), ev1 = Evaluation::min();
	for (int x = 0; x < 8; ++x) {
		for (int y = 0; y < 8; ++y) {
			auto res = bs0.put({ x,y });
			if (res) {
				auto evTmp0 = sab.execute(*res, &perfect, 60, false).eval;
				if (evTmp0.getInt() < 0) {
					++gr.nLose;
				}
				else if (evTmp0.getInt() > 0) {
					++gr.nWin;
				}
				else {
					++gr.nDraw;
				}
				if (evTmp0 > ev0) {
					ev0 = evTmp0;
					gr.winPos = { x,y };
				}

			}
		}
	}
	return gr;
}

//edax一手読み
hnn::othello::BoardPosition edax1(const hnn::othello::BoardState& bs0) {
	Evaluation  ev1 = Evaluation::min();
	hnn::othello::BoardPosition bp1;
	for (int x = 0; x < 8; ++x) {
		for (int y = 0; y < 8; ++y) {
			auto res = bs0.put({ x,y });
			if (res) {
				Evaluation evTmp1;
				bool passed = res->pass();
				evTmp1 = edax.evalate(*res);
				if (!passed) {
					evTmp1 = evTmp1.inverse();
				}
				if (evTmp1 > ev1) {
					ev1 = evTmp1;
					bp1 = { x,y };
				}
			}
		}
	}
	return bp1;
}

bool ok(const hnn::othello::BoardState& bs_) {
	//おける数が2以上
	auto res = countWinLose(bs_);
	//勝てるのは一個だけ
	if (!((res.nLose + res.nDraw) >= 1 && res.nWin == 1))return false;
	//edax一手が間違える
	if (edax1(bs_) == res.winPos)return false;
	//手番を入れ替えたとき、相手の手が一個なら(そこが争点で自明なことが多いのでダメ)
	int nn = 0;
	auto bsRv = bs_.changeTurn();
	for (int x = 0; x < 8; ++x) {
		for (int y = 0; y < 8; ++y) {
			auto res = bsRv.put({ x,y });
			if (res) {
				++nn;
			}
		}
	}
	if (nn == 1)return false;
	return true;
}

hnn::othello::BoardState kifToBoard(const std::string& kif) {
	std::string tmp = "A1";
	hnn::othello::BoardState b = {};
	for (int i = 0; i < kif.size(); i += 2) {
		hnn::othello::BoardPosition bp;
		tmp[0] = kif[i];
		tmp[1] = kif[i + 1];
		bp.set(tmp);
		b = *b.put(bp);
		b.pass();
	}
	return b;
}

}//namespace make_board {
