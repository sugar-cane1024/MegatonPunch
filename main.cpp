#define _USE_MATH_DEFINES
#include "DxLib.h"
#include <math.h>
#include <fstream>
#include <map>
#include <string>

#include "resource.h"

int cursorsound;
int entersound;
int smashsound;
int bgmsound;

static const int CRACK_MIN = 98; // 地面にヒビが入るメガトンの下限
static const int MEGATON_MAX = 192; // メガトンの上限値
static const int COM_TIMING[3][3] = { {56, 103, 90}, {67, 97, 93}, {63, 97, 96} };
//comの押すタイミング。64, 96, 96が最良

int key[256]; //キーの押下時間

int UpdateKey() {
	char tmpKey[256];
	GetHitKeyStateAll(tmpKey);
	for (int i = 0; i < 256; i++) {
		if (tmpKey[i] != 0) {
			key[i]++;
		}
		else {
			key[i] = 0;
		}
	}
	return 0;
}

class PLAYER {
	public:
		PLAYER() {
			phase = 0;
			timing[0] = 0; timing[1] = 0; timing[2] = 0;
			timer = 0;
			megaton = 0;
			point[0] = 0; point[1] = 0; point[2] = 0;
			key = 0;
			posy = 0;
			aimtime = 0;
			crack = 0.0;
			impact = false;
			iscom = false;
		}
		int phase;
		int timing[3];
		int timer;
		int megaton;
		int point[3];
		int key;
		int posy;
		int aimtime;
		double crack;
		bool impact;
		bool iscom = false;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SetOutApplicationLogValidFlag(FALSE);
	
	std::map<std::string, int> importConfig(std::string filePath);

	std::map<std::string, int> config = importConfig("config.cfg");
	int windowmode = config["WindowMode"];
	if (windowmode == 0) {
		ChangeWindowMode(FALSE);
	}
	else {
		ChangeWindowMode(TRUE);
	}
	
	SetMainWindowText("めがとんパンチ - MEGATON PUNCH -");
	SetWindowIconID(IDI_ICON2);

	if (DxLib_Init() == -1)		// ＤＸライブラリ初期化処理
	{
		return -1;			// エラーが起きたら直ちに終了
	}

	SetDrawScreen(DX_SCREEN_BACK);

	void loopPlayer(PLAYER& player);
	

	PLAYER player1 = PLAYER();
	PLAYER player2 = PLAYER();
	int gamephase = 0;
	bool gameend = false;
	int lvl = 1;
	int pl = 1;
	int player2chara = 0;

	int titlegraph;
	titlegraph = LoadGraph("img/title.png");
	int backgraph;
	backgraph = LoadGraph("img/background.png");
	int stonegraph;
	stonegraph = LoadGraph("img/stone.png");
	int playergraph[16];
	LoadDivGraph("img/players.png", 16, 4, 4, 32, 32, playergraph);
	int gaugegraph;
	gaugegraph = LoadGraph("img/gauge.png");
	int innergraph[2];
	LoadDivGraph("img/gauge_inner.png", 2, 2, 1, 16, 64, innergraph);
	int targetgraph[2];
	LoadDivGraph("img/target.png", 2, 2, 1, 16, 16, targetgraph);
	int pendulumgraph[3];
	LoadDivGraph("img/pendulum.png", 3, 3, 1, 16, 16, pendulumgraph);
	int spacegraph;
	spacegraph = LoadGraph("img/space.png");
	int crackgraph;
	crackgraph = LoadGraph("img/crack.png");
	int cursorgraph[2];
	LoadDivGraph("img/cursor.png", 2, 2, 1, 16, 16, cursorgraph);

	int megatonfont = CreateFontToHandle(NULL, 64, -1, DX_FONTTYPE_EDGE, -1, 4);
	int menufont = CreateFontToHandle(NULL, 32, -1, DX_FONTTYPE_NORMAL);

	cursorsound = LoadSoundMem("sound/cursor.wav");
	entersound = LoadSoundMem("sound/enter.wav");
	smashsound = LoadSoundMem("sound/smash.wav");
	bgmsound = LoadSoundMem("sound/bgm.ogg");
	
