
#pragma once

#include <string>
#include <vector>
#include <list>

#include "hnn/archive.hpp"

namespace problem_cache {
	struct Key {
		int nBlank;
		std::vector<int> seed;
		bool operator ==(const Key& other)const = default;
	};
	struct Data :public Key {
		std::string kif;
	};
	//
	inline std::list<Data>cache;
	//
	std::string get(int nBlank, const std::vector<int>& seed);
	//amount:計算する量
	void update1step(double amount);
	//先読み計算を予約(キュー:先入れ先出し)
	void addReserve(int nBlank, const std::vector<int>& seed);
	void clearReserve();
	int getNreserve();
	void setCacheSize(int sz);
}//namespace problem_cache {


namespace hnn {
	template<class Arc>
	struct SerialFunc<Arc, problem_cache::Data> {
		static void f(Arc& ar, problem_cache::Data& data) {
			serialize(ar, data.kif);
			serialize(ar, data.nBlank);
			serialize(ar, data.seed);
		}
	};//class SerialFunc {
}//namespace hnn {
