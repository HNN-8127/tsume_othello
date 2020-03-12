
#include <vector>

#include "ai_edax.h"
#include "evaluation.h"

#pragma warning (disable:4996)		//'fopen': This function or variable may be unsafe.

Evaluation AIEdax::evalate(const hnn::othello::BoardState & state) const {
	const auto f = getFeature(state);
	int score = 0;
	const auto &w = EVAL_WEIGHT[/*state.blackTurn ? 0 : 1*/0][state.countStone() - 4];
	for (size_t i = 0; i < f.size(); ++i) {
		score += w[f[i]];
	}
	score = score * 100 / 128;
	//if (score > 0) score += 1;	else score -= 1;
	score = std::clamp(score, -6500, 6500);
	return Evaluation(score);
}


AIEdax::AIEdax() {
	EVAL_C10.resize(2);
	EVAL_S10.resize(2);
	EVAL_C9.resize(2);
	EVAL_S8.resize(2);
	EVAL_S7.resize(2);
	EVAL_S6.resize(2);
	EVAL_S5.resize(2);
	EVAL_S4.resize(2);
	for (size_t i = 0; i < 2; ++i) {
		EVAL_C10[i].resize(59049);
		EVAL_S10[i].resize(59049);
		EVAL_C9[i].resize(19683);
		EVAL_S8[i].resize(6561);
		EVAL_S7[i].resize(2187);
		EVAL_S6[i].resize(729);
		EVAL_S5[i].resize(243);
		EVAL_S4[i].resize(81);
	}
	loadEvalFile("data\\edax.dat");
}

