
#include <random>


#include "Dxlib.h"

#include "problem_cache.h"
#include "make_board.h"

namespace problem_cache {
	int maxSize = 1000;
	std::list<Key> waited;
	std::optional<Key> calcing;
	std::mt19937 random;
	//
	void setCalcing(const Key& k) {
		calcing = k;
		auto vec = k.seed;
		vec.push_back(k.nBlank);
		std::seed_seq seq(vec.begin(), vec.end());
		random.seed(seq);
	}
	auto findFromCache(int nBlank, const std::vector<int>& seed) {
		for (auto itr = cache.begin(); itr != cache.end(); ++itr) {
			if (itr->seed == seed && itr->nBlank == nBlank) {
				return itr;
			}
		}
		return cache.end();
	}
	std::string get(int nBlank, const std::vector<int>& seed) {
		auto itr = findFromCache(nBlank, seed);
		if (itr != cache.end()) {
			auto res = itr->kif;
			auto one = *itr;
			cache.erase(itr);
			cache.push_front(one);
			return res;
		}
		//å©Ç¬Ç©ÇÁÇ»Ç¢èÍçá
		Key k;
		k.nBlank = nBlank;
		k.seed = seed;
		if (calcing) {
			if ((*calcing) == k) {
				//âΩÇ‡ÇµÇ»Ç¢
			}
			else {
				waited.push_back(*calcing);
				setCalcing(k);
			}
		}
		else {
			setCalcing(k);
		}
		while (calcing) {
			update1step(1.0);
		}
		return get(nBlank, seed);
	}
	//nÉ}ÉXÇÃãlÇﬂÉIÉZÉçåÛï‚1å¬ÇåvéZÇ∑ÇÈÇÃÇ…Ç©Ç©ÇÈÇ®Ç®ÇÊÇªÇÃéûä‘
	double estimatedCalcForce(int n) {
		double res = 1.0;	//60éËé„ë≈ÇøêiÇﬂÇÈéûä‘
		res += std::tgamma(n / 2.0 + 1.0) / 10;		//a-bñ@Ç…ÇÊÇÈÇ∆åvéZó ÇÕê[Ç≥ÇÃîºï™ÇÃäKèÊÇÁÇµÇ¢
		return res;
	}
	double update1stepImpl() {
		if (calcing) {
			auto kif = make_board::calc(calcing->nBlank, random);
			if (kif) {
				auto bd = make_board::kifToBoard(*kif);
				if (make_board::ok(bd)) {
					Data data;
					data.kif = *kif;
					data.nBlank = calcing->nBlank;
					data.seed = calcing->seed;
					cache.push_front(data);
					calcing = std::nullopt;
					if (cache.size() > maxSize) {
						cache.resize(maxSize);
					}
				}
				//printfDx("%d", calcing->nBlank);
				return estimatedCalcForce(calcing->nBlank);
			}
			else {
				return 1.0;
			}
		}
		else if (!waited.empty()) {
			setCalcing(waited.back());
			waited.pop_back();
			return 0.0;
		}
		else {
			//åvéZèIóπ
			return -1.0;
		}
	}
	void update1step(double amount) {
		do {
			double tmp = update1stepImpl();
			if (tmp < 0)break;
			amount -= tmp;
		} while (0 < amount);
	}
	void addReserve(int nBlank, const std::vector<int>& seed) {
		auto itr = findFromCache(nBlank, seed);
		if (itr == cache.end()) {
			Key k;
			k.nBlank = nBlank;
			k.seed = seed;
			waited.push_front(k);
		}
	}
	void clearReserve() {
		waited.clear();
	}
	int getNreserve() {
		return waited.size();
	}
	void setCacheSize(int sz) {
		maxSize = sz;
	}
}//namespace problem_cache {



