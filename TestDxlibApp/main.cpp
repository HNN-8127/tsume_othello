



#include <chrono>

#include "DxLib.h"


#include "hnn/unit_test.hpp"
#include "hnn/file.hpp"
#include "hnn/dxlib_mouse_input.hpp"
#include "hnn/dxlib_color.hpp"
#include "hnn/othello/core.hpp"
#include "hnn/ini2_parser_base.hpp"
#include "hnn/random.hpp"
#include "hnn/system.hpp"
#include "hnn/dxlib_draw_string.hpp"
#include "hnn/date.hpp"

#include "save_data.h"
#include "make_board.h"
#include "problem_cache.h"
#include "message.h"
#include "evaluation_all.h"
#include "save_data_free.h"
#include "user_history.h"




namespace {
constexpr int windowWidth = 800;
constexpr int windowHeight = 600;
constexpr int boardX = 35;
constexpr int boardY = 80;
constexpr int boardSize = 40;
constexpr int maxWait = 20;
constexpr int version = 4;
constexpr int INVALID_SOUND_HANDLE = -1;
}



struct Button {
	std::string title;
	int mode;
	int x, y, w = 100, h = 40;
	unsigned int getColor();
	void draw();
	void update();
	bool mouseHit();
	bool enable();
	std::optional<std::string> changeTitle();
};

struct UndoData {
	hnn::othello::BoardState bd;
	std::optional<hnn::othello::BoardPosition> lastPut;
	EvaluationAll ev;
};

class Timer {
	int milliCount;		//設定時間
	std::chrono::steady_clock::time_point base;
	int milli, milliPrev;			//現在時間
	bool stopFlag = false;
public:
	int getMilli() {
		return milli;
	}
	void start(int milliCount_) {
		base = std::chrono::steady_clock::now();
		milliCount = milliCount_;
		stopFlag = false;
	}
	void update() {
		milliPrev = milli;
		if (!stopFlag) {
			auto now = std::chrono::steady_clock::now();
			milli = milliCount - (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - base).count();
		}
	}
	void stop() {
		stopFlag = true;
	}
	bool reached(int time) {
		return milli <= time && time < milliPrev;
	}
};


std::string getSavePath(int ver) {
	if (ver == 0) {
		return "data/save.dat";
	}
	else {
		return hnn::string::sprintf("data/save%d.dat", ver);
	}
}



double getRatingDiff(double winR, double loseR, int n) {
	double a = 16.0 + 128.0 * 5.0 / (n + 5.0);
	return a / (std::pow(10, (winR - loseR) / 400.0) + 1);
}

namespace {
int seedParam = 1;						//問題制御パラメータ
int timeShift = 0;
bool enableSound = true;
bool doReview = true;
hnn::dxlib::MouseInput mi(windowWidth, windowHeight);
hnn::dxlib::Color color;
hnn::othello::BoardState bs;		//問題
hnn::othello::BoardState bsNow;		//現局面
std::string kifu;
std::optional<hnn::othello::BoardPosition> lastPut;
std::optional<hnn::othello::BoardState> memoryBoard;		//右側の記憶領域
int compareTime = 0;
int result = 0;				//1でwin
bool solved = false;		//今回、一度でも正解したか
bool challanged = false;		//今回、一度でもトライしたか
bool answerMode = false;
bool rateChanged = false;
bool first = true;
int bestEval = 100;				//最善で進んだ場合playerの〇石勝ち
//
int wait = 0;
//
EvaluationAll eAll;
//
std::vector<Button>buttons;
SaveData sds[2];
SavaDataFree sdf;
int freeModeNblank = 2;
//
Problem now;
int nowMode;	//今の盤面のgameMode
int autoSaveFlag = 0;
int nWaitLearning[2] = { 0 ,0 };
//
int fontS, fontM, fontL, fontLL;
int grBlack, grWhite, grGold, grSilver, grCursor;
int sdPut = INVALID_SOUND_HANDLE, sdStart = INVALID_SOUND_HANDLE, sdChime = INVALID_SOUND_HANDLE;
//
std::optional<Timer>timer;
int gameMode = 1;		//0時短 1長考 2フリー
//
std::vector<UndoData> undoBoard, redoBoard;		//redoは順が逆
//
UserHistory userHistory;
}

SaveData& getSd() {
	assert(0 <= gameMode && gameMode <= 1);
	return sds[gameMode];
}

