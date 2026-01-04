#include <Novice.h>
#include <corecrt_math.h>
#include <vector>
#include <time.h> 
#define _USE_MATH_DEFINES
#ifdef USE_IMGUI
#include"imgui.h"
#endif

const char kWindowTitle[] = "GC1C_10_ミャッ_フォン_マウン";

const int kWindowWidth = 1280;
const int kWindowHeight = 720;

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

//map
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

	int worldWidth = kWindowWidth * 2;
	int worldHeight = kWindowHeight * 2;
};

Vector2 world = { 0,0 };

/////
const int screenWidth = kWindowWidth;
const int screenHeight = kWindowHeight;
const int scrollMarginX = 400; // 画面端からスクロール開始する距離
const int scrollMarginY = 300;

//player
struct Player {
	Vector2 pos;
	Vector2 screenPos;
	float width;
	float height;
	float speed;
	int dashCooldown;
	int dashDuration;
	int dashSpeed;
	int hp;
	float angle;
	bool isDashing;
	int invincibilityTimer;
};
struct PlayerBullet {
	Vector2 pos;
	Vector2 velocity;
	Vector2 startPos;
	bool isActive;
	float maxRange;
};

struct BossBullet {
	Vector2 pos;
	Vector2 velocity;
	bool isActive;
};

struct Boss {
	Vector2 pos;
	int hp;
	int radius;
	int phase;
	int attackTimer;
};

int mouseX = 0;
int mouseY = 0;