	int framecount = 0;
	while (ScreenFlip() == 0 && ProcessMessage() == 0 && ClearDrawScreen() == 0 && UpdateKey() == 0 && ++framecount) {
		
		switch (gamephase) {
		case 0:
			//計算
			if (key[KEY_INPUT_UP] == 1) {
				if (pl <= 1) pl = 2;
				else pl--;
				PlaySoundMem(cursorsound, DX_PLAYTYPE_BACK, TRUE);
			}
			else if (key[KEY_INPUT_DOWN] == 1) {
				if (pl >= 2) pl = 1;
				else pl++;
				PlaySoundMem(cursorsound, DX_PLAYTYPE_BACK, TRUE);
			}
			if (pl == 1) {
				if (key[KEY_INPUT_LEFT] == 1) {
					if (lvl <= 1) lvl = 3;
					else lvl--;
					PlaySoundMem(cursorsound, DX_PLAYTYPE_BACK, TRUE);
				}
				else if (key[KEY_INPUT_RIGHT] == 1) {
					if (lvl >= 3) lvl = 1;
					else lvl++;
					PlaySoundMem(cursorsound, DX_PLAYTYPE_BACK, TRUE);
				}
			}
			if (key[KEY_INPUT_SPACE] == 1) {
				if (pl == 1) {
					player1 = PLAYER();
					player2 = PLAYER();
					player2.iscom = true;
					switch (lvl) {
					case 1:
						player2chara = playergraph[8];
						break;
					case 2:
						player2chara = playergraph[4];
						break;
					case 3:
						player2chara = playergraph[12];
						break;
					}
					gamephase = 1;
				}
				else if (pl == 2) {
					player1 = PLAYER();
					player2 = PLAYER();
					player2.iscom = false;
					player2chara = playergraph[4];
					gamephase = 1;
				}
				PlaySoundMem(entersound, DX_PLAYTYPE_BACK, TRUE);
				if (CheckSoundMem(bgmsound) == FALSE && config["Bgm"] == 1) {
					PlaySoundMem(bgmsound, DX_PLAYTYPE_LOOP);
				}
			}

			//描画
			DrawExtendGraph(0, 0, 640, 480, titlegraph, TRUE);
			DrawStringToHandle(180, 320, "1Player Level\n2Player", GetColor(255, 255, 255), menufont);
			DrawExtendGraph(150, 288 + pl * 34, 150 + 32, 288 + pl * 34 + 32, cursorgraph[0], TRUE);
			if (pl == 1) {
				DrawFormatStringToHandle(420, 320, GetColor(255, 255, 0), menufont, "←%d→", lvl);			}
			break;
		case 1:
			//計算
			player1.key = key[KEY_INPUT_Z];
			player2.key = key[KEY_INPUT_M];
			loopPlayer(player1);
			if (player2.iscom == true) {
				if (player2.phase == 0 && player2.aimtime == COM_TIMING[lvl - 1][0]) {
					player2.key = 1;
				}
				else if (player2.phase == 2 && player2.aimtime == COM_TIMING[lvl - 1][1]) {
					player2.key = 1;
				}
				else if (player2.phase == 4 && player2.aimtime == COM_TIMING[lvl - 1][2]) {
					player2.key = 1;
				}
				else {
					player2.key = 0;
				}
			}
			loopPlayer(player2);

			if (player1.phase >= 7 && player2.phase >= 7 && (player1.key == 1 || player2.key == 1)) {
				gamephase = 0;
			}

			//描画
			DrawExtendGraph(0, 0, 640, 480, backgraph, TRUE);
			DrawExtendGraph(160, 320 + player1.posy, 160 + 64, 320 + 64 + player1.posy, playergraph[0], TRUE);
			DrawExtendGraph(416, 320 + player2.posy, 416 + 64, 320 + 64 + player2.posy, player2chara, TRUE);

			if (player1.phase == 0 || player1.phase == 1) {
				DrawExtendGraph(96 - 2, 300 - 2, 128 + 2, 428 + 2, gaugegraph, TRUE);
				if (player1.timing[0] == 64) {
					DrawExtendGraph(96, 428 - 64 * 2, 128, 428, innergraph[1], TRUE);
				}
				else if (player1.timing[0] > 64) {
					DrawExtendGraph(96, 428 - 64 * 2 + (player1.timing[0] - 64) * 2, 128, 428, innergraph[0], TRUE);
				}
				else {
					DrawExtendGraph(96, 428 - player1.timing[0] * 2, 128, 428, innergraph[0], TRUE);
				}
			}
			else if (player1.phase == 2 || player1.phase == 3) {
				int just = 0;
				if (player1.timing[1] == 64) just = 1;
				double radian = ((double)(player1.timing[1] + 64.0) / 64.0) * M_PI;
				DrawExtendGraph(176 + (32 * sin(radian)), 340 + (32 * cos(radian)), 176 + 32 + (32 * sin(radian)), 340 + 32 + (32 * cos(radian)), targetgraph[just], TRUE);
				DrawExtendGraph(176 + (32 * -sin(radian)), 404 + (32 * -cos(radian)), 176 + 32 + (32 * -sin(radian)), 404 + 32 + (32 * -cos(radian)), targetgraph[just], TRUE);
			}
			else if (player1.phase == 4 || player1.phase == 5) {
				int just = 0;
				if (player1.timing[2] == 0 || player1.timing[2] == 64) just = 1;
				double radian = ((double)(player1.timing[2] + 64.0) / 64.0) * M_PI;
				DrawExtendGraph(176 + (48 * sin(sin(radian) * M_PI / 2)), 240 + (48 * cos(sin(radian) * M_PI / 2)), 176 + 32 + (48 * sin(sin(radian) * M_PI / 2)), 240 + 32 + (48 * cos(sin(radian) * M_PI / 2)), pendulumgraph[just], TRUE);
				DrawExtendGraph(176, 240 + (48), 176 + 32, 240 + 32 + (48), pendulumgraph[2], TRUE);
			}

			if (player2.phase == 0 || player2.phase == 1) {
				DrawExtendGraph(512 - 2, 300 - 2, 544 + 2, 428 + 2, gaugegraph, TRUE);
				if (player2.timing[0] == 64) {
					DrawExtendGraph(512, 428 - 64 * 2, 544, 428, innergraph[1], TRUE);
				}
				else if (player2.timing[0] > 64) {
					DrawExtendGraph(512, 428 - 64 * 2 + (player2.timing[0] - 64) * 2, 544, 428, innergraph[0], TRUE);
				}
				else {
					DrawExtendGraph(512, 428 - player2.timing[0] * 2, 544, 428, innergraph[0], TRUE);
				}
			}
			else if (player2.phase == 2 || player2.phase == 3) {
				int just = 0;
				if (player2.timing[1] == 64) just = 1;
				double radian = ((double)(player2.timing[1] + 64.0) / 64.0) * M_PI;
				DrawExtendGraph(432 + (32 * sin(radian)), 340 + (32 * cos(radian)), 432 + 32 + (32 * sin(radian)), 340 + 32 + (32 * cos(radian)), targetgraph[just], TRUE);
				DrawExtendGraph(432 + (32 * -sin(radian)), 404 + (32 * -cos(radian)), 432 + 32 + (32 * -sin(radian)), 404 + 32 + (32 * -cos(radian)), targetgraph[just], TRUE);
			}
			else if (player2.phase == 4 || player2.phase == 5) {
				int just = 0;
				if (player2.timing[2] == 0 || player2.timing[2] == 64) just = 1;
				double radian = ((double)(player2.timing[2] + 64.0) / 64.0) * M_PI;
				DrawExtendGraph(432 + (48 * sin(sin(radian) * M_PI / 2)), 240 + (48 * cos(sin(radian) * M_PI / 2)), 432 + 32 + (48 * sin(sin(radian) * M_PI / 2)), 240 + 32 + (48 * cos(sin(radian) * M_PI / 2)), pendulumgraph[just], TRUE);
				DrawExtendGraph(432, 240 + (48), 432 + 32, 240 + 32 + (48), pendulumgraph[2], TRUE);
			}

			if (player1.impact == true) {
				DrawRectExtendGraph(0, 0, 320, 480, 0, 0, 160, 240, spacegraph, FALSE);
				int cracklength = 0;
				if (player1.timer > 40) {
					cracklength = (int)(128.0 * player1.crack);
					cracklength *= 2;
				}
				else if (player1.timer > 35) {
					cracklength = (int)(128.0 * player1.crack);
				}
				DrawRectExtendGraph(192, 128, 320, 128 + cracklength, 0, 0, 64, cracklength / 2, crackgraph, TRUE);
			}
			if (player2.impact == true) {
				DrawRectExtendGraph(320, 0, 640, 480, 160, 0, 160, 240, spacegraph, FALSE);
				int cracklength = 0;
				if (player2.timer > 40) {
					cracklength = (int)(128.0 * player2.crack);
					cracklength *= 2;
				}
				else if (player2.timer > 35) {
					cracklength = (int)(128.0 * player2.crack);
				}
				DrawRectExtendGraph(320, 128, 448, 128 + cracklength, 64, 0, 64, cracklength / 2, crackgraph, TRUE);
			}

			if (player1.phase == 7) {
				char mt[100];
				sprintf_s(mt, "%dMt.", player1.megaton);
				DrawStringToHandle(64, 128, mt, GetColor(255, 0, 0), megatonfont, GetColor(255, 255, 255));
				SetFontSize(32);
				//DrawFormatString(64, 192, GetColor(255, 0, 0), "%dMt.", player1.point[0]);
				//DrawFormatString(64, 224, GetColor(255, 0, 0), "%dMt.", player1.point[1]);
				//DrawFormatString(64, 256, GetColor(255, 0, 0), "%dMt.", player1.point[2]);
			}
			if (player2.phase == 7) {
				char mt[100];
				sprintf_s(mt, "%dMt.", player2.megaton);
				DrawStringToHandle(384, 128, mt, GetColor(255, 0, 0), megatonfont, GetColor(255, 255, 255));
			}
			break;
		default:
			gameend = true;
		}

		if (key[KEY_INPUT_ESCAPE] == 1)break;

		if (gameend)break;

	}