void loadResourse() {
	int belowButtonX = 50;
	int belowButtonY = 430;
	{
		Button b;
		b.mode = 0;
		b.title = "やり直し";
		b.x = belowButtonX;
		b.y = belowButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 1;
		b.title = "次の問題";
		b.x = belowButtonX + 1 * (20 + b.w);
		b.y = belowButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 2;
		b.title = "検討";
		b.x = belowButtonX + 2 * (20 + b.w);
		b.y = belowButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 3;
		b.title = "棋譜⇒CB";
		b.x = belowButtonX + 3 * (20 + b.w);
		b.y = belowButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 4;
		b.title = "保留";
		b.x = belowButtonX + 4 * (20 + b.w);
		b.y = belowButtonY;
		buttons.push_back(b);
	}
	const int modeButtonX = 390;
	const int modeButtonY = 40;
	{
		Button b;
		b.mode = 5;
		b.title = "時短";
		b.w = 40;
		b.h = 25;
		b.x = modeButtonX + 0 * (5 + b.w);
		b.y = modeButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 6;
		b.title = "長考";
		b.w = 40;
		b.h = 25;
		b.x = modeButtonX + 1 * (5 + b.w);
		b.y = modeButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 7;
		b.title = "ﾌﾘｰ";
		b.w = 40;
		b.h = 25;
		b.x = modeButtonX + 2 * (5 + b.w);
		b.y = modeButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 8;
		b.title = "はじめる";
		b.x = 150;
		b.y = 300;
		buttons.push_back(b);
	}
	const int kentouButtonX = 20;
	const int kentouButtonY = 420;
	const int kentouButtonW = 30;
	{
		Button b;
		b.mode = 9;
		b.title = "|<";
		b.w = kentouButtonW;
		b.h = 25;
		b.x = kentouButtonX + 0 * (5 + b.w);
		b.y = kentouButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 10;
		b.title = "＜";
		b.w = kentouButtonW;
		b.h = 25;
		b.x = kentouButtonX + 1 * (5 + b.w);
		b.y = kentouButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 11;
		b.title = "＞";
		b.w = kentouButtonW;
		b.h = 25;
		b.x = kentouButtonX + 2 * (5 + b.w);
		b.y = kentouButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 12;
		b.title = ">|";
		b.w = kentouButtonW;
		b.h = 25;
		b.x = kentouButtonX + 3 * (5 + b.w);
		b.y = kentouButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 13;
		b.title = "Ｍ";
		b.w = kentouButtonW;
		b.h = 25;
		b.x = kentouButtonX + 0 * (5 + b.w);
		b.y = kentouButtonY + 1 * (5 + b.h);
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 14;
		b.title = "比";
		b.w = kentouButtonW;
		b.h = 25;
		b.x = kentouButtonX + 1 * (5 + b.w);
		b.y = kentouButtonY + 1 * (5 + b.h);
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 15;
		b.title = "ﾎｰﾑ";
		b.w = 40;
		b.h = 25;
		b.x = modeButtonX + 3 * (5 + b.w);
		b.y = modeButtonY;
		buttons.push_back(b);
	}
	int freeButtonX = 400;
	int freeButtonY = 110;
	for (int i = 1; i <= 9; ++i) {
		Button b;
		b.mode = 15 + i;
		b.title = hnn::string::sprintf("%dﾏｽ", i + 1);
		b.w = 40;
		b.h = 25;
		b.x = freeButtonX + (i % 5) * (5 + b.w);
		b.y = freeButtonY + (i / 5) * (5 + b.h);
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 25;
		b.title = "＜前問";
		b.w = 40;
		b.x = belowButtonX + 125;
		b.y = belowButtonY;
		buttons.push_back(b);
	}
	{
		Button b;
		b.mode = 26;
		b.title = "次問＞";
		b.w = 40;
		b.x = belowButtonX + 175;
		b.y = belowButtonY;
		buttons.push_back(b);
	}
	/*for (int i = 0; i < 2; ++i) {
		Button b;
		b.mode = 27 + i;
		b.title = "test" + std::to_string(i);
		b.w = 40;
		b.x = 100 + 50 * i;
		b.y = 100;
		buttons.push_back(b);
	}*/
	//
	hnn::random::setSeedRandom();
	//
	int thick = -1;
	fontS = CreateFontToHandle("", 12, thick, DX_FONTTYPE_ANTIALIASING_4X4);
	fontM = CreateFontToHandle("", 20, thick, DX_FONTTYPE_ANTIALIASING_4X4);
	fontL = CreateFontToHandle("", 40, thick, DX_FONTTYPE_ANTIALIASING_4X4);
	fontLL = CreateFontToHandle("", 60, thick, DX_FONTTYPE_ANTIALIASING_4X4);
	//
	grBlack = LoadGraph("data/image/black.png");
	grWhite = LoadGraph("data/image/white.png");
	grGold = LoadGraph("data/image/gold.png");
	grSilver = LoadGraph("data/image/silver.png");
	grCursor = LoadGraph("data/image/cursor.png");
	//
	if (enableSound) {
		sdPut = LoadSoundMem("data/sound/put.ogg");
		sdStart = LoadSoundMem("data/sound/start.ogg");
		sdChime = LoadSoundMem("data/sound/chime.ogg");
	}
}


bool dxlibInit() {
	SetWindowIconID(100);
	SetMainWindowText(hnn::string::sprintf("ひたすらつめおせろ by HNN_8127 ばーじょん%d", version).c_str());
	ChangeWindowMode(TRUE);		//ウインドウモードにする
	SetWindowSize(windowWidth, windowHeight);
	// 非アクティブ時も処理を止めないようにする
	SetAlwaysRunFlag(TRUE);
	// ＤＸライブラリ初期化処理
	if (DxLib_Init() == -1) {
		return false;
	}
	SetDrawScreen(DX_SCREEN_BACK); //描画先を裏画面に設定
	return true;
}




//次の盤面を検索
void calcNext(double amount) {
	problem_cache::update1step(amount);
	ProcessMessage();
}

EvaluationAll recalcAnswerImpl(const hnn::othello::BoardState& bs_) {
	EvaluationAll res;
	for (int x = 0; x < 8; ++x) {
		for (int y = 0; y < 8; ++y) {
			auto tmp = bs_.put({ x,y });
			if (tmp) {
				auto ev = sab.execute(*tmp, &perfect, 60, false);
				res.set({ x,y }, ev.eval, ev.way);
			}
		}
	}
	return res;
}

void recalcAnswer() {
	eAll = recalcAnswerImpl(bsNow);
}


void retryQuestion() {
	bsNow = bs;
	recalcAnswer();
	lastPut = std::nullopt;
	result = 0;
	bestEval = sab.execute(bsNow, &perfect, 60, false).eval.getInt();
}


bool isFreeMode(int mo = gameMode) {
	return mo >= 2;
}