int shakeTimer = 0;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);


	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	Map map = {};

	/*int boxW = 64;
	int boxH = 64;*/
	int sprite[10] = {

		Novice::LoadTexture("./Sprite/map_background.png"),
		Novice::LoadTexture("./Sprite/object_wood.png"),


	};

	int TrassureBox = Novice::LoadTexture("./Sprite/item.png");
	int PlayerGh = Novice::LoadTexture("./Sprite/player.png");
	int BossGh = Novice::LoadTexture("./Sprite/boss.png");
	int titleGh = Novice::LoadTexture("./Sprite/title.png");
	int gameClearGh = Novice::LoadTexture("./Sprite/clear.png");
	int gameOverGh = Novice::LoadTexture("./Sprite/gameover.png");
	int ReloadGh = Novice::LoadTexture("./Sprite/Reloading.png");
	//int Weapon = Novice::LoadTexture("./Sprite/weapon.png");
	int RifleGh = Novice::LoadTexture("./Sprite/rifle.png");
	int LauncherGh = Novice::LoadTexture("./Sprite/launcher.png");
	int PistolGh = Novice::LoadTexture("./Sprite/pistol.png");

	map.objectTopLeft[0].x = 350;
	map.objectTopLeft[0].y = 350;
	map.objectTopRight[0].x = 440 + 81;
	map.objectTopRight[0].y = 350;
	map.objectBottomLeft[0].x = 350;
	map.objectBottomLeft[0].y = 440 + 128 * 1.5;
	map.objectBottomRight[0].x = 440 + 81;
	map.objectBottomRight[0].y = 440 + 128 * 1.5;

	map.objectTopLeft[1].x = 850;
	map.objectTopLeft[1].y = 850;
	map.objectTopRight[1].x = 940 + 81;
	map.objectTopRight[1].y = 850;
	map.objectBottomLeft[1].x = 850;
	map.objectBottomLeft[1].y = 940 + 128 * 1.5;
	map.objectBottomRight[1].x = 940 + 81;
	map.objectBottomRight[1].y = 940 + 128 * 1.5;



	Player player = {
		.pos = {64.0f, 640.0f},
		.screenPos = {0,0},
		.width = 32.0f,
		.height = 32.0f,
		.speed = 4.0f,
		.dashCooldown = 0,
		.dashDuration = 0,
		.dashSpeed = 5,
		.hp = 20,
		.angle = 0.0f,
		.isDashing = false,
		.invincibilityTimer = 0

	};

	Boss boss = { {600.0f, 100.0f}, 300,50, 1, 0 };
	std::vector<BossBullet> bossBullets;
	//int bossBulletsRadius = 5;

	// pistol's bullets(revolver type)

	int pistolAmmo = 6;
	int pistolMaxAmmo = 6;
	int pistolRadius = 5;
	float pistolRange = 600.0f;
	float pistolBulletSpeed = 10.0f;
	bool isReloading = false;
	int reloadTime = 0;
	const int reloadDuration = 90;
	const int pistolBulletsMax = 6;
	PlayerBullet pistolBullets[pistolBulletsMax] = {};

	// riffle's bullets(automatic type)
	int riffleAmmo = 30;
	int riffleMaxAmmo = 30;
	int riffleRadius = 3;
	float rifleRange = 800.0f;
	float rifleBulletSpeed = 15.0f;
	int rifleFireRate = 5;     // shoot every 5 frames
	int rifleFireTimer = 0;
	const int riffleBulletsMax = 30;
	PlayerBullet riffleBullets[riffleBulletsMax] = {};

	// launcher's bullets (grenade type) - 未実装
	int launcherAmmo = 5;
	int launcherMaxAmmo = 5;
	int launcherRadius = 8;
	float launcherRange = 500.0f;
	float launcherBulletSpeed = 8.0f;
	bool isLauncherReloading = false;
	int launcherReloadTime = 0;
	int launcherFireCooldown = 0;
	const int launcherFireInterval = 48;
	const int launcherReloadDuration = 120;
	const int launcherBulletsMax = 5;
	PlayerBullet launcherBullets[launcherBulletsMax] = {};

	//武器を切り替え
	int currentWeapon = 0;
	const char* weaponName = "Unknown";
	//Weapon box

	Vector2 rifleBoxPos = { 400.0f, 300.0f };
	Vector2 launcherBoxPos = { 800.0f, 500.0f };
	bool rifleUnlocked = false;
	bool launcherUnlocked = false;

	shakeTimer = 0;
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
		/// ↓更新処理ここから
		///
		switch (currentScene)
		{
		case TITLE:
			// タイトル画面の処理

			if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
				player.pos = { 64.0f, 640.0f };
				player.hp = 20;
				boss.pos = { 600.0f, 300.0f };
				boss.hp = 300;
				player.isDashing = false;
				currentWeapon = 0;
				rifleUnlocked = false;
				launcherUnlocked = false;
				player.dashSpeed = 5;
				player.dashCooldown = 0;

				PlayerBullet emptyBullet = {};
				for (int i = 0; i < pistolBulletsMax; i++) { pistolBullets[i] = emptyBullet; }
				for (int i = 0; i < riffleBulletsMax; i++) { riffleBullets[i] = emptyBullet; }

				bossBullets.clear();

				pistolAmmo = pistolMaxAmmo;
				riffleAmmo = riffleMaxAmmo;
				launcherAmmo = launcherMaxAmmo;

				rifleBoxPos = { 400.0f, 300.0f };
				launcherBoxPos = { 800.0f, 500.0f };

				currentScene = GAMEPLAY;
			}

			// タイトル描画
			Novice::DrawSprite(0, 0, titleGh, 1.0f, 1.0f, 0.0f, WHITE);
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

			float playerCenterX = player.pos.x + player.width / 2.0f;
			float playerCenterY = player.pos.y + player.height / 2.0f;

			// マウス位置（ワールド座標に変換済み）
			float mouseWorldX = mouseX + world.x;
			float mouseWorldY = mouseY + world.y;

			// プレイヤー -> マウス の方向ベクトル
			float dxA = mouseWorldX - playerCenterX;
			float dyA = mouseWorldY - playerCenterY;

			// 角度を求める（ラジアン）
			player.angle = atan2f(dyA, dxA);

			// 横スクロール
			if (player.pos.x - world.x < scrollMarginX) {
				world.x = player.pos.x - scrollMarginX;
			}
			else if (player.pos.x - world.x > screenWidth - scrollMarginX) {
				world.x = player.pos.x - (screenWidth - scrollMarginX);
			}

			// 縦スクロール
			if (player.pos.y - world.y < scrollMarginY) {
				world.y = player.pos.y - scrollMarginY;
			}
			else if (player.pos.y - world.y > screenHeight - scrollMarginY) {
				world.y = player.pos.y - (screenHeight - scrollMarginY);
			}

			// ワールド座標の制限
			if (player.pos.x < -kWindowWidth) {
				player.pos.x = -kWindowWidth;
			}
			if (player.pos.y < -kWindowHeight) {
				player.pos.y = -kWindowHeight;
			}
			if (player.pos.x + player.width > map.worldWidth) {
				player.pos.x = map.worldWidth - player.width;
			}
			if (player.pos.y + player.height > map.worldHeight) {
				player.pos.y = map.worldHeight - player.height;
			}

			// カメラのワールド座標の制限
			if (world.x < -kWindowWidth) {
				world.x = -kWindowWidth;
			}
			if (world.y < -kWindowHeight) {
				world.y = -kWindowHeight;
			}
			if (world.x > map.worldWidth - screenWidth) {
				world.x = map.worldWidth - (float)screenWidth;
			}
			if (world.y > map.worldHeight - screenHeight) {
				world.y = map.worldHeight - (float)screenHeight;
			}

			if (preKeys[DIK_J] == 0 && keys[DIK_J] != 0) {
				int nextWeapon = currentWeapon;
				while (true) {
					nextWeapon++;
					if (nextWeapon > 2) nextWeapon = 0;

					// pistol はいつでも使える
					if (nextWeapon == 0) {
						currentWeapon = 0;
						break;
					}
					// rifle はアンロックされているときだけ
					if (nextWeapon == 1 && rifleUnlocked) {
						currentWeapon = 1;
						break;
					}
					// launcher はアンロックされているときだけ
					if (nextWeapon == 2 && launcherUnlocked) {
						currentWeapon = 2;
						break;
					}
				}
			}
			if (currentWeapon == 0) {
				//pistol shoot
				if (Novice::IsTriggerMouse(0)) {
					if (!isReloading && pistolAmmo > 0) {
						for (int i = 0; i < pistolBulletsMax; i++) {
							if (!pistolBullets[i].isActive) {
								pistolAmmo--;
								pistolBullets[i].isActive = true;
								pistolBullets[i].pos = {
									player.pos.x + player.width / 2,
									player.pos.y + player.height / 2
								};
								pistolBullets[i].startPos = {
									player.pos.x + player.width / 2,
									player.pos.y + player.height / 2
								};
								pistolBullets[i].maxRange = pistolRange;

								// プレイヤーからマウスへの方向ベクトルを正規化
								float dx = mouseWorldX - player.pos.x;
								float dy = mouseWorldY - player.pos.y;
								float len = sqrtf(dx * dx + dy * dy);
								if (len != 0) {
									pistolBullets[i].velocity = { (dx / len) * pistolBulletSpeed, (dy / len) * pistolBulletSpeed };
								}

								break;
							}
						}
					}
				}
				//reload
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
				for (int i = 0; i < pistolBulletsMax; i++) {
					if (pistolBullets[i].isActive) {
						pistolBullets[i].pos.x += pistolBullets[i].velocity.x;
						pistolBullets[i].pos.y += pistolBullets[i].velocity.y;

						float dx = pistolBullets[i].pos.x - pistolBullets[i].startPos.x;
						float dy = pistolBullets[i].pos.y - pistolBullets[i].startPos.y;
						float distance = sqrtf(dx * dx + dy * dy);

						// 射程を超えたら消す
						if (distance >= pistolBullets[i].maxRange) {
							pistolBullets[i].isActive = false;
						}
					}
				}
			}
			else if (currentWeapon == 1) {

				//riffle shoot

				rifleFireTimer++;
				if (Novice::IsPressMouse(0)) {
					if (!isReloading && riffleAmmo > 0 && rifleFireTimer >= rifleFireRate) {
						rifleFireTimer = 0;
						for (int i = 0; i < riffleBulletsMax; i++) {
							if (!riffleBullets[i].isActive) {
								riffleAmmo--;
								riffleBullets[i].isActive = true;
								riffleBullets[i].pos = {
									player.pos.x + player.width / 2,
									player.pos.y + player.height / 2
								};
								riffleBullets[i].startPos = {
									player.pos.x + player.width / 2,
									player.pos.y + player.height / 2
								};
								riffleBullets[i].maxRange = rifleRange;

								// プレイヤーからマウスへの方向ベクトルを正規化
								float dx = mouseWorldX - player.pos.x;
								float dy = mouseWorldY - player.pos.y;
								float len = sqrtf(dx * dx + dy * dy);
								if (len != 0) {
									riffleBullets[i].velocity = { (dx / len) * rifleBulletSpeed, (dy / len) * rifleBulletSpeed };
								}
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

				for (int i = 0; i < riffleBulletsMax; i++) {
					if (riffleBullets[i].isActive) {
						riffleBullets[i].pos.x += riffleBullets[i].velocity.x;
						riffleBullets[i].pos.y += riffleBullets[i].velocity.y;

						float dx = riffleBullets[i].pos.x - riffleBullets[i].startPos.x;
						float dy = riffleBullets[i].pos.y - riffleBullets[i].startPos.y;
						float distance = sqrtf(dx * dx + dy * dy);
						// 射程を超えたら消す
						if (distance >= riffleBullets[i].maxRange) {
							riffleBullets[i].isActive = false;
						}
					}
				}
			}

			else if (currentWeapon == 2) {
				// launcher shoot
				if (launcherFireCooldown > 0) {
					launcherFireCooldown--;
				}
				if (Novice::IsTriggerMouse(0)) {
					if (!isLauncherReloading && launcherAmmo > 0 && launcherFireCooldown <= 0) {
						for (int i = 0; i < 5; i++) {
							if (!launcherBullets[i].isActive) {
								launcherAmmo--;
								launcherBullets[i].isActive = true;
								launcherBullets[i].pos = {
									player.pos.x + player.width / 2,
									player.pos.y + player.height / 2
								};
								launcherBullets[i].startPos = {
									player.pos.x + player.width / 2,
									player.pos.y + player.height / 2
								};
								launcherBullets[i].maxRange = launcherRange;
								// プレイヤーからマウスへの方向ベクトルを正規化
								float dx = mouseWorldX - player.pos.x;
								float dy = mouseWorldY - player.pos.y;
								float len = sqrtf(dx * dx + dy * dy);
								if (len != 0) {
									launcherBullets[i].velocity = { (dx / len) * launcherBulletSpeed, (dy / len) * launcherBulletSpeed };
								}
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
				for (int i = 0; i < 5; i++) {
					if (launcherBullets[i].isActive) {
						launcherBullets[i].pos.x += launcherBullets[i].velocity.x;
						launcherBullets[i].pos.y += launcherBullets[i].velocity.y;

						float dx = launcherBullets[i].pos.x - launcherBullets[i].startPos.x;
						float dy = launcherBullets[i].pos.y - launcherBullets[i].startPos.y;
						float distance = sqrtf(dx * dx + dy * dy);
						// 射程を超えたら消す
						if (distance >= launcherBullets[i].maxRange) {
							launcherBullets[i].isActive = false;
						}
					}
				}
			}


			// Rifle weapon box
			if (!rifleUnlocked) {
				float dxR = player.pos.x - rifleBoxPos.x;
				float dyR = player.pos.y - rifleBoxPos.y;
				float disR = sqrtf(dxR * dxR + dyR * dyR);

				if (disR < 30.0f) {
					rifleUnlocked = true;

					// 取られたら画面外へ移動して終了
					rifleBoxPos.x = -200.0f;
					rifleBoxPos.y = -200.0f;
				}
			}


			// Launcher weapon box
			if (!launcherUnlocked) {
				float dxL = player.pos.x - launcherBoxPos.x;
				float dyL = player.pos.y - launcherBoxPos.y;
				float distL = sqrtf(dxL * dxL + dyL * dyL);

				if (distL < 30.0f) {
					launcherUnlocked = true;

					// 取られたら画面外へ移動して終了
					launcherBoxPos.x = -200.0f;
					launcherBoxPos.y = -200.0f;
				}
			}
			/*if (!rifleUnlocked) {
				rifleBoxPos.x = rand() % 1200 + 40.0f;
				rifleBoxPos.y = rand() % 640 + 40.0f;
			}

			if (!launcherUnlocked) {
				launcherBoxPos.x = rand() % 1200 + 40.0f;
				launcherBoxPos.y = rand() % 640 + 40.0f;
			}*/

			//dash

			if (player.dashCooldown > 0) {
				player.dashCooldown--;
			}
			if (player.dashDuration > 0) {
				player.dashDuration--;
				if (player.dashDuration == 0) {
					player.isDashing = false;
				}
			}
			if (keys[DIK_V] && player.dashCooldown == 0 && !player.isDashing) {
				player.isDashing = true;
				player.dashDuration = 15;
				player.dashCooldown = 180;
			}

			if (player.isDashing) {
				if (keys[DIK_W]) player.pos.y -= player.dashSpeed;
				if (keys[DIK_S]) player.pos.y += player.dashSpeed;
				if (keys[DIK_A]) player.pos.x -= player.dashSpeed;
				if (keys[DIK_D]) player.pos.x += player.dashSpeed;
			}

			//Boss
			if (boss.hp <= 70 && boss.phase == 1) {
				boss.phase = 2;
			}
			if (boss.hp <= 40 && boss.phase == 2) {
				boss.phase = 3;
			}

			boss.attackTimer++;

			int interval =
				(boss.phase == 1) ? 120 :
				(boss.phase == 2) ? 80 :
				40;

			int shotCount =
				(boss.phase == 1) ? 1 :
				(boss.phase == 2) ? 2 :
				3;

			float bulletSpeed =
				(boss.phase == 1) ? 6.0f :
				(boss.phase == 2) ? 8.0f :
				12.0f;
			if (boss.attackTimer >= interval) {
				Vector2 dir = {
		   player.pos.x - boss.pos.x,
		   player.pos.y - boss.pos.y
				};

				float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
				if (length == 0) length = 1;

				dir.x /= length;
				dir.y /= length;
				for (int i = 0; i < shotCount; i++) {
					float angleOffset = (i - (shotCount - 1) / 2.0f) * 0.20f;

					float cosA = cosf(angleOffset);
					float sinA = sinf(angleOffset);

					Vector2 rotated = {
						dir.x * cosA - dir.y * sinA,
						dir.x * sinA + dir.y * cosA
					};

					BossBullet b;
					b.pos = boss.pos;
					b.velocity = { rotated.x * bulletSpeed, rotated.y * bulletSpeed };
					b.isActive = true;

					bossBullets.push_back(b);
				}

				boss.attackTimer = 0;
			}
			for (auto& b : bossBullets) {
				if (b.isActive) {
					b.pos.x += b.velocity.x;
					b.pos.y += b.velocity.y;

				}
			}
			float bx = player.pos.x - boss.pos.x;
			float by = player.pos.y - boss.pos.y;
			float blen = sqrtf(bx * bx + by * by);

			// 0除算防止
			if (blen != 0) {
				bx /= blen;
				by /= blen;
			}

			// ボスの移動スピード（好きに調整） 
			float bossSpeed =
				(boss.phase == 1) ? 1.5f :
				(boss.phase == 2) ? 3.0f :
				4.0f;

			// プレイヤーの方へ移動
			boss.pos.x += bx * bossSpeed;
			boss.pos.y += by * bossSpeed;

			//  Player当たり判定 
			for (auto& b : bossBullets) {
				if (b.isActive && player.invincibilityTimer <= 0) {
					float dx = (b.pos.x + player.width / 4) - (player.pos.x + player.width / 2);
					float dy = (b.pos.y + player.width / 4) - (player.pos.y + player.height / 2);
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

			Vector2 prePlayerPos = player.pos;
			// player とボスの当たり判定 
			float playerLeft = player.pos.x;
			float playerRight = player.pos.x + player.width;
			float playerTop = player.pos.y;
			float playerBottom = player.pos.y + player.height;
			float closestX = max(playerLeft, min(boss.pos.x, playerRight));
			float closestY = max(playerTop, min(boss.pos.y, playerBottom));
			float dx = boss.pos.x - closestX;
			float dy = boss.pos.y - closestY;
			float dist = sqrtf(dx * dx + dy * dy);
			if (dist < boss.radius) {
				player.hp -= 1;
				float len = sqrtf(dx * dx + dy * dy);
				if (len != 0) {
					dx /= len; dy /= len;
				}
				float overlap = boss.radius - dist;
				player.pos.x -= dx * overlap;
				player.pos.y -= dy * overlap;
				float knockBackPower = 12.0f;
				player.pos.x -= dx * knockBackPower;
				player.pos.y -= dy * knockBackPower;
			}

			// pistolBulletとボスの当たり判定

			for (int i = 0; i < pistolBulletsMax; i++) {
				if (pistolBullets[i].isActive) {
					float pistolDx = (pistolBullets[i].pos.x) - (boss.pos.x);
					float pistolDy = (pistolBullets[i].pos.y) - (boss.pos.y);
					float pistolDistance = sqrtf(pistolDx * pistolDx + pistolDy * pistolDy);
					if (pistolDistance < (pistolRadius + boss.radius)) {
						pistolBullets[i].isActive = false;
						boss.hp -= 2;
						if (boss.hp < 0) {
							boss.hp = 0;
						}
					}
				}
			}
			// riffleBulletとボスの当たり判定
			for (int i = 0; i < riffleBulletsMax; i++) {
				if (riffleBullets[i].isActive) {
					float rifleDx = (riffleBullets[i].pos.x) - (boss.pos.x);
					float rifleDy = (riffleBullets[i].pos.y) - (boss.pos.y);
					float rifleDistance = sqrtf(rifleDx * rifleDx + rifleDy * rifleDy);
					if (rifleDistance < (riffleRadius + boss.radius)) {
						riffleBullets[i].isActive = false;
						boss.hp -= 1;
						if (boss.hp < 0) {
							boss.hp = 0;
						}
					}
				}
			}
			// launcherBullet とボスの当たり判定
			for (int i = 0; i < launcherBulletsMax; i++) {
				if (launcherBullets[i].isActive) {
					float launcherDx = launcherBullets[i].pos.x - boss.pos.x;
					float launcherDy = launcherBullets[i].pos.y - boss.pos.y;
					float launcherDistance = sqrtf(launcherDx * launcherDx + launcherDy * launcherDy);

					if (launcherDistance < (launcherRadius + boss.radius)) {
						launcherBullets[i].isActive = false;
						boss.hp -= 4;
						if (boss.hp < 0) boss.hp = 0;
					}
				}
			}
		}



		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		
		

		for (int i = -1; i < 2; i++) {
			for (int k = -1; k < 2; k++) {
				Novice::DrawSprite(-(int)world.x + kWindowWidth * k, -(int)world.y + kWindowHeight * i, sprite[0], 1, 1, 0.0f, WHITE);
			}
		}
		

		//オブジェクト
		for (int i = 0; i < 15; i++) {
			Novice::DrawQuad(
				(int)map.objectTopLeft[i].x - (int)world.x,
				(int)map.objectTopLeft[i].y - (int)world.y,
				(int)map.objectTopRight[i].x - (int)world.x,
				(int)map.objectTopRight[i].y - (int)world.y,
				(int)map.objectBottomLeft[i].x - (int)world.x,
				(int)map.objectBottomLeft[i].y - (int)world.y,
				(int)map.objectBottomRight[i].x - (int)world.x,
				(int)map.objectBottomRight[i].y - (int)world.y,
				0, 0, 81, 128,
				sprite[1],
				WHITE
			);
		}
		if (player.invincibilityTimer <= 0 || (player.invincibilityTimer / 5) % 2 == 0) {
			unsigned int color = (player.invincibilityTimer > 0) ? 0xFFAAAAFF : 0xFFFFFFFF;
			//Novice::DrawBox((int)(player.pos.x) - (int)world.x, (int)(player.pos.y) - (int)world.y, (int)player.width, (int)player.height, 0.0f, color, kFillModeWireFrame);
			Novice::DrawSprite((int)(player.pos.x+player.width/2) - int(world.x), (int)(player.pos.y+player.height/2) - int(world.y), PlayerGh, 1, 1, player.angle, color);
		}

		//aim line

		Novice::DrawLine((int)player.pos.x - (int)world.x + (int)player.width / 2, (int)player.pos.y - (int)world.y + (int)player.height / 2, mouseX, mouseY, WHITE);

		//マップオブジェクト当たり判定のデバッグ

		for (int i = 0; i < 15; i++) {
			Novice::DrawLine((int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y - (int)world.y, (int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y - (int)world.y, GREEN);
			Novice::DrawLine((int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y - (int)world.y, (int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y - (int)world.y, GREEN);
			Novice::DrawLine((int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y - (int)world.y, (int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y - (int)world.y, GREEN);
			Novice::DrawLine((int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y - (int)world.y, (int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y - (int)world.y, GREEN);
		}
		// Weapon Boxes
					// ライフルボックス 
		Novice::DrawSprite(
			(int)(rifleBoxPos.x - world.x),
			(int)(rifleBoxPos.y - world.y),
			TrassureBox,
			1.0f,
			1.0f,
			0.0f,
			WHITE
		);
		// ローンチャーボックス
		Novice::DrawSprite(
			(int)(launcherBoxPos.x - world.x),
			(int)(launcherBoxPos.y - world.y),
			TrassureBox,
			1.0f,
			1.0f,
			0.0f,
			WHITE
		);

		//武器を切り替え
		if (currentWeapon == 0) {
			for (int i = 0; i < pistolBulletsMax; i++) {
				if (pistolBullets[i].isActive) {
					Novice::DrawEllipse((int)pistolBullets[i].pos.x - (int)world.x, (int)pistolBullets[i].pos.y - (int)world.y, 5, 5, 0.0f, BLUE, kFillModeSolid);
				}
			}
		}
		else if (currentWeapon == 1) {
			for (int i = 0; i < riffleBulletsMax; i++) {
				if (riffleBullets[i].isActive) {
					Novice::DrawEllipse((int)riffleBullets[i].pos.x - (int)world.x, (int)riffleBullets[i].pos.y - (int)world.y, 5, 5, 0.0f, GREEN, kFillModeSolid);
				}
			}
		}
		else if (currentWeapon == 2) {
			for (int i = 0; i < launcherBulletsMax; i++) {
				if (launcherBullets[i].isActive) {
					Novice::DrawEllipse(
						(int)launcherBullets[i].pos.x - (int)world.x,
						(int)launcherBullets[i].pos.y - (int)world.y,
						8, 8, 0.0f, RED, kFillModeSolid
					);
				}
			}
		}


		Novice::DrawEllipse(mouseX, mouseY, 4, 4, 0.0f, RED, kFillModeSolid);

		////boss

		Novice::DrawSprite(
			(int)(boss.pos.x - world.x) - boss.radius,
			(int)(boss.pos.y - world.y) - boss.radius,
			BossGh,
			(boss.radius * 2.0f) / 64.0f,
			(boss.radius * 2.0f) / 64.0f,
			0.0f,
			WHITE
		);
		for (auto& b : bossBullets) if (b.isActive) Novice::DrawEllipse((int)b.pos.x - (int)world.x, (int)b.pos.y - (int)world.y, 5, 5, 0.0f, RED, kFillModeSolid);


		// UI
		Novice::DrawBox(900, 50, boss.hp, 20, 0.0f, GREEN, kFillModeSolid); // Boss HP
		Novice::ScreenPrintf(900, 30, "BOSS HP: %d", boss.hp);

		Novice::DrawBox(50, 50, player.hp * 5, 20, 0.0f, BLUE, kFillModeSolid); // Player HP
		Novice::ScreenPrintf(50, 30, "PLAYER HP: %d", player.hp);


		for (auto& b : bossBullets) {
			if (b.isActive) {
				Novice::DrawEllipse((int)b.pos.x - (int)world.x, (int)b.pos.y - (int)world.y, 5, 5, 0.0f, RED, kFillModeSolid);
			}
		}
		//Novice::DrawSprite(0, 0, Weapon, 1, 1, 0.0f, WHITE);

		///	
		if (currentWeapon == 0) {
			weaponName = "Pistol";
			Novice::DrawSprite(0, 0, PistolGh, 1, 1, 0.0f, WHITE); // ピストル画像
		}
		else if (currentWeapon == 1) {
			weaponName = "Rifle";
			Novice::DrawSprite(0, 0, RifleGh, 1, 1, 0.0f, WHITE); // ライフル画像
		}
		else if (currentWeapon == 2) {
			weaponName = "Launcher";
			Novice::DrawSprite(0, 0, LauncherGh, 1, 1, 0.0f, WHITE); // ランチャー画像
		}
		if (currentWeapon == 0) {   // 武器がピストルの時
			if (pistolAmmo == 0 && !isReloading) {
				Novice::DrawSprite(0, 0, ReloadGh, 1, 1, 0.0f, WHITE); // リロード警告画像
			}
		}

		// ライフルのリロードメッセージ
		if (currentWeapon == 1) {
			if (riffleAmmo == 0 && !isReloading) {
				Novice::DrawSprite(0, 0, ReloadGh, 1, 1, 0.0f, WHITE);
			}
		}

		// ランチャーのリロードメッセージ
		if (currentWeapon == 2) {
			if (launcherAmmo == 0 && !isLauncherReloading) {
				Novice::DrawSprite(0, 0, ReloadGh, 1, 1, 0.0f, WHITE);
			}
		}

		if (player.hp <= 0) {
			currentScene = GAMEOVER;
		}
		if (boss.hp <= 0) {
			currentScene = GAMECLEAR;
		}

		break;

		case GAMECLEAR:
			Novice::DrawSprite(0, 0, gameClearGh, 1, 1, 0.0f, WHITE); // 背景画像

			if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
				currentScene = TITLE;
			}
			break;

		case GAMEOVER:
			Novice::DrawSprite(0, 0, gameOverGh, 1, 1, 0.0f, WHITE); // 背景画像

			if (keys[DIK_SPACE] && preKeys[DIK_SPACE] == 0) {
				currentScene = TITLE;
			}
			break;
		}


		///
		/// ↑描画処理ここまで
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