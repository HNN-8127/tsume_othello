
#pragma once
#include <string>
#include <map>
#include <vector>
#include <chrono>

#include "hnn/string.hpp"
#include "hnn/archive.hpp"
#include "hnn/random.hpp"
#include "hnn/numerical_limits.hpp"
#include "hnn/dxlib_cout_dx.hpp"

#include "rating_table.h"
#include "time_second.h"
#include "rating_history.h"


using hnn::dxlib::cDx;
using hnn::dxlib::endlDx;
using hnn::dxlib::flushDx;

struct Problem {
	int nBlank, id;
	bool operator==(const Problem& other)const = default;
};

//���������̍ďo��̏��
struct ProblemInfo {
	Problem p;
	int nSucceedWin = 3;		//�A������
	long long nextTime;			//���o�莞��
	std::vector<char> history = { 0,0,0,0,0 };	//����(0:�����{ 1:�Z 2:�~),�E���ŋ�
	int nWin = 0, nLose = 0;
	//
	void setTime(long long baseTime) {
		nextTime = baseTime + (long long)hnn::random::getInt(1400, 2200) * hnn::pow(3, nSucceedWin);
	}
	bool cleared() {
		return nWin > 0;
	}
};

struct OneHistory {
	int win = 0, lose = 0;		//����/�s����(��������1��̂݃J�E���g)
	int nextID = 1;				//���ɏo�Ă�����ԍ�
	int nCleared = 0;			//��x�ł�����
	int nSolved = 0;			//�������␔(�������ŕ����J�E���g)
	int n() {
		return win + lose;
	}
	std::string str() {
		return hnn::string::sprintf("%d��/%d��(%d��%d�s:%.3f)", nSolved, n(), win, lose, n() != 0 ? (double)win / n() : 0.0);
	}
};