void nextQuestion() {
	if (isFreeMode()) {
		now.id = sdf.blanks[freeModeNblank - 2].cursorPos;
		now.nBlank = freeModeNblank;
	}
	else {
		now = getSd().next(now, gameMode, doReview);
	}
	nowMode = gameMode;
	//
	kifu = problem_cache::get(now.nBlank, { now.id,gameMode,seedParam });
	bs = make_board::kifToBoard(kifu);
	if (!isFreeMode()) {
		auto itr = getSd().findProblem(now);
		if (itr != getSd().problem.end() && itr->cleared()) {
			//一度でもクリアしてるなら向きをランダムに
			int rotate = hnn::random::getInt(0, 4);
			bs = bs.rotate(rotate);
			if (rotate % 2 == 0)bs = bs.reverse();
		}
	}
	retryQuestion();
	solved = false;
	challanged = false;
	answerMode = false;
	rateChanged = false;
	if (isFreeMode()) {
		if (sdf.blanks[now.nBlank - 2].answerResult[now.id] != 0) {
			//フリーかつ解答済なら最初から検討可能
			solved = true;
			challanged = true;
		}
	}
	wait = 0;
	//
	problem_cache::clearReserve();
	if (!isFreeMode()) {
		nWaitLearning[gameMode] = sds[gameMode].getNumberOfReLearning(now);
		timer = Timer();
		timer->start(getLimitTime(gameMode, now.nBlank) * 1000);
		//次問題の予約
		problem_cache::addReserve(now.nBlank, { getSd().history[now.nBlank].nextID  ,gameMode ,seedParam });
		for (auto& one : getSd().getNextReLearning(now)) {
			problem_cache::addReserve(one.nBlank, { one.id ,gameMode,seedParam });
		}
		for (int n = 2; n <= nBlankMax; ++n) {
			if (now.nBlank != n) {
				problem_cache::addReserve(n, { getSd().history[n].nextID ,gameMode,seedParam });
			}
		}
		problem_cache::addReserve(now.nBlank, { getSd().history[now.nBlank].nextID + 1 ,gameMode,seedParam });
	}
	else {
		//free問題
		for (auto offset : { 1,-1,2,-2 }) {
			for (int n = 2; n <= nBlankMax; ++n) {
				int id = sdf.blanks[n - 2].cursorPos + offset;
				if (1 <= id) {
					problem_cache::addReserve(n, { id ,2,seedParam });
				}
			}
		}
	}
	//
	memoryBoard = std::nullopt;
	PlaySoundMem(sdStart, DX_PLAYTYPE_BACK);
}

void changeRate(bool win) {
	if (rateChanged)return;
	auto& sd = getSd();
	auto itr = sd.findProblem(now);
	if (itr == sd.problem.end()) {
		//初回のみrated
		int nn = sd.sumHistory().n();
		if (win) {
			sd.rate += getRatingDiff(sd.rate, ratingTable[nowMode][now.nBlank], nn);
			sd.history[now.nBlank].win++;
			if (sd.succeedWinStat < 0)sd.succeedWinStat /= 2;
			++sd.succeedWinStat;
		}
		else {
			sd.rate -= getRatingDiff(ratingTable[nowMode][now.nBlank], sd.rate, nn);
			sd.history[now.nBlank].lose++;
			if (sd.succeedWinStat > 0)sd.succeedWinStat /= 2;
			--sd.succeedWinStat;
		}
		sd.ratingHistory.push_back((int)sd.rate);
	}
	sd.registerResult(now, win);
	sd.rateMax = std::max(sd.rateMax, sd.getAppearRate());
	if (timer) {
		timer->stop();
	}
	rateChanged = true;
}




void update(int x, int y, int size) {
	//時間切れ判定は着手より前(着手に時間がかかるので)
	if (timer) {
		timer->update();
		//時間切れ
		if (timer->reached(0) && !rateChanged) {
			changeRate(false);
			message::add().set(200, 200, 50, "時間切れです");
		}
		if (timer->reached(10 * 1000)) {
			message::add().set(200, 200, 50, "残り10秒");
			PlaySoundMem(sdChime, DX_PLAYTYPE_BACK);
		}
		if (timer->reached(30 * 1000)) {
			message::add().set(200, 200, 50, "残り30秒");
			PlaySoundMem(sdChime, DX_PLAYTYPE_BACK);
		}
		for (int i = 1; i <= 9; ++i) {
			if (timer->reached(i * 60 * 1000)) {
				message::add().set(200, 200, 50, hnn::string::sprintf("残り%d分", i));
				PlaySoundMem(sdChime, DX_PLAYTYPE_BACK);
			}
		}
		for (int i = 0; i <= 5; ++i) {
			if (timer->reached(i * 1000)) {
				PlaySoundMem(sdChime, DX_PLAYTYPE_BACK);
			}
		}
	}
	//
	--wait;
	if (wait == 1) {
		//終局判定
		auto end = bsNow.checkEndGame();
		if (end && !answerMode) {
			result = hnn::sign(*end) * (bsNow.blackTurn ? 1 : -1) * (bs.blackTurn ? 1 : -1);
			//引き分け負け
			if (result == 0)result = -1;
			if (1 == result && !solved) {
				if (isFreeMode()) {
					//free特有の正答記録処理
					if (sdf.blanks[now.nBlank - 2].answerResult[now.id] == 0) {
						sdf.blanks[now.nBlank - 2].answerResult[now.id] = 1;
						sdf.blanks[now.nBlank - 2].nSolved++;
						userHistory.addProgress(timeShift);
					}
				}
				else {
					auto& sd = getSd();
					sd.history[now.nBlank].nSolved++;
					userHistory.addProgress(timeShift);
				}
				solved = true;
			}
			if (!challanged) {
				challanged = true;
				if (!isFreeMode()) {
					changeRate(result == 1);
				}
			}
		}
	}
	else if (wait == 3) {
		//敵の番なら置く
		if (bs.blackTurn != bsNow.blackTurn) {
			bsNow = *bsNow.put(*eAll.getMaximumPos());
			lastPut = *eAll.getMaximumPos();
			bsNow.pass();
			if (!bsNow.checkEndGame()) {
				wait = maxWait;
			}
			PlaySoundMem(sdPut, DX_PLAYTYPE_BACK);
		}
		recalcAnswer();
	}
	else if (wait < 0) {
		//自分の番の置く処理
		wait = 0;
		if (mi.m_leftClickedTime == 1) {
			int i = (mi.m_lastClicked_x_L - x + size * 100) / size - 100;
			int j = (mi.m_lastClicked_y_L - y + size * 100) / size - 100;
			if (0 <= i && i < 8 && 0 <= j && j < 8) {
				if (bs.blackTurn == bsNow.blackTurn || answerMode) {
					if (answerMode) {
						undoBoard.push_back({ bsNow,lastPut ,eAll });
						redoBoard.clear();
					}
					auto newBs = bsNow.put({ i,j });
					if (newBs) {
						bsNow = *newBs;
						if (bsNow.pass() || answerMode) {
							wait = 2;
						}
						else {
							wait = maxWait;
						}
						lastPut = { i,j };
						recalcAnswer();
						PlaySoundMem(sdPut, DX_PLAYTYPE_BACK);
					}
				}
			}
		}
	}
	else if (10000 < wait) {
		//無限に待たせる
		wait = 12000;
	}
	//
	for (auto& one : buttons) {
		one.update();
	}
	--compareTime;
	if (compareTime < 0)compareTime = 0;
}

