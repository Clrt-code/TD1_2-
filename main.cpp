#include <Novice.h>
#include <corecrt_math.h>
#include <vector>
#define _USE_MATH_DEFINES
#ifdef USE_IMGUI
#include"imgui.h"
#endif

const char kWindowTitle[] = "GC1C_10_ミャッ_フォン_マウン";

const int kWindowWidth = 1280;
const int kWindowHeight = 720;

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

int mouseX = 0;
int mouseY = 0;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, kWindowWidth, kWindowHeight);

	struct Vector2 {
		float x;
		float y;
	};

	//map
	struct Map {

		Vector2 topLeft = { 0,0 };
		Vector2 topRight = { 1280,0 };
		Vector2 bottomLeft = { 0,720 };
		Vector2 bottomRight = { 1280,720 };

		Vector2 objectTopLeft[15];
		Vector2 objectTopRight[15];
		Vector2 objectBottomLeft[15];
		Vector2 objectBottomRight[15];
		int objectWidth[15];
		int objectHeight[15];

	};

	Vector2 world = { 0,0 };

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
		bool isDashing;
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
		int phase;
		int attackTimer;
	};


	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	Map map = {};

	int sprite[10] = {

		Novice::LoadTexture("./sprite/map_background.png"),
		Novice::LoadTexture("./sprite/object_wood.png"),
		Novice::LoadTexture("./sprite/itemTest.png"),

	};

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
		.width = 20.0f,
		.height = 20.0f,
		.speed = 4.0f,
		.dashCooldown = 0,
		.dashDuration = 0,
		.dashSpeed = 12,
		.isDashing = false

	};

	Boss boss = { {600.0f, 300.0f}, 100, 1, 0 };
	std::vector<BossBullet> bossBullets;

	// pistol's bullets(revolver type)

	int pistolAmmo = 6;
	int pistolMaxAmmo = 6;
	bool isReloading = false;
	int reloadTime = 0;
	const int reloadDuration = 90;
	const int pistolBulletsMax = 6;
	PlayerBullet pistolBullets[pistolBulletsMax] = {};

	// riffle's bullets(automatic type)
	int riffleAmmo = 30;
	int riffleMaxAmmo = 30;
	int rifleFireRate = 5;     // shoot every 5 frames
	int rifleFireTimer = 0;
	const int riffleBulletsMax = 30;
	PlayerBullet riffleBullets[riffleBulletsMax] = {};

	// launcher's bullets (grenade type) - 未実装
	int launcherAmmo = 5;
	int launcherMaxAmmo = 5;
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

	//マップ当たり判定
		if (player.pos.x < map.topLeft.x) {
			player.pos.x = map.topLeft.x;
		}
		if (player.pos.x > map.topRight.x - player.width) {
			player.pos.x = map.topRight.x - player.width;
		}
		if (player.pos.y < map.topLeft.y) {
			player.pos.y = map.topLeft.y;
		}
		if (player.pos.y > map.bottomLeft.y - player.height) {
			player.pos.y = map.bottomLeft.y - player.height;
		}
		//当たり判定のデバッグ
		if (Novice::CheckHitKey(DIK_1) && Novice::CheckHitKey(DIK_RIGHT)) {
			map.topLeft.x += 5;
			map.bottomLeft.x += 5;
		}if (Novice::CheckHitKey(DIK_1) && Novice::CheckHitKey(DIK_LEFT)) {
			map.topLeft.x -= 5;
			map.bottomLeft.x -= 5;
		}if (Novice::CheckHitKey(DIK_1) && Novice::CheckHitKey(DIK_UP)) {
			map.topLeft.y -= 5;
			map.topRight.y -= 5;
		}if (Novice::CheckHitKey(DIK_1) && Novice::CheckHitKey(DIK_DOWN)) {
			map.topLeft.y += 5;
			map.topRight.y += 5;
		}

		if (Novice::CheckHitKey(DIK_2) && Novice::CheckHitKey(DIK_RIGHT)) {
			map.topRight.x += 5;
			map.bottomRight.x += 5;
		}if (Novice::CheckHitKey(DIK_2) && Novice::CheckHitKey(DIK_LEFT)) {
			map.topRight.x -= 5;
			map.bottomRight.x -= 5;
		}if (Novice::CheckHitKey(DIK_2) && Novice::CheckHitKey(DIK_UP)) {
			map.bottomLeft.y -= 5;
			map.bottomRight.y -= 5;
		}if (Novice::CheckHitKey(DIK_2) && Novice::CheckHitKey(DIK_DOWN)) {
			map.bottomLeft.y += 5;
			map.bottomRight.y += 5;
		}

		/*if (keys[DIK_W]) player.pos.y -= player.speed;
		if (keys[DIK_S]) player.pos.y += player.speed;
		if (keys[DIK_A]) player.pos.x -= player.speed;
		if (keys[DIK_D]) player.pos.x += player.speed;*/


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
							// プレイヤーからマウスへの方向ベクトルを正規化
							float dx = mouseX - player.pos.x;
							float dy = mouseY - player.pos.y;
							float len = sqrtf(dx * dx + dy * dy);
							if (len != 0) {
								pistolBullets[i].velocity = { (dx / len) * 10.0f, (dy / len) * 10.0f };
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

					if (pistolBullets[i].pos.x < 0 || pistolBullets[i].pos.x > kWindowWidth ||
						pistolBullets[i].pos.y < 0 || pistolBullets[i].pos.y > kWindowHeight) {
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
							// プレイヤーからマウスへの方向ベクトルを正規化
							float dx = mouseX - player.pos.x;
							float dy = mouseY - player.pos.y;
							float len = sqrtf(dx * dx + dy * dy);
							if (len != 0) {
								riffleBullets[i].velocity = { (dx / len) * 15.0f, (dy / len) * 15.0f };
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
					if (riffleBullets[i].pos.x < 0 || riffleBullets[i].pos.x > kWindowWidth ||
						riffleBullets[i].pos.y < 0 || riffleBullets[i].pos.y > kWindowHeight) {
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
							// プレイヤーからマウスへの方向ベクトルを正規化
							float dx = mouseX - player.pos.x;
							float dy = mouseY - player.pos.y;
							float len = sqrtf(dx * dx + dy * dy);
							if (len != 0) {
								launcherBullets[i].velocity = { (dx / len) * 8.0f, (dy / len) * 8.0f };
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
					if (launcherBullets[i].pos.x < 0 || launcherBullets[i].pos.x > kWindowWidth ||
						launcherBullets[i].pos.y < 0 || launcherBullets[i].pos.y > kWindowHeight) {
						launcherBullets[i].isActive = false;
					}
				}
			}
		}




		// Rifle weapon box
		float dxR = player.pos.x - rifleBoxPos.x;
		float dyR = player.pos.y - rifleBoxPos.y;
		float disR = sqrtf(dxR * dxR + dyR * dyR);

		if (disR < 30.0f) {
			rifleBoxPos.x = -100.0f;
			rifleBoxPos.y = -100.0f;
			rifleUnlocked = true;
		}
	
		// Launcher weapon box
		float dxL = player.pos.x - launcherBoxPos.x;
		float dyL = player.pos.y - launcherBoxPos.y;
		float distL = sqrtf(dxL * dxL + dyL * dyL);

		if (distL < 30.0f) {
			launcherBoxPos.x = -100.0f;
			launcherBoxPos.y = -100.0f;
			launcherUnlocked = true;
		}

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
			player.dashCooldown = 60;
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

		int interval = (boss.phase == 1) ? 120 : (boss.phase == 2) ? 80 : 40;
		int shotCount = (boss.phase == 1) ? 1 : (boss.phase == 2) ? 2 : 3;

		if (boss.attackTimer >= interval) {
			for (int i = 0; i < shotCount; i++) {
				Vector2 dir = {
					player.pos.x - boss.pos.x,
					player.pos.y - boss.pos.y
				};
				float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
				dir.x /= length;
				dir.y /= length;
				BossBullet b;
				b.pos = boss.pos;
				b.velocity = { dir.x * 5.0f, dir.y * 5.0f };
				b.isActive = true;
				bossBullets.push_back(b);
			}
			boss.attackTimer = 0;
		}
		for (auto& b : bossBullets) {
			if (b.isActive) {
				b.pos.x += b.velocity.x;
				b.pos.y += b.velocity.y;
				if (b.pos.x < 0 || b.pos.x > kWindowWidth || b.pos.y < 0 || b.pos.y > kWindowHeight) {
					b.isActive = false;
				}
			}
		}

		// pistolBulletとボスの当たり判定

		for (int i = 0; i < pistolBulletsMax; i++) {
			if (pistolBullets[i].isActive) {
				float pistolDx = (pistolBullets[i].pos.x) - (boss.pos.x);
				float pistolDy = (pistolBullets[i].pos.y) - (boss.pos.y);
				float pistolDistance = sqrtf(pistolDx * pistolDx + pistolDy * pistolDy);
				if (pistolDistance < 25.0f) {
					pistolBullets[i].isActive = false;
					boss.hp -= 5;
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
				if (rifleDistance < 25.0f) {
					riffleBullets[i].isActive = false;
					boss.hp -= 5;
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

				if (launcherDistance < 40.0f) { 
					launcherBullets[i].isActive = false;
					boss.hp -= 20; 
					if (boss.hp < 0) boss.hp = 0;
				}
			}
		}



		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		ImGui::Begin("MAP");
		ImGui::SliderFloat("world map x", &world.x, -1280.0f, 1280.0f);
		ImGui::SliderFloat("world map y", &world.y, -720.0f, 720.0f);
		ImGui::End();

		for (int i = -1; i < 2; i++) {
			for (int k = -1; k < 2; k++) {
				Novice::DrawSprite(-(int)world.x + 1280 * k, (int)world.y + 720 * i, sprite[0], 1, 1, 0.0f, WHITE);
			}
		}

		//オブジェクト
		for (int i = 0; i < 15; i++) {
			Novice::DrawQuad((int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y + (int)world.y, (int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y + (int)world.y, (int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y + (int)world.y, (int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y + (int)world.y, 0, 0, 81, 128, sprite[1], WHITE);
		}

		Novice::DrawBox(
			int(player.pos.x - (int)world.x),
			int(player.pos.y + (int)world.y),
			int(player.width),
			int(player.height),
			0.0f,
			0xFFFFFFFF,
			kFillModeSolid
		);

		//aim line

		Novice::DrawLine((int)player.pos.x -(int)world.x + (int)player.width / 2, (int)player.pos.y + (int)world.y + (int)player.height / 2, mouseX, mouseY, WHITE);

			//マップ範囲描画
			Novice::DrawLine((int)map.topLeft.x - (int)world.x, (int)map.topLeft.y + (int)world.y, (int)map.topRight.x - (int)world.x, (int)map.topRight.y + (int)world.y, RED);
			Novice::DrawLine((int)map.bottomLeft.x - (int)world.x, (int)map.bottomLeft.y + (int)world.y, (int)map.bottomRight.x - (int)world.x, (int)map.bottomRight.y + (int)world.y, RED);
			Novice::DrawLine((int)map.topLeft.x - (int)world.x, (int)map.topLeft.y + (int)world.y, (int)map.bottomLeft.x - (int)world.x, (int)map.bottomLeft.y + (int)world.y, RED);
			Novice::DrawLine((int)map.topRight.x - (int)world.x, (int)map.topRight.y + (int)world.y, (int)map.bottomRight.x - (int)world.x, (int)map.bottomRight.y + (int)world.y, RED);

			//マップオブジェクト当たり判定のデバッグ

			for (int i = 0; i < 15; i++) {
				Novice::DrawLine((int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y + (int)world.y, (int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y + (int)world.y, GREEN);
				Novice::DrawLine((int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y + (int)world.y, (int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y + (int)world.y, GREEN);
				Novice::DrawLine((int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y + (int)world.y, (int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y + (int)world.y, GREEN);
				Novice::DrawLine((int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y + (int)world.y, (int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y + (int)world.y, GREEN);
			}


		//Novice::DrawBox(0, 0, kWindowWidth, kWindowHeight, 0.0f, BLACK, kFillModeSolid);

		////player
		//Novice::DrawBox(
		//	int(player.pos.x),
		//	int(player.pos.y),
		//	int(player.width),
		//	int(player.height),
		//	0.0f,
		//	WHITE,
		//	kFillModeSolid
		//);



		//weapon box
		Novice::DrawBox((int)rifleBoxPos.x - (int)world.x, (int)rifleBoxPos.y + (int)world.y, 20, 20, 0.0f, WHITE, kFillModeSolid);
		Novice::DrawBox((int)launcherBoxPos.x - (int)world.x, (int)launcherBoxPos.y + (int)world.y, 20, 20, 0.0f, WHITE, kFillModeSolid);
		

		//武器を切り替え
		if (currentWeapon == 0) {
			for (int i = 0; i < pistolBulletsMax; i++) {
				if (pistolBullets[i].isActive) {
					Novice::DrawEllipse((int)pistolBullets[i].pos.x - (int)world.x, (int)pistolBullets[i].pos.y + (int)world.y, 5, 5, 0.0f, BLUE, kFillModeSolid);
				}
			}
		}
		else if (currentWeapon == 1) {
			for (int i = 0; i < riffleBulletsMax; i++) {
				if (riffleBullets[i].isActive) {
					Novice::DrawEllipse((int)riffleBullets[i].pos.x - (int)world.x, (int)riffleBullets[i].pos.y + (int)world.y, 5, 5, 0.0f, GREEN, kFillModeSolid);
				}
			}
		}
		else if (currentWeapon == 2) {
			for (int i = 0; i < launcherBulletsMax; i++) {
				if (launcherBullets[i].isActive) {
					Novice::DrawEllipse(
						(int)launcherBullets[i].pos.x - (int)world.x,
						(int)launcherBullets[i].pos.y + (int)world.y,
						8, 8, 0.0f, RED, kFillModeSolid
					);
				}
			}
		}


		Novice::DrawEllipse(mouseX, mouseY, 4, 4, 0.0f, RED, kFillModeSolid);

		//boss
		Novice::DrawEllipse((int)boss.pos.x - (int)world.x, (int)boss.pos.y + (int)world.y, 50, 50, 0.0f, RED, kFillModeSolid);

		// HP表示
		Novice::DrawBox(900, 50, boss.hp * 2, 20, 0.0f, GREEN, kFillModeSolid);

		for (auto& b : bossBullets) {
			if (b.isActive) {
				Novice::DrawEllipse((int)b.pos.x - (int)world.x, (int)b.pos.y + (int)world.y, 5, 5, 0.0f, RED, kFillModeSolid);
			}
		}

		///	
		if (currentWeapon == 0) {
			weaponName = "Pistol";
		}
		else if (currentWeapon == 1) {
			weaponName = "Rifle";
		}
		else if (currentWeapon == 2) {
			weaponName = "Launcher";
		}

		Novice::ScreenPrintf(0, 100, "Current Weapon: %s", weaponName);
		Novice::ScreenPrintf(0, 100, "Current Weapon: %s", weaponName);
		Novice::ScreenPrintf(0, 180, "Rifle Unlocked: %d", rifleUnlocked);
		Novice::ScreenPrintf(0, 220, "Launcher Unlocked: %d", launcherUnlocked);
	
		Novice::ScreenPrintf(0, 120, "pistolAmmo: %d / %d", pistolAmmo, pistolMaxAmmo);
		Novice::ScreenPrintf(0, 160, "rifleAmmo: %d / %d", riffleAmmo, riffleMaxAmmo);
		Novice::ScreenPrintf(0, 200, "launcherAmmo: %d / %d", launcherAmmo, launcherMaxAmmo);
		Novice::ScreenPrintf(0, 140, "Reloading: %d", isReloading);
		Novice::ScreenPrintf(0, 0, "Mouse X: %d Y: %d", mouseX, mouseY);
		Novice::ScreenPrintf(0, 20, "Player X: %.2f Y: %.2f", player.pos.x, player.pos.y);
		Novice::ScreenPrintf(0, 40, "Dash Cooldown: %d", player.dashCooldown);
		Novice::ScreenPrintf(0, 60, "Dash Duration: %d", player.dashDuration);
		Novice::ScreenPrintf(0, 80, "Is Dashing: %d", player.isDashing ? 1 : 0);
		

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
