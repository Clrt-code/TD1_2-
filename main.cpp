#include <Novice.h>
#include <corecrt_math.h>
#include <vector>
#include <time.h> 
#define _USE_MATH_DEFINES
#ifdef USE_IMGUI
#include"imgui.h"
#endif

const char kWindowTitle[] = "boss";

const int kWindowWidth = 1280;
const int kWindowHeight = 720;
const int kStageWidth = 2048;
const int kStageHeight = 1440;

// シーン管理用
enum Scene {
	TITLE,
	GAMEPLAY,
	GAMECLEAR,
	GAMEOVER
};

enum Direction {
	UP,
	DOWN,
	RIGHT,
	LEFT,
	UP_RIGHT,
	UP_LEFT,
	DOWN_RIGHT,
	DOWN_LEFT,
};

struct Vector2 {
	float x;
	float y;
};

struct Map {
	Vector2 topLeft = { 0,0 };
	Vector2 topRight = { kWindowWidth,0 };
	Vector2 bottomLeft = { 0,kWindowHeight };
	Vector2 bottomRight = { kWindowWidth,kWindowHeight };

	Vector2 objectTopLeft[15];
	Vector2 objectTopRight[15];
	Vector2 objectBottomLeft[15];
	Vector2 objectBottomRight[15];
	int objectWidth[15];
	int objectHeight[15];
};

struct Player {
	Vector2 pos;
	Vector2 screenPos;
	float width;
	float height;
	float speed;
	int dashCooldown;
	int dashDuration;
	int dashSpeed;
	bool isDashing;
	int hp;
	int maxHp; 
	int invincibilityTimer;
};

struct PlayerBullet {
	Vector2 pos;
	Vector2 velocity;
	bool isActive;
};

struct BossBullet {
	Vector2 pos;
	Vector2 velocity;
	bool isActive;
};

struct Boss {
	Vector2 pos;
	int hp;
	int maxHp; 
	int phase;
	int attackTimer;
};

//変数定義
Player player;
Boss boss;
std::vector<BossBullet> bossBullets;
PlayerBullet pistolBullets[6];
PlayerBullet riffleBullets[30];
PlayerBullet launcherBullets[5];

int pistolAmmo;
int riffleAmmo;
int launcherAmmo;
bool isReloading;
int reloadTime;
bool isLauncherReloading;
int launcherReloadTime;
int launcherFireCooldown;
int rifleFireTimer;

int currentWeapon;
const char* weaponName;

Vector2 rifleBoxPos;
Vector2 launcherBoxPos;
bool rifleUnlocked;
bool launcherUnlocked;

Vector2 world;
int shakeTimer = 0;


void InitGame() {
	player = {
		.pos = {64.0f, 640.0f},
		.screenPos = {0,0},
		.width = 20.0f,
		.height = 20.0f,
		.speed = 4.0f,
		.dashCooldown = 0,
		.dashDuration = 0,
		.dashSpeed = 12,
		.isDashing = false,
		.hp = 10,     
		.maxHp = 10,
		.invincibilityTimer = 0
	};

	boss = { {600.0f, 300.0f}, 100, 100, 1, 0 };
	bossBullets.clear();

	// 弾の初期化
	for (int i = 0; i < 6; i++) pistolBullets[i] = { {0,0}, {0,0}, false };
	for (int i = 0; i < 30; i++) riffleBullets[i] = { {0,0}, {0,0}, false };
	for (int i = 0; i < 5; i++) launcherBullets[i] = { {0,0}, {0,0}, false };

	pistolAmmo = 6;
	riffleAmmo = 30;
	launcherAmmo = 5;

	isReloading = false;
	reloadTime = 0;
	isLauncherReloading = false;
	launcherReloadTime = 0;
	launcherFireCooldown = 0;
	rifleFireTimer = 0;

	currentWeapon = 0;
	weaponName = "Pistol";

	rifleBoxPos = { 400.0f, 300.0f };
	launcherBoxPos = { 800.0f, 500.0f };
	rifleUnlocked = false;
	launcherUnlocked = false;

	world = { 0,0 };
	shakeTimer = 0;
}