int drawLineAAint(int x1, int y1, int x2, int y2, unsigned int Color, float Thickness = 1.0f) {
	return DrawLineAA((float)x1, (float)y1, (float)x2, (float)y2, Color, Thickness);
}
int drawCircleAAint(int x, int y, int r, int posnum, unsigned int Color, int FillFlag, float LineThickness = 1.0f) {
	return DrawCircleAA((float)x, (float)y, (float)r, posnum, Color, FillFlag, LineThickness);
}

//盤っぽいのを描画
void drawBoard(int x, int y, int size, const hnn::othello::BoardState& bs_, bool drawExtra) {
	float radius = 1.0f * size * 2 / 5;
	float radius2 = 1.0f * size / 8;
	float radius3 = 1.0f * size / 12;
	int edgeSize = size / 3;
	float lineThickness = radius / 20.0f;
	auto lineColor = color.darkGreen;
	const int nVertex = 32;	//円を近似する際の角
	DrawBox(x - edgeSize, y - edgeSize, x + size * 8 + edgeSize, y + size * 8 + edgeSize, color.darkGray, TRUE);
	DrawBox(x, y, x + size * 8, y + size * 8, color.othelloGreen, TRUE);
	for (int i = 0; i <= 8; ++i) {
		drawLineAAint(x, y + size * i, x + size * 8, y + size * i, lineColor, lineThickness);
		drawLineAAint(x + size * i, y, x + size * i, y + size * 8, lineColor, lineThickness);
	}
	//星
	drawCircleAAint(x + 2 * size, y + 2 * size, radius3, nVertex, lineColor, TRUE);
	drawCircleAAint(x + 2 * size, y + 6 * size, radius3, nVertex, lineColor, TRUE);
	drawCircleAAint(x + 6 * size, y + 6 * size, radius3, nVertex, lineColor, TRUE);
	drawCircleAAint(x + 6 * size, y + 2 * size, radius3, nVertex, lineColor, TRUE);
	//
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j) {
			auto pos = hnn::othello::BoardPosition{ i,j };
			int si = bs_.blackSign(pos);
			std::string str;
			int handle = -1;
			if (si == 1) {
				handle = grBlack;
			}
			else if (si == -1) {
				handle = grWhite;
			}
			if (handle != -1) {
				if (compareTime > 0 && memoryBoard && memoryBoard->blackSign(pos) == bs_.blackSign(pos) && drawExtra) {
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, 32);
				}
				else {
					SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
				}
				int rr = size * 2 / 5;
				DrawExtendGraph(x + i * size + size / 2 - rr, y + j * size + size / 2 - rr
					, x + i * size + size / 2 + rr, y + j * size + size / 2 + rr
					, handle, TRUE);
			}
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
			//最終着手
			if (lastPut && *lastPut == pos && drawExtra) {
				drawCircleAAint(x + i * size + size / 2, y + j * size + size / 2, radius2, nVertex, color.red, TRUE);
			}
			//着手可能マーカ
			auto ev = eAll.get(pos);
			bool myTurn = (bs.blackTurn == bs_.blackTurn);
			if (ev && drawExtra) {
				int mx = (mi.m_pointer_x - x) / size;
				int my = (mi.m_pointer_y - y) / size;
				//カーソルの重なり
				if (mx == i && my == j && (answerMode || myTurn)) {
					DrawBoxAA(x + i * size + lineThickness, y + j * size + lineThickness
						, x + i * size + size - lineThickness, y + j * size + size - lineThickness, color.lightGreen, TRUE);
				}
				if (answerMode) {
					//評価値
					int val = ev->getInt();
					hnn::dxlib::draw::string::advanced(x + i * size + size / 4, y + j * size + size / 4, 20
						, (val > 0 ? color.cyan : color.orange), fontM, 0.5, false, "%s%d", (val >= 0 ? "+" : ""), val);
				}
				else if (myTurn) {
					DrawBox(x + i * size + size * 2 / 6, y + j * size + size * 2 / 6
						, x + i * size + size * 4 / 6, y + j * size + size * 4 / 6, color.darkYellow, TRUE);
				}
			}
		}
	}
	//石数
	if (!first) {
		int yy = 0;
		int xx = (drawExtra ? 0 : 40);
		if (bs.blackTurn) {
			yy = 7;
		}
		drawCircleAAint(x + 8 * size + 60 - xx, y + yy * size + size / 2, 16, nVertex, color.black, TRUE);
		hnn::dxlib::draw::string::advanced(x + 8 * size + 60 - 10 - xx, y + yy * size + size / 2 - 10, 20
			, color.white, fontM, 0.5, false, "%d", bs_.black.count());
		drawCircleAAint(x + 8 * size + 60 - xx, y + (7 - yy) * size + size / 2, 16, nVertex, color.white, TRUE);
		hnn::dxlib::draw::string::advanced(x + 8 * size + 60 - 10 - xx, y + (7 - yy) * size + size / 2 - 10, 20
			, color.black, fontM, 0.5, false, "%d", bs_.white.count());
		//手番カーソル
		if (!bs_.checkEndGame() && !first && drawExtra) {
			if (bs.blackTurn == bs_.blackTurn) {
				DrawGraph(x + 8 * size + size / 4 + 5, y + 7 * size + 5, grCursor, TRUE);
			}
			else {
				DrawGraph(x + 8 * size + size / 4 + 5, y + 0 * size + 5, grCursor, TRUE);
			}
		}
	}
	//
	if (!answerMode && drawExtra) {
		if (result == 1) {
			DrawStringToHandle(x + 3 * size, y + 3 * size + size / 4, hnn::string::sprintf("WIN").c_str()
				, color.red, fontLL);
			if (abs(bestEval) == 64 - 2 * std::min({ bs_.black.count(), bs_.white.count() })) {
				DrawStringToHandle(x + 2 * size, y + 3 * size + size * 1 + size * 3 / 4, hnn::string::sprintf("perfect!!").c_str()
					, color.red, fontL);
			}
		}
		else if (result == -1) {
			DrawStringToHandle(x + 3 * size, y + 3 * size + size / 4, hnn::string::sprintf("LOSE").c_str()
				, color.blue, fontLL);
		}
	}
	//座標(ABCD123)
	for (int i = 0; i < 8; ++i) {
		std::string s0 = "A";
		std::string s1 = "1";
		s0[0] += i;
		s1[0] += i;
		DrawStringToHandle(x + i * size + size / 2 - 3, y - 12, s0.c_str(), color.black, fontS);
		DrawStringToHandle(x - 10, y + i * size + size / 2 - 5, s1.c_str(), color.black, fontS);
	}
}

