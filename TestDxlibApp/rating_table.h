
#pragma once

#include <map>

static inline std::map<int, int>ratingTable[2] = {
	{
		{1,0} ,
		{2,1550} ,
		{3,1700} ,
		{4,1850} ,
		{5,2000} ,
		{6,2050} ,
		{7,2100} ,
		{8,2150} ,
		{9,2200} ,
		{10,2250} ,
		{11,9999} ,
		},
	{
		{1,0} ,
		{2,1200} ,
		{3,1400} ,
		{4,1600} ,
		{5,1700} ,
		{6,1800} ,
		{7,1900} ,
		{8,2000} ,
		{9,2100} ,
		{10,2150} ,
		{11,9999} ,
	}
};

static const inline int nBlankMax = 10;

inline int getLimitTime(int mode, int nBlank) {
	if (mode == 0) {
		return 20 * (nBlank - 1);
	}
	else if (mode == 1) {
		return 60 * nBlank;
	}
	else {
		return -1;
	}
}