	DxLib_End();				// ＤＸライブラリ使用の終了処理
	
	return 0;				// ソフトの終了 
}

void loopPlayer(PLAYER& player) {
	if (player.phase == 0) { //0～127、64に近いほど良
		if (player.key == 1) {
			if (player.timing[0] == 64) {
				PlaySoundMem(entersound, DX_PLAYTYPE_BACK, TRUE);
			}
			else {
				PlaySoundMem(cursorsound, DX_PLAYTYPE_BACK, TRUE);
			}
			player.phase = 1;
			player.aimtime = 0;
			player.timer = 25 + GetRand(10);
			return;
		}
		else {
			player.timing[0]++;
			if (player.timing[0] > 127) {
				player.timing[0] = 0;
			}
			player.aimtime++;
		}
	}
	else if (player.phase == 1) {
		if (player.timer <= 0) {
			player.phase = 2;
			player.timing[1] = 96;
		}
		else {
			player.timer--;
		}
	}
	else if (player.phase == 2) { //0～127、64に近いほど良
		if (player.key == 1) {
			if (player.timing[1] == 64) {
				PlaySoundMem(entersound, DX_PLAYTYPE_BACK, TRUE);
			}
			else {
				PlaySoundMem(cursorsound, DX_PLAYTYPE_BACK, TRUE);
			}
			player.phase = 3;
			player.aimtime = 0;
			player.timer = 25 + GetRand(10);
			return;
		}
		else {
			player.timing[1]++;
			if (player.timing[1] > 127) {
				player.timing[1] = 0;
			}
			player.aimtime++;
		}
	}
	else if (player.phase == 3) {
		if (player.timer <= 0) {
			player.phase = 4;
			player.timing[2] = 96;
		}
		else {
			player.timer--;
		}
	}
	else if (player.phase == 4) { //0～127、0か64に近いほど良
		if (player.key == 1) {
			if (player.timing[2] == 0 || player.timing[2] == 64) {
				PlaySoundMem(entersound, DX_PLAYTYPE_BACK, TRUE);
			}
			else {
				PlaySoundMem(cursorsound, DX_PLAYTYPE_BACK, TRUE);
			}
			player.phase = 5;
			player.aimtime = 0;
			player.timer = 30;
			player.point[0] = 64 - abs(player.timing[0] - 64);
			player.point[1] = 64 - abs(player.timing[1] - 64);
			if (player.timing[2] <= 32) {
				player.point[2] = 32 - player.timing[2];
			}
			else if (player.timing[2] <= 96) {
				player.point[2] = 32 - abs(64 - player.timing[2]);
			}
			else {
				player.point[2] = 32 - abs(128 - player.timing[2]);
			}
			player.point[2] *= 2;
			player.megaton = 2 + player.point[0] + player.point[1] + player.point[2];
			//最小0Mt. 最大192Mt.
			player.crack = (double)(player.megaton - CRACK_MIN) / (double)(MEGATON_MAX - CRACK_MIN);
			return;
		}
		else {
			player.timing[2]++;
			if (player.timing[2] > 127) {
				player.timing[2] = 0;
			}
			player.aimtime++;
		}
	}
	else if (player.phase == 5) {
		if (player.timer <= 0) {
			player.phase = 6;
			player.timer = 0;
		}
		else {
			player.timer--;
		}
	}
	else if (player.phase == 6) {
		if (player.timer < 20) {
			player.posy -= 3;
		}
		else if (player.timer < 30) {
			player.posy += 6;
		}
		else if (player.timer < 150) {
			if (player.megaton >= CRACK_MIN) {
				player.impact = true;
			}
		}
		else {
			player.impact = false;
			player.phase = 7;
		}
		if (player.timer == 30)PlaySoundMem(smashsound, DX_PLAYTYPE_BACK);
		player.timer++;
	}
	else if (player.phase == 7) {

	}
	return;
}

std::map<std::string, int> importConfig(std::string filePath) {
	std::map<std::string, int> confMap;
	std::ifstream ifs(filePath.c_str());
	std::string buf;
	while (ifs && std::getline(ifs, buf)) {
		if (buf.substr(0, 1).compare("#") != 0) {
			
			size_t nEq = buf.find("=");
			if (nEq != -1) {
				std::string key = buf.substr(0, nEq);
				std::string val = buf.substr(nEq + 1);
				confMap[key] = std::stoi(val);
			}
		}
	}
	ifs.close();
	return confMap;
}