auto getRatingColor(int r) {
	auto col = color.darkerGray;
	if (1200 <= r)col = color.darkGreen;
	if (1500 <= r)col = color.blue;
	if (1700 <= r)col = color.yellow;
	if (2000 <= r)col = color.red;
	return col;
}

//芝生
void drawGarden(int x, int y) {
	int size = 26;
	int margin = 1;
	int length = size + margin * 2;
	int dx = -4;
	int dy = 22;
	//
	std::vector<std::string>weekDay = { "日","月" ,"火" ,"水" ,"木" ,"金" ,"土" };
	for (int i = 0; i < 7; ++i) {
		DrawStringToHandle(x + i * length, y, weekDay[i].c_str(), color.black, fontM);
	}
	auto date = getDate();
	date->h -= 3;
	date->regularization();
	int today = hnn::date::getWeakday(*date);
	for (int j = 0; j < 4; ++j) {
		for (int i = 0; i < 7; ++i) {
			int id = (6 - i) + 7 * (3 - j) - (6 - today);
			if (0 <= id) {
				unsigned int col;
				double r = 0.0;
				if (0 == userHistory.progress[id]) {
					col = GetColor(150, 150, 150);
				}
				else {
					r = std::clamp(userHistory.progress[id] / 40.0, 0.0, 1.0);
					col = GetColor((int)(160 * (1 - r) + 15 * r), (int)(255 * (1 - r) + 60 * r), (int)(160 * (1 - r) + 0 * r));
				}
				DrawBox(x + i * length + margin + dx, y + j * length + margin + dy
					, x + i * length + margin + size + dx, y + j * length + margin + size + dy, col, TRUE);
				if (0 < userHistory.progress[id]) {
					int nnn = userHistory.progress[id];
					hnn::dxlib::draw::string::advanced(x + i * length + margin + dx, y + j * length + margin + dy + 5, size
						, 0.5 < r ? color.white : color.black, fontM, 0.5, true, hnn::string::sprintf("%d", nnn).c_str());
				}
				if (id == 0) {
					DrawBoxAA(x + i * length + margin + dx, y + j * length + margin + dy
						, x + i * length + margin + size + dx, y + j * length + margin + size + dy, GetColor(255, 255, 100), FALSE, 3.0f);
				}
			}
		}
	}
}


