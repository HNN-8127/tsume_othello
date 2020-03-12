
#pragma once

#include <list>
#include <limits>
#include <optional>

#include "hnn/math.hpp"
#include "hnn/archive.hpp"

//�����̃��O(���l)�ȂǁA��ɑ��������闚�����Ǘ�����R���e�i�N���X
//�f�[�^�������������Ƃ��ɉߋ��������܂��Ԉ����ĊǗ�


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
	//�񐔂Ƃ��Ď���2�̐���Ԃ�
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
	//�v�Z��(size)	�������ƍ���log�I�[�_�ɂł��邪�߂�ǂ�����
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
		//���k����
		for (auto itr = con.begin(); itr != con.end();) {
			auto diff = id - itr->first;
			int nn = diff / baseSize;
			int deg = hnn::math::log2Int(nn) + 1;
			//�ő�ŏ��l�͏����Ȃ�
			if (deg > countFactor2(itr->first) && itr->first != minID && itr->first != maxID) {
				itr = con.erase(itr);
			}
			else {
				++itr;
			}
		}
	}
	//��{�T�C�Y(=���T�C�Y�̔��� �����Apush_back��2^size���傫���Ƃ��̓T�C�Y�𒴉߂���d�l)
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

