
#pragma once

#include <cstdint>
#include <cmath>
#include <string>

#undef min
#undef max

//�]���l��}�X������

class Evaluation {
	std::int16_t val = 0;
public:
	Evaluation() = default;
	Evaluation(int val_) {
		set((std::int16_t)val_);
	}
	int getInt() const {
		return (int)std::floor(((double)val + 50) / 100.0);
	}
	std::int16_t getRawValue() const {
		return val;
	}
	double getDouble() const {
		return val / 100.0;
	}
	void set(std::int16_t val_) {
		val = val_;
	}
	// "+1"���̕\���p������
	std::string getString() const;
	//
	void reverse() {
		val = -val;
	}
	Evaluation inverse() const {
		Evaluation res = *this;
		res.reverse();
		return res;
	}
	bool operator ==(const Evaluation& other)const {
		return val == other.val;
	}
	bool operator <(const Evaluation& other)const {
		return val < other.val;
	}
	bool operator >(const Evaluation& other)const {
		return val > other.val;
	}
	bool operator <=(const Evaluation& other)const {
		return val <= other.val;
	}
	bool operator >=(const Evaluation& other)const {
		return val >= other.val;
	}
	Evaluation operator +(const Evaluation& other)const {
		return Evaluation(val + other.val);
	}
	static Evaluation min() {
		return Evaluation(-10000);
	}
	static Evaluation max() {
		return Evaluation(10000);
	}
}; //class Evaluation {



inline static Evaluation minEvaluationValue = { -std::numeric_limits<std::int16_t>::max() };	//min���g����-1�{�̉��Z���ł��Ȃ�
inline static Evaluation maxEvaluationValue = { std::numeric_limits<std::int16_t>::max() };