void drawGraph(int grX, int grY, int grWidth, int grHeight) {
	DrawBox(grX, grY, grX + grWidth, grY + grHeight, color.darkerGray, FALSE);
	auto& sd = getSd();
	int i = 0;
	auto minMax = sd.ratingHistory.getMinMax();
	if (minMax) {
		int w = sd.ratingHistory.size() - 1;
		int h = minMax->second - minMax->first;
		if (w < 1)w = 1;
		if (h == 0)h = 1;
		int min = minMax->first;
		{
			int line1200 = (1200 - min) * grHeight / h;
			int line1500 = (1500 - min) * grHeight / h;
			int line1700 = (1700 - min) * grHeight / h;
			int line2000 = (2000 - min) * grHeight / h;
			line1200 = std::clamp(line1200, 0, grHeight);
			line1500 = std::clamp(line1500, 0, grHeight);
			line1700 = std::clamp(line1700, 0, grHeight);
			line2000 = std::clamp(line2000, 0, grHeight);
			if (0 != line1200) {
				DrawBox(grX, grY + grHeight - 0, grX + grWidth, grY + grHeight - line1200, color.darkGray, TRUE);
			}
			if (line1200 != line1500) {
				DrawBox(grX, grY + grHeight - line1200, grX + grWidth, grY + grHeight - line1500, color.darkGreen, TRUE);
			}
			if (line1500 != line1700) {
				DrawBox(grX, grY + grHeight - line1500, grX + grWidth, grY + grHeight - line1700, color.blue, TRUE);
			}
			if (line1700 != line2000) {
				DrawBox(grX, grY + grHeight - line1700, grX + grWidth, grY + grHeight - line2000, color.yellow, TRUE);
			}
			if (line2000 != grHeight) {
				DrawBox(grX, grY + grHeight - line2000, grX + grWidth, grY + grHeight - grHeight, color.red, TRUE);
			}
		}
		//
		std::optional<int>prevX, prevY;
		for (auto itr = sd.ratingHistory.begin(); itr != sd.ratingHistory.end(); ++itr) {
			//DrawLine(grX
			auto drawMarker = [&](int x, int y) {
				DrawBox(grX + x - 2, grY + grHeight - y - 2, grX + x + 2, grY + grHeight - y + 2, color.black, TRUE);
				if (prevX && prevY) {
					DrawLine(grX + x, grY + grHeight - y, grX + *prevX, grY + grHeight - *prevY, color.black, TRUE);
				}
				prevX = x;
				prevY = y;
			};
			drawMarker(i * grWidth / w, (itr->second - min) * grHeight / h);
			++i;
		}
		//
		DrawString(grX - 35, grY + grHeight - 15, std::to_string(minMax->first).c_str(), color.black);
		DrawString(grX - 35, grY, std::to_string(minMax->second).c_str(), color.black);
	}
}

void draw() {
	//
	DrawBox(0, 0, windowWidth, windowHeight, GetColor(100, 120, 135), TRUE);
	//
	drawBoard(boardX, boardY, boardSize, bsNow, true);
	if (memoryBoard) {
		drawBoard(500, 340, boardSize / 5, *memoryBoard, false);
	}
	//
	if (first) {
		hnn::dxlib::draw::string::advanced(20, 20, 255, color.red, fontL, 0, true, hnn::string::sprintf("ホーム").c_str());
	}
	else {
		std::map<int, const char*>dic = { {0,"短"} ,{1,"長"} ,{2,"フ"} };
		if (!isFreeMode(nowMode) && !challanged) {
			hnn::dxlib::draw::string::advanced(20, 20, 255, color.red, fontL, 0, true, hnn::string::sprintf("%s-%dマス #？", dic[nowMode], now.nBlank).c_str());
		}
		else {
			hnn::dxlib::draw::string::advanced(20, 20, 255, color.red, fontL, 0, true, hnn::string::sprintf("%s-%dマス #%d", dic[nowMode], now.nBlank, now.id).c_str());
		}
	}
	if (!first) {
		//history
		if (!isFreeMode(nowMode)) {
			if (challanged) {
				auto itr = sds[nowMode].findProblem(now);
				std::vector<char>his;
				if (itr == sds[nowMode].problem.end()) {
					his = { 0,0,0,0,0 };
				}
				else {
					his = itr->history;
				}
				std::map<int, const char*>dic = { {0,"- "} ,{1,"〇"} ,{2,"×"} };
				if (his.back() == 0) {
					DrawString(290, 40, "初挑戦", color.black);
				}
				else {
					for (int i = 0; i < (int)his.size(); ++i) {
						DrawString(270 + 20 * i, 40, dic[his[i]], color.black);
					}
				}
			}
			else {
				DrawString(280, 40, "？？？？", color.black);
			}
			DrawString(280, 20, "history", color.black);
		}
		else {
			if (sdf.blanks[now.nBlank - 2].answerResult[now.id] != 0) {
				DrawStringToHandle(280, 20, "[済]", color.black, fontL);
			}
		}
	}
	//
	int charX = 370;
	if (!isFreeMode()) {
		auto& sd = getSd();
		int xx = charX;
		int grW, grH;
		int graphDy = 0;
		GetGraphSize(grSilver, &grW, &grH);
		//
		auto drawCrown = [&](int r) {
			if (r >= 2400) {
				DrawGraph(xx, 120 + graphDy, grGold, TRUE); xx += grW;
			}
			else if (r >= 2200) {
				DrawGraph(xx, 120 + graphDy, grSilver, TRUE); xx += grW;
			}
		};
		//
		std::string s;
		s = "レート[";
		DrawStringToHandle(xx, 120, s.c_str(), color.black, fontM); xx += GetDrawStringWidthToHandle(s.c_str(), s.size(), fontM);
		drawCrown(sd.getAppearRate());
		s = hnn::string::sprintf("%d", sd.getAppearRate());
		DrawStringToHandle(xx, 120, s.c_str(), getRatingColor(sd.getAppearRate()), fontM); xx += GetDrawStringWidthToHandle(s.c_str(), s.size(), fontM);
		s = "]";
		DrawStringToHandle(xx, 120, s.c_str(), color.black, fontM); xx += GetDrawStringWidthToHandle(s.c_str(), s.size(), fontM);
		s = "最高:";
		DrawStringToHandle(xx, 120, s.c_str(), color.black, fontM); xx += GetDrawStringWidthToHandle(s.c_str(), s.size(), fontM);
		drawCrown(sd.rateMax);
		s = hnn::string::sprintf("%d", sd.rateMax);
		DrawStringToHandle(xx, 120, s.c_str(), getRatingColor(sd.rateMax), fontM); xx += GetDrawStringWidthToHandle(s.c_str(), s.size(), fontM);
	}
	//
	for (auto& one : buttons) {
		one.draw();
	}
	//
	int charY = 175;
	if (!isFreeMode()) {
		auto& sd = getSd();
		DrawStringToHandle(charX, charY, hnn::string::sprintf("総計:").c_str(), color.black, fontM);
		DrawStringToHandle(charX, charY + 25, hnn::string::sprintf("%s", sd.sumHistory().str().c_str()).c_str(), color.black, fontM);
		if (!first) {
			DrawStringToHandle(charX, charY + 75, hnn::string::sprintf("%dマス:", now.nBlank).c_str(), color.black, fontM);
			DrawStringToHandle(charX, charY + 100, hnn::string::sprintf("%s", sd.history[now.nBlank].str().c_str()).c_str(), color.black, fontM);
		}
		//
		DrawStringToHandle(530, 10, hnn::string::sprintf("復習残:%d", nWaitLearning[gameMode]).c_str(), color.darkRed, fontM);
	}
	else {
		int n = 0;
		for (int i = 2; i <= 10; ++i) {
			n += sdf.blanks[i - 2].nSolved;
		}
		DrawStringToHandle(charX, charY, hnn::string::sprintf("総計:%d種正解", n).c_str(), color.black, fontM);
		DrawStringToHandle(charX, charY + 25, hnn::string::sprintf("%dマス:%d種正解", freeModeNblank, sdf.blanks[freeModeNblank - 2].nSolved).c_str(), color.black, fontM);
	}
	//タイマー
	if (timer && !first && !isFreeMode()) {
		int xx = 450;
		int yy = 75;
		DrawRoundRect(xx - 10, yy, xx + 170, yy + 40, 5, 5, color.darkGray, TRUE);
		int time = timer->getMilli();
		if (0 > time)time = 0;
		int second = time / 1000;
		int s = second % 60;
		int m = second / 60;
		int milli = (time % 1000) / 10;
		DrawStringToHandle(xx, yy, hnn::string::sprintf("%02d:%02d.%02d", m, s, milli).c_str(), color.orange, fontL);
	}
	DrawStringToHandle(450, 5, hnn::string::sprintf("ロード中:%d問", problem_cache::getNreserve()).c_str(), color.black, fontS);
	if (first) {
		drawGarden(400, 330);
	}
	//グラフ
	if (first) {
		if (!isFreeMode()) {
			drawGraph(400, 225, 200, 100);
		}
	}
	else if (!memoryBoard && !isFreeMode(nowMode) && !isFreeMode()) {
		drawGraph(480, 315, 150, 100);
	}
	//
	message::draw();
}


