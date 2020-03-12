
#pragma once

#include <list>
#include <limits>
#include <optional>

#include "hnn/math.hpp"
#include "hnn/archive.hpp"

//何かのログ(数値)など、常に増え続ける履歴を管理するコンテナクラス
//データが増えすぎたときに過去分をうまく間引いて管理


template<class T>
class HistoryContainer;

namespace hnn {
template<class Arc, class T>
struct SerialFunc<Arc, HistoryContainer<T>>;
}

template<class T>
class HistoryContainer {
	int id = 0;
	size_t baseSize = 10;
	std::list<std::pair<size_t, T>>con;
	std::optional<T> min, max;
	size_t minID = -1, maxID = -1;
public:
	friend struct hnn::SerialFunc<hnn::detail::ArchiveR, HistoryContainer<T>>;
	friend struct hnn::SerialFunc<hnn::detail::ArchiveW, HistoryContainer<T>>;
	//約数として持つ2の数を返す
	int countFactor2(int n) {
		if (n == 0)return std::numeric_limits<int>::max();
		int res = 0;
		if (n < 0)n = -n;
		while (true) {
			if (n % 2 == 1) {
				return res;
			}
			n /= 2;
			++res;
		}
	}
	//計算量(size)	※ちゃんと作ればlogオーダにできるがめんどくさい
	void push_back(const T& t) {
		if (!min || t < *min) {
			minID = id;
			min = t;
		}
		if (!max || *max < t) {
			maxID = id;
			max = t;
		}
		//
		con.push_back({ id,t });
		++id;
		//圧縮処理
		for (auto itr = con.begin(); itr != con.end();) {
			auto diff = id - itr->first;
			int nn = diff / baseSize;
			int deg = hnn::math::log2Int(nn) + 1;
			//最大最小値は消さない
			if (deg > countFactor2(itr->first) && itr->first != minID && itr->first != maxID) {
				itr = con.erase(itr);
			}
			else {
				++itr;
			}
		}
	}
	//基本サイズ(=実サイズの半分 だが、push_backが2^sizeより大きいときはサイズを超過する仕様)
	void setBaseSize(size_t sz) {
		baseSize = sz;
	}
	auto begin() {
		return con.begin();
	}
	auto end() {
		return con.end();
	}
	std::optional<std::pair<T, T>> getMinMax() {
		if (min && max) {
			return std::optional<std::pair<T, T>>(std::pair<T, T>{ *min, * max });
		}
		else {
			return std::nullopt;
		}
	}
	size_t size() const {
		return con.size();
	}
};

namespace hnn {
template<class Arc, class T>
struct SerialFunc<Arc, HistoryContainer<T>> {
	static void f(Arc& ar, HistoryContainer<T>& hc) {
		serialize(ar, hc.id);
		serialize(ar, hc.baseSize);
		serialize(ar, hc.con);
		serialize(ar, hc.min);
		serialize(ar, hc.max);
		serialize(ar, hc.minID);
		serialize(ar, hc.maxID);
	}
};//class SerialFunc {
}//namespace hnn {