int mouseX = 0;
int mouseY = 0;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);
	srand((unsigned int)time(nullptr));

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// 画像読み込み
	int boxW = 40;//itemboxの幅
	int boxH = 40;
	int sprite[10] = {
		Novice::LoadTexture("./sprite/map_background.png"),
		Novice::LoadTexture("./sprite/object_wood.png"),
		Novice::LoadTexture("./sprite/itemTest.png"),
	};

	// マップデータ設定
	Map map = {};
	map.objectTopLeft[0].x = 350;
	map.objectTopLeft[0].y = 350;
	map.objectTopRight[0].x = 440 + 81;
	map.objectTopRight[0].y = 350;
	map.objectBottomLeft[0].x = 350;
	map.objectBottomLeft[0].y = 440 + 128 * 1.5f;
	map.objectBottomRight[0].x = 440 + 81;
	map.objectBottomRight[0].y = 440 + 128 * 1.5f;

	map.objectTopLeft[1].x = 850;
	map.objectTopLeft[1].y = 850;
	map.objectTopRight[1].x = 940 + 81;
	map.objectTopRight[1].y = 850;
	map.objectBottomLeft[1].x = 850;
	map.objectBottomLeft[1].y = 940 + 128 * 1.5f;
	map.objectBottomRight[1].x = 940 + 81;
	map.objectBottomRight[1].y = 940 + 128 * 1.5f;

	const int scrollMarginX = 400;
	const int scrollMarginY = 300;

	// 各種定数
	const int pistolMaxAmmo = 6;
	const int riffleMaxAmmo = 30;
	const int launcherMaxAmmo = 5;
	const int reloadDuration = 90;
	const int launcherReloadDuration = 120;
	const int rifleFireRate = 5;
	const int launcherFireInterval = 48;

	// 最初の初期化
	InitGame();

	// シーン管理変数
	Scene currentScene = TITLE;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);
		Novice::GetMousePosition(&mouseX, &mouseY);

		///
		/// ↓更新・描画処理ここから
		///

		switch (currentScene)
		{
		case TITLE:
			// タイトル画面の処理
			if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
				InitGame();
				currentScene = GAMEPLAY;
			}

			// タイトル描画
			Novice::ScreenPrintf(kWindowWidth / 2 - 50, kWindowHeight / 2 - 20, "SHOOTING GAME");
			Novice::ScreenPrintf(kWindowWidth / 2 - 80, kWindowHeight / 2 + 20, "PRESS SPACE TO START");
			break;

		case GAMEPLAY:
		{		
			if (keys[DIK_W]) {
				player.pos.y -= player.speed;
				for (int o = 0; o < 15; o++) {
					if (map.objectBottomLeft[o].x < player.pos.x + player.width && map.objectBottomRight[o].x > player.pos.x) {
						if (map.objectBottomLeft[o].y - 20 < player.pos.y && map.objectBottomRight[o].y - 20 < player.pos.y) {
							if (map.objectBottomLeft[o].y > player.pos.y && map.objectBottomRight[o].y > player.pos.y) {
								player.pos.y += player.speed;
							}
						}
					}
				}
			}
			if (keys[DIK_S]) {
				player.pos.y += player.speed;

				for (int o = 0; o < 15; o++) {
					if (map.objectTopLeft[o].x < player.pos.x + player.width && map.objectTopRight[o].x > player.pos.x) {
						if (map.objectTopLeft[o].y + 20 > player.pos.y + player.height && map.objectTopRight[o].y + 20 > player.pos.y + player.height) {//+20は当たり判定の厚み
							if (map.objectTopLeft[o].y < player.pos.y + player.height && map.objectTopRight[o].y < player.pos.y + player.height) {
								player.pos.y -= player.speed;
							}
						}
					}
				}
			}
			if (keys[DIK_A]) {
				player.pos.x -= player.speed;

				for (int o = 0; o < 15; o++) {
					if (map.objectTopRight[o].y - player.width / 2 < player.pos.y && map.objectBottomRight[o].y > player.pos.y) {
						if (map.objectTopRight[o].x - 20 < player.pos.x && map.objectBottomRight[o].x - 20 < player.pos.x) {
							if (map.objectTopRight[o].x > player.pos.x && map.objectBottomRight[o].x > player.pos.x) {
								player.pos.x += player.speed;
							}
						}
					}
				}
			}
			if (keys[DIK_D]) {
				player.pos.x += player.speed;
				for (int o = 0; o < 15; o++) {
					if (map.objectTopLeft[o].y - player.width / 2 < player.pos.y && map.objectBottomLeft[o].y - player.width / 2 > player.pos.y - player.height / 2) {
						if (map.objectTopLeft[o].x - player.width / 2 + 20 > player.pos.x && map.objectBottomLeft[o].x - player.width / 2 + 20 > player.pos.x) {
							if (map.objectTopLeft[o].x - player.width < player.pos.x && map.objectBottomLeft[o].x - player.width < player.pos.x) {
								player.pos.x -= player.speed;
							}
						}
					}
				}
			}

			// --- スクロール処理 ---
			if (player.pos.x - world.x < scrollMarginX) {
				world.x = player.pos.x - scrollMarginX;
			}
			else if (player.pos.x - world.x > kWindowWidth - scrollMarginX) {
				world.x = player.pos.x - (kWindowWidth - scrollMarginX);
			}

			if (player.pos.y - world.y < scrollMarginY) {
				world.y = player.pos.y - scrollMarginY;
			}
			else if (player.pos.y - world.y > kWindowHeight - scrollMarginY) {
				world.y = player.pos.y - (kWindowHeight - scrollMarginY);
			}

			if (world.x < 0) world.x = 0;
			if (world.y < 0) world.y = 0;
			if (world.x > kStageWidth - kWindowWidth) world.x = kStageWidth - kWindowWidth;
			if (world.y > kStageHeight - kWindowHeight) world.y = kStageHeight - kWindowHeight;

			// 被ダメージ時のシェイク処理
			if (shakeTimer > 0) {
				shakeTimer--;
				world.x += (rand() % 11 - 5); // -5 ~ +5
				world.y += (rand() % 11 - 5);
			}


			// --- 武器切り替え ---
			if (preKeys[DIK_J] == 0 && keys[DIK_J] != 0) {
				int nextWeapon = currentWeapon;
				while (true) {
					nextWeapon++;
					if (nextWeapon > 2) nextWeapon = 0;
					if (nextWeapon == 0) { currentWeapon = 0; break; }
					if (nextWeapon == 1 && rifleUnlocked) { currentWeapon = 1; break; }
					if (nextWeapon == 2 && launcherUnlocked) { currentWeapon = 2; break; }
				}
			}

			// --- 攻撃処理 ---
			if (currentWeapon == 0) { 
				if (Novice::IsTriggerMouse(0)) {
					if (!isReloading && pistolAmmo > 0) {
						for (int i = 0; i < pistolMaxAmmo; i++) {
							if (!pistolBullets[i].isActive) {
								pistolAmmo--;
								pistolBullets[i].isActive = true;
								pistolBullets[i].pos = { player.pos.x + player.width / 2, player.pos.y + player.height / 2 };
								float dx = (mouseX + world.x) - player.pos.x;
								float dy = (mouseY + world.y) - player.pos.y;
								float len = sqrtf(dx * dx + dy * dy);
								if (len != 0) pistolBullets[i].velocity = { (dx / len) * 10.0f, (dy / len) * 10.0f };
								break;
							}
						}
					}
				}
				if (keys[DIK_R] && !isReloading && pistolAmmo < pistolMaxAmmo) {
					isReloading = true;
					reloadTime = reloadDuration;
				}
				if (isReloading) {
					reloadTime--;
					if (reloadTime <= 0) {
						isReloading = false;
						pistolAmmo = pistolMaxAmmo;
					}
				}
				for (int i = 0; i < pistolMaxAmmo; i++) {
					if (pistolBullets[i].isActive) {
						pistolBullets[i].pos.x += pistolBullets[i].velocity.x;
						pistolBullets[i].pos.y += pistolBullets[i].velocity.y;
						if (pistolBullets[i].pos.x < 0 || pistolBullets[i].pos.x > kStageWidth ||
							pistolBullets[i].pos.y < 0 || pistolBullets[i].pos.y > kStageHeight) {
							pistolBullets[i].isActive = false;
						}
					}
				}
			}
			else if (currentWeapon == 1) { // Rifle
				rifleFireTimer++;
				if (Novice::IsPressMouse(0)) {
					if (!isReloading && riffleAmmo > 0 && rifleFireTimer >= rifleFireRate) {
						rifleFireTimer = 0;
						for (int i = 0; i < riffleMaxAmmo; i++) {
							if (!riffleBullets[i].isActive) {
								riffleAmmo--;
								riffleBullets[i].isActive = true;
								riffleBullets[i].pos = { player.pos.x + player.width / 2, player.pos.y + player.height / 2 };
								float dx = (mouseX + world.x) - player.pos.x;
								float dy = (mouseY + world.y) - player.pos.y;
								float len = sqrtf(dx * dx + dy * dy);
								if (len != 0) riffleBullets[i].velocity = { (dx / len) * 15.0f, (dy / len) * 15.0f };
								break;
							}
						}
					}
				}
				if (keys[DIK_R] && !isReloading && riffleAmmo < riffleMaxAmmo) {
					isReloading = true;
					reloadTime = reloadDuration;
				}
				if (isReloading) {
					reloadTime--;
					if (reloadTime <= 0) {
						isReloading = false;
						riffleAmmo = riffleMaxAmmo;
					}
				}
				for (int i = 0; i < riffleMaxAmmo; i++) {
					if (riffleBullets[i].isActive) {
						riffleBullets[i].pos.x += riffleBullets[i].velocity.x;
						riffleBullets[i].pos.y += riffleBullets[i].velocity.y;
						if (riffleBullets[i].pos.x < 0 || riffleBullets[i].pos.x > kStageWidth ||
							riffleBullets[i].pos.y < 0 || riffleBullets[i].pos.y > kStageHeight) {
							riffleBullets[i].isActive = false;
						}
					}
				}
			}
			else if (currentWeapon == 2) { // Launcher
				if (launcherFireCooldown > 0) launcherFireCooldown--;
				if (Novice::IsTriggerMouse(0)) {
					if (!isLauncherReloading && launcherAmmo > 0 && launcherFireCooldown <= 0) {
						for (int i = 0; i < launcherMaxAmmo; i++) {
							if (!launcherBullets[i].isActive) {
								launcherAmmo--;
								launcherBullets[i].isActive = true;
								launcherBullets[i].pos = { player.pos.x + player.width / 2, player.pos.y + player.height / 2 };
								float dx = (mouseX + world.x) - player.pos.x;
								float dy = (mouseY + world.y) - player.pos.y;
								float len = sqrtf(dx * dx + dy * dy);
								if (len != 0) launcherBullets[i].velocity = { (dx / len) * 8.0f, (dy / len) * 8.0f };
								launcherFireCooldown = launcherFireInterval;
								break;
							}
						}
					}
				}
				if (keys[DIK_R] && !isLauncherReloading && launcherAmmo < launcherMaxAmmo) {
					isLauncherReloading = true;
					launcherReloadTime = launcherReloadDuration;
				}
				if (isLauncherReloading) {
					launcherReloadTime--;
					if (launcherReloadTime <= 0) {
						isLauncherReloading = false;
						launcherAmmo = launcherMaxAmmo;
					}
				}
				for (int i = 0; i < launcherMaxAmmo; i++) {
					if (launcherBullets[i].isActive) {
						launcherBullets[i].pos.x += launcherBullets[i].velocity.x;
						launcherBullets[i].pos.y += launcherBullets[i].velocity.y;
						if (launcherBullets[i].pos.x < 0 || launcherBullets[i].pos.x > kStageWidth ||
							launcherBullets[i].pos.y < 0 || launcherBullets[i].pos.y > kStageHeight) {
							launcherBullets[i].isActive = false;
						}
					}
				}
			}

			//  Player当たり判定 
			for (auto& b : bossBullets) {
				if (b.isActive && player.invincibilityTimer <= 0) {
					float dx = b.pos.x - (player.pos.x + player.width / 2);
					float dy = b.pos.y - (player.pos.y + player.height / 2);
					float dist = sqrtf(dx * dx + dy * dy);

					if (dist < 15.0f) {
						player.hp--;
						player.invincibilityTimer = 60;
						b.isActive = false;
						shakeTimer = 15; 
					}
				}
			}
			if (player.invincibilityTimer > 0) player.invincibilityTimer--;

			// --- 武器 ---
			float dxR = player.pos.x - rifleBoxPos.x;
			float dyR = player.pos.y - rifleBoxPos.y;
			if (sqrtf(dxR * dxR + dyR * dyR) < 30.0f) {
				rifleBoxPos = { -100.0f, -100.0f };
				rifleUnlocked = true;
			}
			float dxL = player.pos.x - launcherBoxPos.x;
			float dyL = player.pos.y - launcherBoxPos.y;
			if (sqrtf(dxL * dxL + dyL * dyL) < 30.0f) {
				launcherBoxPos = { -100.0f, -100.0f };
				launcherUnlocked = true;
			}

			// --- ダッシュ ---
			if (player.dashCooldown > 0) player.dashCooldown--;
			if (player.dashDuration > 0) {
				player.dashDuration--;
				if (player.dashDuration == 0) player.isDashing = false;
			}
			if (keys[DIK_V] && player.dashCooldown == 0 && !player.isDashing) {
				player.isDashing = true;
				player.dashDuration = 15;
				player.dashCooldown = 60;
			}
			if (player.isDashing) {
				if (keys[DIK_W]) player.pos.y -= player.dashSpeed;
				if (keys[DIK_S]) player.pos.y += player.dashSpeed;
				if (keys[DIK_A]) player.pos.x -= player.dashSpeed;
				if (keys[DIK_D]) player.pos.x += player.dashSpeed;
			}

			// ステージ外制限
			if (player.pos.x < 0) player.pos.x = 0;
			if (player.pos.y < 0) player.pos.y = 0;
			if (player.pos.x > kStageWidth - player.width) player.pos.x = kStageWidth - player.width;
			if (player.pos.y > kStageHeight - player.height) player.pos.y = kStageHeight - player.height;

			// --- Boss Logic ---
			if (boss.hp <= 70 && boss.phase == 1) boss.phase = 2;
			if (boss.hp <= 40 && boss.phase == 2) boss.phase = 3;

			boss.attackTimer++;
			int interval = (boss.phase == 1) ? 120 : (boss.phase == 2) ? 80 : 40;
			int shotCount = (boss.phase == 1) ? 1 : (boss.phase == 2) ? 2 : 3;

			if (boss.attackTimer >= interval) {
				for (int i = 0; i < shotCount; i++) {
					Vector2 dir = { player.pos.x - boss.pos.x, player.pos.y - boss.pos.y };
					float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
					if (length != 0) {
						dir.x /= length;
						dir.y /= length;
						BossBullet b;
						b.pos = boss.pos;
						b.velocity = { dir.x * 5.0f, dir.y * 5.0f };
						b.isActive = true;
						bossBullets.push_back(b);
					}
				}
				boss.attackTimer = 0;
			}
			for (auto& b : bossBullets) {
				if (b.isActive) {
					b.pos.x += b.velocity.x;
					b.pos.y += b.velocity.y;
					if (b.pos.x < 0 || b.pos.x > kStageWidth || b.pos.y < 0 || b.pos.y > kStageHeight) b.isActive = false;
				}
			}

			// --- Player Bullets vs Boss ---
			for (int i = 0; i < pistolMaxAmmo; i++) {
				if (pistolBullets[i].isActive) {
					float d = sqrtf(powf(pistolBullets[i].pos.x - boss.pos.x, 2) + powf(pistolBullets[i].pos.y - boss.pos.y, 2));
					if (d < 25.0f) {
						pistolBullets[i].isActive = false;
						boss.hp -= 5;
					}
				}
			}
			for (int i = 0; i < riffleMaxAmmo; i++) {
				if (riffleBullets[i].isActive) {
					float d = sqrtf(powf(riffleBullets[i].pos.x - boss.pos.x, 2) + powf(riffleBullets[i].pos.y - boss.pos.y, 2));
					if (d < 25.0f) {
						riffleBullets[i].isActive = false;
						boss.hp -= 5;
					}
				}
			}
			for (int i = 0; i < launcherMaxAmmo; i++) {
				if (launcherBullets[i].isActive) {
					float d = sqrtf(powf(launcherBullets[i].pos.x - boss.pos.x, 2) + powf(launcherBullets[i].pos.y - boss.pos.y, 2));
					if (d < 40.0f) {
						launcherBullets[i].isActive = false;
						boss.hp -= 20;
					}
				}
			}
			if (boss.hp < 0) boss.hp = 0;
		}


			ImGui::Begin("MAP");
			ImGui::SliderFloat("world map x", &world.x, -kWindowWidth, kWindowWidth);
			ImGui::SliderFloat("world map y", &world.y, -kWindowHeight, kWindowHeight);
			ImGui::End();

			// 背景
			for (int i = -1; i < 2; i++) {
				for (int k = -1; k < 2; k++) {
					Novice::DrawSprite(-(int)world.x + kWindowWidth * k, -(int)world.y + kWindowHeight * i, sprite[0], 1, 1, 0.0f, WHITE);
				}
			}
			// オブジェクト
			for (int i = 0; i < 15; i++) {
				Novice::DrawQuad(
					(int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y - (int)world.y,
					(int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y - (int)world.y,
					(int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y - (int)world.y,
					(int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y - (int)world.y,
					0, 0, 81, 128, sprite[1], WHITE
				);
			}

			// Player描画 
			if (player.invincibilityTimer <= 0 || (player.invincibilityTimer / 5) % 2 == 0) {
				unsigned int color = (player.invincibilityTimer > 0) ? 0xFFAAAAFF : 0xFFFFFFFF;
				Novice::DrawBox(
					(int)(player.pos.x - world.x), (int)(player.pos.y - world.y),
					(int)player.width, (int)player.height, 0.0f, color, kFillModeSolid
				);
			}

			// Aim Line
			Novice::DrawLine((int)player.pos.x - (int)world.x + (int)player.width / 2, (int)player.pos.y - (int)world.y + (int)player.height / 2, mouseX, mouseY, WHITE);

			// Weapon Boxes
         	// ライフルボックス 
			Novice::DrawQuad(
				(int)(rifleBoxPos.x - world.x), (int)(rifleBoxPos.y - world.y),
				(int)(rifleBoxPos.x - world.x) + boxW, (int)(rifleBoxPos.y - world.y),
				(int)(rifleBoxPos.x - world.x), (int)(rifleBoxPos.y - world.y) + boxH,
				(int)(rifleBoxPos.x - world.x) + boxW, (int)(rifleBoxPos.y - world.y) + boxH,
				0, 0, 81, 128, 
				sprite[2],
				WHITE
			);

			// ランチャーボックス 
			Novice::DrawQuad(
				(int)(launcherBoxPos.x - world.x), (int)(launcherBoxPos.y - world.y),
				(int)(launcherBoxPos.x - world.x) + boxW, (int)(launcherBoxPos.y - world.y),
				(int)(launcherBoxPos.x - world.x), (int)(launcherBoxPos.y - world.y) + boxH,
				(int)(launcherBoxPos.x - world.x) + boxW, (int)(launcherBoxPos.y - world.y) + boxH,
				0, 0, 81, 128, 
				sprite[2],
				WHITE
			);
			// Bullets
			if (currentWeapon == 0) {
				for (int i = 0; i < pistolMaxAmmo; i++) if (pistolBullets[i].isActive) Novice::DrawEllipse((int)pistolBullets[i].pos.x - (int)world.x, (int)pistolBullets[i].pos.y - (int)world.y, 5, 5, 0.0f, BLUE, kFillModeSolid);
			}
			else if (currentWeapon == 1) {
				for (int i = 0; i < riffleMaxAmmo; i++) if (riffleBullets[i].isActive) Novice::DrawEllipse((int)riffleBullets[i].pos.x - (int)world.x, (int)riffleBullets[i].pos.y - (int)world.y, 5, 5, 0.0f, GREEN, kFillModeSolid);
			}
			else if (currentWeapon == 2) {
				for (int i = 0; i < launcherMaxAmmo; i++) if (launcherBullets[i].isActive) Novice::DrawEllipse((int)launcherBullets[i].pos.x - (int)world.x, (int)launcherBullets[i].pos.y - (int)world.y, 8, 8, 0.0f, RED, kFillModeSolid);
			}

			Novice::DrawEllipse(mouseX, mouseY, 4, 4, 0.0f, RED, kFillModeSolid);

			// Boss
			Novice::DrawEllipse((int)boss.pos.x - (int)world.x, (int)boss.pos.y - (int)world.y, 50, 50, 0.0f, RED, kFillModeSolid);
			// Boss Bullets
			for (auto& b : bossBullets) if (b.isActive) Novice::DrawEllipse((int)b.pos.x - (int)world.x, (int)b.pos.y - (int)world.y, 5, 5, 0.0f, RED, kFillModeSolid);

			// UI
			Novice::DrawBox(900, 50, boss.hp * 2, 20, 0.0f, GREEN, kFillModeSolid); // Boss HP
			Novice::ScreenPrintf(900, 30, "BOSS HP");

			Novice::DrawBox(50, 50, player.hp * 20, 20, 0.0f, BLUE, kFillModeSolid); // Player HP
			Novice::ScreenPrintf(50, 30, "PLAYER HP: %d", player.hp);

			if (currentWeapon == 0) weaponName = "Pistol";
			else if (currentWeapon == 1) weaponName = "Rifle";
			else if (currentWeapon == 2) weaponName = "Launcher";

			Novice::ScreenPrintf(0, 100, "Current Weapon: %s", weaponName);
			Novice::ScreenPrintf(0, 120, "Ammo: %d", (currentWeapon == 0) ? pistolAmmo : (currentWeapon == 1) ? riffleAmmo : launcherAmmo);

			// シーン遷移判定
			if (player.hp <= 0) {
				currentScene = GAMEOVER;
			}
			if (boss.hp <= 0) {
				currentScene = GAMECLEAR;
			}

			break;

		case GAMECLEAR:
			Novice::ScreenPrintf(kWindowWidth / 2 - 50, kWindowHeight / 2 - 20, "GAME CLEAR!!");
			Novice::ScreenPrintf(kWindowWidth / 2 - 80, kWindowHeight / 2 + 20, "PRESS SPACE TO TITLE");
			if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
				currentScene = TITLE;
			}
			break;

		case GAMEOVER:
			Novice::DrawBox(0, 0, kWindowWidth, kWindowHeight, 0.0f, 0x00000080, kFillModeSolid); // 半透明黒
			Novice::ScreenPrintf(kWindowWidth / 2 - 50, kWindowHeight / 2 - 20, "GAME OVER...");
			Novice::ScreenPrintf(kWindowWidth / 2 - 80, kWindowHeight / 2 + 20, "PRESS SPACE TO TITLE");
			if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
				currentScene = TITLE;
			}
			break;
		}

		///
		/// ↑更新処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}