void save() {
	//
	{
		hnn::Archive ar;
		ar.setVersion(version);
		ar << sds[0] << sds[1] << sdf << gameMode << freeModeNblank << userHistory;
		ar.save(getSavePath(version));
	}
	{
		hnn::Archive ar;
		ar << problem_cache::cache;
		ar.save("data/problem_cache.dat");
	}
}
void load() {
	int minVer = 4;		//互換性のある最小ver
	for (int v = version; minVer <= v; --v) {
		hnn::Archive ar;
		ar.setVersion(v);
		if (hnn::file::exist(getSavePath(v))) {
			if (ar.load(getSavePath(v))) {
				ar >> sds[0] >> sds[1] >> sdf >> gameMode >> freeModeNblank >> userHistory;
				break;
			}
		}
	}
	//
	{
		hnn::Archive ar;
		if (ar.load("data/problem_cache.dat")) {
			ar >> problem_cache::cache;
		}
	}
	{
		hnn::Ini2ParserBase ini2;
		ini2.loadFile("data/config.ini");
		seedParam = ini2.getInt("problem_kind").value_or(0);
		enableSound = ini2.getBool("sound", true);
		problem_cache::setCacheSize(ini2.getInt("cache_size").value_or(1000));
		doReview = ini2.getBool("do_review", true);
		timeShift = ini2.getInt("date_shift").value_or(0);
	}
}

int surpressCalcFlag = 100;

void mainLoop() {
	while (ProcessMessage() == 0) {
		ClearDrawScreen();
		//
		mi.update();
		update(boardX, boardY, boardSize);
		if (GetFPS() < 50.0f) {
			++surpressCalcFlag;
		}
		else {
			surpressCalcFlag *= 3;
			surpressCalcFlag /= 4;
			--surpressCalcFlag;
			if (0 > surpressCalcFlag)surpressCalcFlag = 0;
		}
		calcNext(20.0 / (surpressCalcFlag + 1));
		//
		message::update();
		//
		draw();
		if (ScreenFlip() != 0) {
			break;
		}
	}
}

