
#pragma once

#include "ai_base.h"
#include "hnn/othello/board_state.hpp"

//edax�̈ڐA

class AIEdax final :public AIbase {
public:
	Evaluation evalate(const hnn::othello::BoardState&state) const override;
	void loadEvalFile(const std::string &path);
private:
	/** number of plies */
	static constexpr int EVAL_N_PLY = 61;
	/** number of features */
	static constexpr int EVAL_N_FEATURE = 47;
	/** number of (unpacked) weights */
	static constexpr int EVAL_N_WEIGHT = 226315;
	static constexpr auto VERSION = 4;
	static constexpr auto RELEASE = 4;
	static constexpr auto BOOK = 0x424f4f4b;
	static constexpr auto EDAX = 0x45444158;
	static constexpr auto EVAL = 0x4556414c;
	static constexpr auto XADE = 0x58414445;
	static constexpr auto LAVE = 0x4c415645;
	/** feature size */
	static constexpr std::array<int, 13> EVAL_SIZE
		= { 19683, 59049, 59049, 59049, 6561, 6561, 6561, 6561, 2187,729, 243, 81, 1 };
	/** packed feature size */
	static constexpr std::array<int, 13> EVAL_PACKED_SIZE
		= { 10206, 29889, 29646, 29646, 3321, 3321, 3321, 3321, 1134, 378, 135, 45, 1 };
	/** constants for square coordinates */
	enum {
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		PASS, NOMOVE
	};
	/** feature to coordinates conversion */
	struct FeatureToCoordinate {
		int n_square;
		std::array<int, 16> x;
	};
	/** array to convert features into coordinates */
	static constexpr std::array<FeatureToCoordinate, EVAL_N_FEATURE> EVAL_F2X = {
		FeatureToCoordinate{ 9, {A1, B1, A2, B2, C1, A3, C2, B3, C3}},
		{ 9, {H1, G1, H2, G2, F1, H3, F2, G3, F3}},
		{ 9, {A8, A7, B8, B7, A6, C8, B6, C7, C6}},
		{ 9, {H8, H7, G8, G7, H6, F8, G6, F7, F6}},

		{10, {A5, A4, A3, A2, A1, B2, B1, C1, D1, E1}},
		{10, {H5, H4, H3, H2, H1, G2, G1, F1, E1, D1}},
		{10, {A4, A5, A6, A7, A8, B7, B8, C8, D8, E8}},
		{10, {H4, H5, H6, H7, H8, G7, G8, F8, E8, D8}},

		{10, {B2, A1, B1, C1, D1, E1, F1, G1, H1, G2}},
		{10, {B7, A8, B8, C8, D8, E8, F8, G8, H8, G7}},
		{10, {B2, A1, A2, A3, A4, A5, A6, A7, A8, B7}},
		{10, {G2, H1, H2, H3, H4, H5, H6, H7, H8, G7}},

		{10, {A1, C1, D1, C2, D2, E2, F2, E1, F1, H1}},
		{10, {A8, C8, D8, C7, D7, E7, F7, E8, F8, H8}},
		{10, {A1, A3, A4, B3, B4, B5, B6, A5, A6, A8}},
		{10, {H1, H3, H4, G3, G4, G5, G6, H5, H6, H8}},

		{ 8, {A2, B2, C2, D2, E2, F2, G2, H2}},
		{ 8, {A7, B7, C7, D7, E7, F7, G7, H7}},
		{ 8, {B1, B2, B3, B4, B5, B6, B7, B8}},
		{ 8, {G1, G2, G3, G4, G5, G6, G7, G8}},

		{ 8, {A3, B3, C3, D3, E3, F3, G3, H3}},
		{ 8, {A6, B6, C6, D6, E6, F6, G6, H6}},
		{ 8, {C1, C2, C3, C4, C5, C6, C7, C8}},
		{ 8, {F1, F2, F3, F4, F5, F6, F7, F8}},

		{ 8, {A4, B4, C4, D4, E4, F4, G4, H4}},
		{ 8, {A5, B5, C5, D5, E5, F5, G5, H5}},
		{ 8, {D1, D2, D3, D4, D5, D6, D7, D8}},
		{ 8, {E1, E2, E3, E4, E5, E6, E7, E8}},

		{ 8, {A1, B2, C3, D4, E5, F6, G7, H8}},
		{ 8, {A8, B7, C6, D5, E4, F3, G2, H1}},

		{ 7, {B1, C2, D3, E4, F5, G6, H7}},
		{ 7, {H2, G3, F4, E5, D6, C7, B8}},
		{ 7, {A2, B3, C4, D5, E6, F7, G8}},
		{ 7, {G1, F2, E3, D4, C5, B6, A7}},

		{ 6, {C1, D2, E3, F4, G5, H6}},
		{ 6, {A3, B4, C5, D6, E7, F8}},
		{ 6, {F1, E2, D3, C4, B5, A6}},
		{ 6, {H3, G4, F5, E6, D7, C8}},

		{ 5, {D1, E2, F3, G4, H5}},
		{ 5, {A4, B5, C6, D7, E8}},
		{ 5, {E1, D2, C3, B4, A5}},
		{ 5, {H4, G5, F6, E7, D8}},

		{ 4, {D1, C2, B3, A4}},
		{ 4, {A5, B6, C7, D8}},
		{ 4, {E1, F2, G3, H4}},
		{ 4, {H5, G6, F7, E8}},

		{ 0, {NOMOVE}}
	};
	/** feature offset */
	static constexpr std::array<int, EVAL_N_FEATURE> EVAL_OFFSET = {
			 0,      0,      0,      0,
		 19683,  19683,  19683,  19683,
		 78732,  78732,  78732,  78732,
		137781,	137781, 137781, 137781,
		196830,	196830, 196830, 196830,
		203391,	203391, 203391, 203391,
		209952,	209952, 209952, 209952,
		216513,	216513,
		223074,	223074,	223074,	223074,
		225261,	225261,	225261,	225261,
		225990,	225990, 225990,	225990,
		226233,	226233, 226233, 226233,
		226314,
	};
public:
	AIEdax();
	std::array<int, EVAL_N_FEATURE> getFeature(const hnn::othello::BoardState&state) const;
private:
	/**
	 * @brief Opponent feature.
	 *
	 * Compute a feature from the opponent point of view.
	 * @param l feature.
	 * @param d feature size.
	 * @return opponent feature.
	 */
	static int opponent_feature(int l, int d);
	/**
	 * @brief Mirror the unsigned int (little <-> big endian).
	 * @param i An unsigned int.
	 * @return The mirrored int.
	 */
	unsigned int bswap_int(unsigned int i);
	/**
	 * @brief Swap bytes of a short (little <-> big endian).
	 * @param s An unsigned short.
	 * @return The mirrored short.
	 */
	unsigned short bswap_short(unsigned short s);
	/**
	 * @brief Get square color.
	 *
	 * returned value: 0 = player, 1 = opponent, 2 = empty;
	 *
	 * @param board board.
	 * @param x square coordinate.
	 * @return square color.
	 */
	int board_get_square_color(const hnn::othello::BoardState&state, const int x) const {
		return 2 - 2 * ((state.turnStone().get() >> x) & 1) - ((state.nonTurnStone().get() >> x) & 1);
	}

private:
	/** feature symetry packing */
	std::vector<std::vector<int>> EVAL_C10;
	std::vector<std::vector<int>> EVAL_S10;
	std::vector<std::vector<int>> EVAL_C9;
	std::vector<std::vector<int>> EVAL_S8;
	std::vector<std::vector<int>> EVAL_S7;
	std::vector<std::vector<int>> EVAL_S6;
	std::vector<std::vector<int>> EVAL_S5;
	std::vector<std::vector<int>> EVAL_S4;
	std::vector<std::vector<std::vector<short>>> EVAL_WEIGHT;
	double EVAL_A, EVAL_B, EVAL_C, EVAL_a, EVAL_b, EVAL_c;
};//class AIEdax