void AIEdax::loadEvalFile(const std::string & path) {
	unsigned int edax_header, eval_header;
	unsigned int version, release, build;
	double date;
	const int n_w = 114364;
	std::vector<int> T;
	int ply, i, j, k, l, n;
	int r;
	int offset;
	FILE* f;
	// the following is assumed:
	//	-(unsigned) int are 32 bits
	static_assert(sizeof(int) == 4, "int size is not compatible with Edax.\n");
	//	-(unsigned) short are 16 bits
	static_assert(sizeof(short) == 2, "short size is not compatible with Edax.\n");
	// create unpacking tables
	T.resize(59049);
	for (l = n = 0; l < 6561; l++) { /* 8 squares : 6561 -> 3321 */
		k = ((l / 2187) % 3) + ((l / 729) % 3) * 3 + ((l / 243) % 3) * 9 +
			((l / 81) % 3) * 27 + ((l / 27) % 3) * 81 + ((l / 9) % 3) * 243 +
			((l / 3) % 3) * 729 + (l % 3) * 2187;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S8[0][l] = T[l];
		EVAL_S8[1][opponent_feature(l, 8)] = T[l];
	}
	for (l = n = 0; l < 2187; l++) { /* 7 squares : 2187 -> 1134 */
		k = ((l / 729) % 3) + ((l / 243) % 3) * 3 + ((l / 81) % 3) * 9 +
			((l / 27) % 3) * 27 + ((l / 9) % 3) * 81 + ((l / 3) % 3) * 243 +
			(l % 3) * 729;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S7[0][l] = T[l];
		EVAL_S7[1][opponent_feature(l, 7)] = T[l];
	}
	for (l = n = 0; l < 729; l++) { /* 6 squares : 729 -> 378 */
		k = ((l / 243) % 3) + ((l / 81) % 3) * 3 + ((l / 27) % 3) * 9 +
			((l / 9) % 3) * 27 + ((l / 3) % 3) * 81 + (l % 3) * 243;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S6[0][l] = T[l];
		EVAL_S6[1][opponent_feature(l, 6)] = T[l];
	}
	for (l = n = 0; l < 243; l++) { /* 5 squares : 243 -> 135 */
		k = ((l / 81) % 3) + ((l / 27) % 3) * 3 + ((l / 9) % 3) * 9 +
			((l / 3) % 3) * 27 + (l % 3) * 81;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S5[0][l] = T[l];
		EVAL_S5[1][opponent_feature(l, 5)] = T[l];
	}
	for (l = n = 0; l < 81; l++) { /* 4 squares : 81 -> 45 */
		k = ((l / 27) % 3) + ((l / 9) % 3) * 3 + ((l / 3) % 3) * 9 + (l % 3) * 27;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S4[0][l] = T[l];
		EVAL_S4[1][opponent_feature(l, 4)] = T[l];
	}
	for (l = n = 0; l < 19683; l++) { /* 9 corner squares : 19683 -> 10206 */
		k = ((l / 6561) % 3) * 6561 + ((l / 729) % 3) * 2187 +
			((l / 2187) % 3) * 729 + ((l / 243) % 3) * 243 + ((l / 27) % 3) * 81 +
			((l / 81) % 3) * 27 + ((l / 3) % 3) * 9 + ((l / 9) % 3) * 3 + (l % 3);
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_C9[0][l] = T[l];
		EVAL_C9[1][opponent_feature(l, 9)] = T[l];
	}
	for (l = n = 0; l < 59049; l++) { /* 10 squares (edge +X ) : 59049 -> 29646 */
		k = ((l / 19683) % 3) + ((l / 6561) % 3) * 3 + ((l / 2187) % 3) * 9 +
			((l / 729) % 3) * 27 + ((l / 243) % 3) * 81 + ((l / 81) % 3) * 243 +
			((l / 27) % 3) * 729 + ((l / 9) % 3) * 2187 + ((l / 3) % 3) * 6561 +
			(l % 3) * 19683;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_S10[0][l] = T[l];
		EVAL_S10[1][opponent_feature(l, 10)] = T[l];
	}
	for (l = n = 0; l < 59049; l++) { /* 10 squares (angle + X) : 59049 -> 29889 */
		k = ((l / 19683) % 3) + ((l / 6561) % 3) * 3 + ((l / 2187) % 3) * 9 +
			((l / 729) % 3) * 27 + ((l / 243) % 3) * 243 + ((l / 81) % 3) * 81 +
			((l / 27) % 3) * 729 + ((l / 9) % 3) * 2187 + ((l / 3) % 3) * 6561 +
			(l % 3) * 19683;
		if (k < l) T[l] = T[k];
		else T[l] = n++;
		EVAL_C10[0][l] = T[l];
		EVAL_C10[1][opponent_feature(l, 10)] = T[l];
	}
	T.clear();
	// allocation
	try {
		EVAL_WEIGHT.resize(2);
		EVAL_WEIGHT[0].resize(EVAL_N_PLY);
		EVAL_WEIGHT[1].resize(EVAL_N_PLY);
		for (size_t id = 0; id < EVAL_N_PLY; ++id) {
			EVAL_WEIGHT[0][id].resize(EVAL_N_WEIGHT);
			EVAL_WEIGHT[1][id].resize(EVAL_N_WEIGHT);
		}
	}
	catch (...) {
		throw std::runtime_error("Cannot evaluation weights");
	}
	// data reading
	std::vector<short> w(n_w);
	f = fopen(path.c_str(), "rb");
	if (f == NULL) {
		throw std::runtime_error("Cannot open file");
	}
	// File header
	r = fread(&edax_header, sizeof(int), 1, f);
	r += fread(&eval_header, sizeof(int), 1, f);
	if (r != 2 || (!(edax_header == EDAX || eval_header == EVAL)
		&& !(edax_header == XADE || eval_header == LAVE))) {
		throw std::runtime_error("not an Edax evaluation file");
	}
	r = fread(&version, sizeof(int), 1, f);
	r += fread(&release, sizeof(int), 1, f);
	r += fread(&build, sizeof(int), 1, f);
	r += fread(&date, sizeof(double), 1, f);
	if (r != 4) {
		throw std::runtime_error("Cannot read version info from file");
	}
	if (edax_header == XADE) {
		version = bswap_int(version);
		release = bswap_int(release);
		build = bswap_int(build);
	}
	// Weights : read & unpacked them
	for (ply = 0; ply < EVAL_N_PLY; ply++) {
		r = fread(&w.front(), sizeof(short), n_w, f);
		if (r != n_w) {
			throw std::runtime_error("Cannot read evaluation weight from file");
		}
		if (edax_header == XADE) for (i = 0; i < n_w; ++i) w[i] = bswap_short(w[i]);
		i = j = offset = 0;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_C9[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_C9[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_C10[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_C10[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S10[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S10[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S10[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S10[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S8[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S8[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S8[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S8[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S8[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S8[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S8[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S8[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S7[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S7[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S6[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S6[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S5[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S5[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		for (k = 0; k < EVAL_SIZE[i]; k++, j++) {
			EVAL_WEIGHT[0][ply][j] = w[EVAL_S4[0][k] + offset];
			EVAL_WEIGHT[1][ply][j] = w[EVAL_S4[1][k] + offset];
		}
		offset += EVAL_PACKED_SIZE[i];
		i++;
		EVAL_WEIGHT[0][ply][j] = w[offset];
		EVAL_WEIGHT[1][ply][j] = w[offset];
	}
	fclose(f);
	w.clear();
	EVAL_A = -0.10026799, EVAL_B = 0.31027733, EVAL_C = -0.57772603;
	EVAL_a = 0.07585621, EVAL_b = 1.16492647, EVAL_c = 5.4171698;
}


std::array<int, AIEdax::EVAL_N_FEATURE> AIEdax::getFeature(const hnn::othello::BoardState& state) const {
	std::array<int, AIEdax::EVAL_N_FEATURE> feature;
	for (int i = 0; i < EVAL_N_FEATURE; ++i) {
		feature[i] = 0;
		for (int j = 0; j < EVAL_F2X[i].n_square; j++) {
			int c = board_get_square_color(state, EVAL_F2X[i].x[j]);
			feature[i] = feature[i] * 3 + c;
		}
		feature[i] += EVAL_OFFSET[i];
	}
	return feature;
}

int AIEdax::opponent_feature(int l, int d) {
	static const int o[] = { 1, 0, 2 };
	int f = o[l % 3];

	if (d > 1) f += opponent_feature(l / 3, d - 1) * 3;

	return f;
}

unsigned int AIEdax::bswap_int(unsigned int i) {
	i = ((i >> 8) & 0x00FF00FFU) | ((i << 8) & 0xFF00FF00U);
	i = ((i >> 16) & 0x0000FFFFU) | ((i << 16) & 0xFFFF0000U);
	return i;
}

unsigned short AIEdax::bswap_short(unsigned short s) {
	return (unsigned short)((s >> 8) & 0x00FF) | ((s << 8) & 0xFF00);
}