void init() {
	solved = true;
	challanged = true;
	answerMode = false;
	first = true;
	wait = 9999999;
	bs = bsNow = {};
	lastPut = std::nullopt;
	result = 0;
	memoryBoard = std::nullopt;
	eAll = {};
	if (sdf.blanks.size() < 9) {
		sdf.blanks.resize(9);
	}
	if (userHistory.progress.size() < 28) {
		userHistory.progress.resize(28);
	}
	{
		//先読み
		problem_cache::clearReserve();
		//rated問題
		for (int n = 2; n <= nBlankMax; ++n) {
			for (int mode = 0; mode <= 1; ++mode) {
				problem_cache::addReserve(n, { sds[mode].history[n].nextID ,mode,seedParam });
			}
		}
		//復習
		for (int mode = 0; mode <= 1; ++mode) {
			for (auto& one : sds[mode].getNextReLearning(now)) {
				problem_cache::addReserve(one.nBlank, { one.id ,gameMode,seedParam });
			}
		}
		//free問題
		for (int n = 2; n <= nBlankMax; ++n) {
			problem_cache::addReserve(n, { sdf.blanks[n - 2].cursorPos ,2,seedParam });
		}
	}
	//
	nWaitLearning[0] = sds[0].getNumberOfReLearning(std::nullopt);
	nWaitLearning[1] = sds[1].getNumberOfReLearning(std::nullopt);
	userHistory.addProgress(timeShift, 0);
	//
	for (int i = 0; i <= 1; ++i) {
		sds[i].ratingHistory.setBaseSize(50);
		if (sds[i].ratingHistory.size() == 0) {
			sds[i].ratingHistory.push_back(sds[i].rate);
		}
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	(void)hInstance, (void)hPrevInstance, (void)lpCmdLine, (void)nCmdShow;
	if (!dxlibInit()) {
		// エラーが起きたら直ちに終了
		return 1;
	}
	load();
	loadResourse();
	init();
	mainLoop();
	save();
	DxLib_End();
	return 0;
}

unsigned int Button::getColor() {
	if (5 <= mode && mode <= 7 && gameMode + 5 == mode) {
		return color.red;
	}
	if (16 <= mode && mode <= 24 && freeModeNblank + 14 == mode) {
		return color.red;
	}
	return mouseHit() ? color.darkYellow : color.darkerYellow;
}

void Button::draw() {
	if (enable()) {
		DrawBox(x, y, x + w, y + h, getColor(), TRUE);
		hnn::dxlib::draw::string::advanced(x, y + h / 2 - 10, w
			, color.black, fontM, 0.5, true, changeTitle().value_or(title).c_str());
	}
}

bool undo() {
	if (!undoBoard.empty()) {
		redoBoard.push_back({ bsNow,lastPut ,eAll });
		bsNow = undoBoard.back().bd;
		lastPut = undoBoard.back().lastPut;
		eAll = undoBoard.back().ev;
		undoBoard.pop_back();
		return true;
	}
	return false;
}
bool redo() {
	if (!redoBoard.empty()) {
		undoBoard.push_back({ bsNow,lastPut ,eAll });
		bsNow = redoBoard.back().bd;
		lastPut = redoBoard.back().lastPut;
		eAll = redoBoard.back().ev;
		redoBoard.pop_back();
		return true;
	}
	return false;
}

void Button::update() {
	if (mi.m_leftClickedTime == 1 && mouseHit() && enable()) {
		switch (mode) {
		case 0:
			retryQuestion();
			break;
		case 1:
			nextQuestion();
			++autoSaveFlag;
			if (autoSaveFlag >= 5) {
				autoSaveFlag = 0;
				save();
			}
			break;
		case 2:
			if (answerMode) {
				answerMode = false;
				retryQuestion();
			}
			else {
				answerMode = true;
				if (result != 0) {
					retryQuestion();
				}
				redoBoard.clear();
				undoBoard.clear();
			}
			break;
		case 3:
			hnn::system::copyToClickBoard(kifu);
			message::add().set(450, 400, 50, "棋譜をクリップボードにコピーしました");
			break;
		case 4: {
			auto itr = getSd().findProblem(now);
			if (itr != getSd().problem.end()) {
				itr->setTime(getSd().getNowTime() + 3600LL * 24 * 40);
				nextQuestion();
			}
			break; }
		case 5:
		case 6: {
			gameMode = mode - 5;
			break; }
		case 7: {
			gameMode = 2;
			//if (!first && nowMode != 2)nextQuestion();
			break; }
		case 8:
			first = false;
			nextQuestion();
			break;
		case 9:
			while (undo());
			break;
		case 10:
			undo();
			break;
		case 11:
			redo();
			break;
		case 12:
			while (redo());
			break;
		case 13:
			memoryBoard = bsNow;
			break;
		case 14:
			if (!memoryBoard) {
				message::add().set(50, 400, 80, "先にMボタンで局面を登録してください");
			}
			else {
				compareTime = 50;
			}
			break;
		case 15:
			init();
			break;
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
			freeModeNblank = mode - 14;
			if (!first)nextQuestion();
			break;
		case 25:
			sdf.blanks[freeModeNblank - 2].cursorPos--;
			if (sdf.blanks[freeModeNblank - 2].cursorPos <= 0)sdf.blanks[freeModeNblank - 2].cursorPos = 1;
			nextQuestion();
			break;
		case 26:
			sdf.blanks[freeModeNblank - 2].cursorPos++;
			nextQuestion();
			break;
			/*case 27:
				getSd().rate += getRatingDiff(getSd().rate, 1800, 40);
				getSd().ratingHistory.push_back((int)getSd().rate);
				break;
			case 28:
				getSd().rate -= getRatingDiff(1800, getSd().rate, 40);
				getSd().ratingHistory.push_back((int)getSd().rate);
				break;*/
		}
	}
}
bool Button::mouseHit() {
	return x <= mi.m_pointer_x && mi.m_pointer_x <= x + w && y <= mi.m_pointer_y && mi.m_pointer_y <= y + h;
}

bool Button::enable() {
	switch (mode) {
	case 0:
		return challanged && !first && !answerMode && !(bs == bsNow);
	case 1:
		return !first && ((solved && !isFreeMode()) || (isFreeMode(nowMode) && !isFreeMode()));
	case 2:
		return challanged && !first;
	case 3:
		return solved && !first;
	case 4: {
		if (first)return false;
		if (isFreeMode())return false;
		auto itr = getSd().findProblem(now);
		if (itr == getSd().problem.end()) return false;
		if (itr->history.back() == 2)return true;
		return false; }
	case 5:
	case 6:
	case 7: {
		return solved || mode - 5 == gameMode || isFreeMode(nowMode); }
	case 8:
		return first;
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		return answerMode;
	case 15:
		return (solved || isFreeMode(nowMode)) && !first;
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
		return isFreeMode();
	case 25:
	case 26:
		return isFreeMode() && !first;
	}
	return true;
}

std::optional<std::string> Button::changeTitle() {
	if (mode == 2 && answerMode) {
		return "検討終了";
	}
	return std::nullopt;
}


