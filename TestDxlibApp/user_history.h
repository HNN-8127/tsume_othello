
#pragma once

#include <vector>
#include <unordered_map>
#include <optional>

#include "dxlib.h"

#include "hnn/date.hpp"

//芝生部分の実装

std::optional<hnn::Date> getDate() {
	hnn::Date res;
	DATEDATA time;
	if (GetDateTime(&time) != 0)return std::nullopt;
	hnn::Date date;
	date.y = time.Year;
	date.mo = time.Mon;
	date.d = time.Day;
	date.h = time.Hour;
	date.mi = time.Min;
	date.s = time.Sec;
	return date;
}

//freeモードのうち一個の空きマス数
struct UserHistory {
	std::vector<int>progress;	//過去28日の解答状況(dateの基準が[0])
	hnn::Date baseDate;
	//
	bool addProgress(int timeShift, int count = 1) {
		const int progressMaxSize = 28;
		auto tmp = getDate();
		if (!tmp)return false;
		auto date = *tmp;
		date.h -= 3;
		date.regularization();
		date.h = 0;
		date.mi = 0;
		date.s = 0;
		if (date < baseDate)return false;
		//
		if (progress.size() < progressMaxSize)progress.resize(progressMaxSize);
		if (date == baseDate) {
			progress[timeShift] += count;
			return true;
		}
		int shift = 0;
		for (; shift < progressMaxSize + 2; ++shift) {
			if (date < baseDate || date == baseDate) {
				break;
			}
			date.d--;
			date.regularization();
		}
		for (int i = progress.size() - 1; 0 <= i; --i) {
			int id = i - shift;
			if (0 <= id) {
				progress[i] = progress[id];
			}
			else {
				progress[i] = 0;
			}
		}
		progress[timeShift] += count;
		date.d += shift;
		date.regularization();
		baseDate = date;
		return true;
	}
};




//独自クラスに対するシリアライズ定義(各自でhnn名前空間に定義すること)
namespace hnn {
	template<class Arc>
	struct SerialFunc<Arc, UserHistory> {
		static void f(Arc& ar, UserHistory& uh) {
			serialize(ar, uh.progress);
			serialize(ar, uh.baseDate);
		}
	};//class SerialFunc {
	template<class Arc>
	struct SerialFunc<Arc, hnn::Date> {
		static void f(Arc& ar, hnn::Date& da) {
			serialize(ar, da.y);
			serialize(ar, da.mo);
			serialize(ar, da.d);
			serialize(ar, da.h);
			serialize(ar, da.mi);
			serialize(ar, da.s);
		}
	};//class SerialFunc {
}