//�e���[�h(���Z�A���l)��̃f�[�^
struct SaveData {
	static inline const double initialRate = 900;
	static inline const double initialAppearRate = 900;
	double rate = initialAppearRate;
	int rateMax = (int)initialRate;			//�\�����[�g�̍ō�
	std::map<int, OneHistory>history;		//[�}�X]
	std::vector<ProblemInfo>problem;		//���ԁA���̗����̃L�[�Ō�������̂Ŏb���vec�Ŏ���
	int timeOffset = 0;
	int succeedWinStat = 0;		//�A���X�e�[�^�X(3=3�A���A-4=4�A�s)
	HistoryContainer<int> ratingHistory;
	//
	//���ݎ���(�b)
	long long getNowTime() {
		using namespace std::chrono;
		steady_clock::time_point begin = steady_clock::now();
		auto second = duration_cast<seconds>(begin.time_since_epoch());
		return second.count() + timeOffset;
	}
	//�S�}�X���v
	OneHistory sumHistory() {
		OneHistory res;
		for (auto& one : history) {
			res.win += one.second.win;
			res.lose += one.second.lose;
			res.nCleared += one.second.nCleared;
			res.nSolved += one.second.nSolved;
		}
		return res;
	}
	int getAppearRate() {
		return (int)(rate - (initialAppearRate - initialRate) * 20 / (20 + sumHistory().n()));
	}
	Problem nextImpl(int gameMode) {
		Problem p;
		int ratePlus = getAppearRate() + hnn::random::getInt(-300, -200)
			+ succeedWinStat * 75;		//���𗦁`70%�ɂ���ɂ�R�����Ⴂ�̂��o��
		for (auto& one : ratingTable[gameMode]) {
			if (ratePlus < one.second) {
				p.nBlank = std::clamp(one.first, 2, nBlankMax);
				p.id = history[p.nBlank].nextID;
				++history[p.nBlank].nextID;
				break;
			}
		}
		return p;
	}
	//���ɏo�肷�ׂ����(���O�Ɠ����̂͏o���Ȃ�)
	Problem next(const Problem& prev, int gameMode, bool doReview) {
		//���K����T��
		if (doReview) {
			long long minTime = hnn::MaxValue();
			auto minItr = problem.end();
			for (auto itr = problem.begin(); itr != problem.end(); ++itr) {
				if (itr->nextTime < minTime && !(itr->p == prev)) {
					minItr = itr;
					minTime = itr->nextTime;
				}
			}
			if (minItr != problem.end()) {
				if (minTime < getNowTime()) {
					if (hnn::random::getBool(0.8)) {
						//8���ŕ��K���
						return minItr->p;
					}
				}
			}
		}
		//�V�K���
		return nextImpl(gameMode);
	}
	//���K�҂����̐�
	int getNumberOfReLearning(const std::optional<Problem>& now) {
		int res = 0;
		auto nowTime = getNowTime();
		for (auto itr = problem.begin(); itr != problem.end(); ++itr) {
			if (itr->nextTime < nowTime) {
				if (!now || !(itr->p == *now)) {
					++res;
				}
			}
		}
		return res;
	}
	//���K�҂����̂���(now�ȊO��)�A�����ɏo�肳�ꂻ���Ȃ̂��擾
	//�o�肪�߂�����sort����ĕԂ�
	std::vector<Problem> getNextReLearning(const std::optional<Problem>& now) {
		int maxSize = 3;
		auto proSorted = problem;
		if (now) {
			for (int i = 0; i < proSorted.size(); ++i) {
				if (*now == proSorted[i].p) {
					proSorted.erase(proSorted.begin() + i);
					break;
				}
			}
		}
		std::sort(proSorted.begin(), proSorted.end(), [](const ProblemInfo& left, const ProblemInfo& right)->bool {
			return left.nextTime < right.nextTime;
			});
		if (maxSize < proSorted.size())proSorted.resize(maxSize);
		std::vector<Problem> res(proSorted.size());
		for (int i = 0; i < proSorted.size(); ++i) {
			res[i] = proSorted[i].p;
		}
		return res;
	}
	auto findProblem(const Problem& p) {
		for (auto itr = problem.begin(); itr != problem.end(); ++itr) {
			if (itr->p == p)return itr;
		}
		return problem.end();
	}
	//�񓚌��ʓo�^
	void registerResult(const Problem& p, bool win) {
		auto itr = findProblem(p);
		ProblemInfo pi;
		//�V�K���
		if (itr == problem.end()) {
			problem.emplace_back();
			itr = problem.end();
			--itr;
			pi.p = p;
		}
		else {
			pi = *itr;
		}
		//
		if (win) {
			pi.nSucceedWin++;
			if (!pi.cleared()) {
				history[p.nBlank].nCleared++;
			}
			pi.nWin++;
		}
		else {
			pi.nSucceedWin = 0;
		}
		pi.setTime(getNowTime());
		//
		timeOffset += 120;
		for (int i = 1; i < (int)pi.history.size(); ++i) {
			pi.history[i - 1] = pi.history[i];
		}
		pi.history.back() = (win ? (char)1 : (char)2);
		//
		*itr = pi;

	}
};

//�Ǝ��N���X�ɑ΂���V���A���C�Y��`(�e����hnn���O��Ԃɒ�`���邱��)
namespace hnn {
	template<class Arc>
	struct SerialFunc<Arc, Problem> {
		static void f(Arc& ar, Problem& p) {
			serialize(ar, p.nBlank);
			serialize(ar, p.id);
		}
	};//class SerialFunc {
	template<class Arc>
	struct SerialFunc<Arc, ProblemInfo> {
		static void f(Arc& ar, ProblemInfo& p) {
			serialize(ar, p.p);
			serialize(ar, p.nSucceedWin);
			serialize(ar, p.nWin);
			serialize(ar, p.nLose);
			serialize(ar, p.nextTime);
			serialize(ar, p.history);
		}
	};//class SerialFunc {
	template<class Arc>
	struct SerialFunc<Arc, OneHistory> {
		static void f(Arc& ar, OneHistory& oh) {
			serialize(ar, oh.win);
			serialize(ar, oh.lose);
			serialize(ar, oh.nCleared);
			serialize(ar, oh.nSolved);
			serialize(ar, oh.nextID);
		}
	};//class SerialFunc {
	//
	template<class Arc>
	struct SerialFunc<Arc, SaveData> {
		static void f(Arc& ar, SaveData& sd_) {
			serialize(ar, sd_.rate);
			serialize(ar, sd_.rateMax);
			serialize(ar, sd_.history);
			serialize(ar, sd_.problem);
			serialize(ar, sd_.timeOffset);
			serialize(ar, sd_.ratingHistory);
		}
	};//class SerialFunc {
}//namespace hnn {

