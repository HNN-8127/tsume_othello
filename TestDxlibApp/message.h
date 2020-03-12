
#pragma once

#include <vector>
#include <string>

#include "Dxlib.h"

#include "hnn/dxlib_draw_string.hpp"

class Message {
	int x, y;
	int h = 30;
	int lifeTime;
	int count;
	std::string mes;
public:
	void set(int x_, int y_, int time_, const std::string& s) {
		x = x_;
		y = y_;
		lifeTime = time_;
		mes = s;
		count = 0;
	}
	void draw() {
		int alpha = 200;
		if (count >= lifeTime - 10) {
			alpha = (lifeTime - count) * alpha / 10;
		}
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		int w = GetDrawStringWidth(mes.c_str(), mes.size());
		int ww = w + 10;
		DrawRoundRect(x - ww / 2, y - h / 2, x + ww / 2, y + h / 2, 5, 5, GetColor(48, 48, 48), TRUE);
		DrawString(x - w / 2, y + h / 2 - 24, mes.c_str(), GetColor(255, 255, 255));
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	bool update() {
		++count;
		if (count >= lifeTime)return false;
		return true;
	}
};

namespace message {
	inline std::vector<Message>messages;
	//
	void draw() {
		for (auto& one : messages) {
			one.draw();
		}
	}
	Message& add() {
		messages.push_back({});
		return messages.back();
	}
	void update() {
		for (auto itr = messages.begin(); itr != messages.end(); ) {
			if (!itr->update()) {
				itr = messages.erase(itr);
			}
			else {
				++itr;
			}
		}
	}
}//namespace message {
