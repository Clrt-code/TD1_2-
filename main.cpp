#include <Novice.h>
#include <corecrt_math.h>
#include <vector>
#define _USE_MATH_DEFINES

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
	//int riffleFireRate = 5; // 発射レート（フレーム数）
	//int riffleFireTimer = 0;
	const int riffleBulletsMax = 30;
	PlayerBullet riffleBullets[riffleBulletsMax] = {};

	//武器を切り替え
	int currentWeapon = 0;

	//Weapon box

	Vector2 weaponBoxPos = { 400.0f, 300.0f };
	bool rifleUnlocked = false;


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



		if (keys[DIK_W]) player.pos.y -= player.speed;
		if (keys[DIK_S]) player.pos.y += player.speed;
		if (keys[DIK_A]) player.pos.x -= player.speed;
		if (keys[DIK_D]) player.pos.x += player.speed;


		if (preKeys[DIK_J] == 0 && keys[DIK_J] != 0) {
			if (rifleUnlocked) {
			currentWeapon = 1 - currentWeapon;
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

			if (Novice::IsPressMouse(0)) {
				if (!isReloading && riffleAmmo > 0) {
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
					//riffleFireTimer = riffleFireRate;
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

		//weapon box pickup
		float dx = player.pos.x - weaponBoxPos.x;
		float dy = player.pos.y - weaponBoxPos.y;
		float distance = sqrtf(dx * dx + dy * dy);

		if (distance < 30.0f) {
			weaponBoxPos.x = -100.0f;
			weaponBoxPos.y = -100.0f;
			rifleUnlocked = true;
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

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::DrawBox(0, 0, kWindowWidth, kWindowHeight, 0.0f, BLACK, kFillModeSolid);

		//player
		Novice::DrawBox(
			int(player.pos.x),
			int(player.pos.y),
			int(player.width),
			int(player.height),
			0.0f,
			WHITE,
			kFillModeSolid
		);

		//aim line

		Novice::DrawLine((int)player.pos.x + (int)player.width / 2, (int)player.pos.y + (int)player.height / 2, mouseX, mouseY, WHITE);

		//weapon box
		Novice::DrawBox((int)weaponBoxPos.x, (int)weaponBoxPos.y, 20, 20, 0.0f, WHITE, kFillModeSolid);


		//武器を切り替え
		if (currentWeapon == 0) {
			for (int i = 0; i < pistolBulletsMax; i++) {
				if (pistolBullets[i].isActive) {
					Novice::DrawEllipse((int)pistolBullets[i].pos.x, (int)pistolBullets[i].pos.y, 5, 5, 0.0f, BLUE, kFillModeSolid);
				}
			}
		}
		else if (currentWeapon == 1) {
			for (int i = 0; i < riffleBulletsMax; i++) {
				if (riffleBullets[i].isActive) {
					Novice::DrawEllipse((int)riffleBullets[i].pos.x, (int)riffleBullets[i].pos.y, 5, 5, 0.0f, GREEN, kFillModeSolid);
				}
			}
		}

		Novice::DrawEllipse(mouseX, mouseY, 4, 4, 0.0f, RED, kFillModeSolid);

		//boss
		Novice::DrawEllipse((int)boss.pos.x, (int)boss.pos.y, 50, 50, 0.0f, RED, kFillModeSolid);

		// HP表示
		Novice::DrawBox(900, 50, boss.hp * 2, 20, 0.0f, GREEN, kFillModeSolid);

		for (auto& b : bossBullets) {
			if (b.isActive) {
				Novice::DrawEllipse((int)b.pos.x, (int)b.pos.y, 5, 5, 0.0f, RED, kFillModeSolid);
			}
		}

		///
		Novice::ScreenPrintf(0, 180, "Rifle Unlocked: %d", rifleUnlocked);
		Novice::ScreenPrintf(0, 100, "Current Weapon: %s", currentWeapon == 0 ? "Pistol" : "Riffle");
		Novice::ScreenPrintf(0, 120, "Ammo: %d / %d", pistolAmmo, pistolMaxAmmo);
		Novice::ScreenPrintf(0, 160, "Ammo: %d / %d", riffleAmmo, riffleMaxAmmo);
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
