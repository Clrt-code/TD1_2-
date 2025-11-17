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

	//bool isHit = false;

	const int playerBulletMax = 10;
	PlayerBullet playerBullets[playerBulletMax] = {};

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

			if (Novice::IsPressMouse(0)) { // クリックされた瞬間
				for (int i = 0; i < playerBulletMax; i++) {
					if (!playerBullets[i].isActive) {
						playerBullets[i].isActive = true;
						playerBullets[i].pos = {
							player.pos.x + player.width / 2,
							player.pos.y + player.height / 2
						};
						// プレイヤーからマウスへの方向ベクトルを正規化
						float dx = mouseX - player.pos.x;
						float dy = mouseY - player.pos.y;
						float len = sqrtf(dx * dx + dy * dy);
						if (len != 0) {
							playerBullets[i].velocity = { (dx / len) * 10.0f, (dy / len) * 10.0f };
						}

						break;
					}
				}
			}
			for (int i = 0; i < playerBulletMax; i++) {
				if (playerBullets[i].isActive) {
					playerBullets[i].pos.x += playerBullets[i].velocity.x;
					playerBullets[i].pos.y += playerBullets[i].velocity.y;

					if (playerBullets[i].pos.x < 0 || playerBullets[i].pos.x > kWindowWidth ||
						playerBullets[i].pos.y < 0 || playerBullets[i].pos.y > kWindowHeight) {
						playerBullets[i].isActive = false;
					}
				}
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

			// プレイヤーとボスの当たり判定

			for (int i = 0; i < playerBulletMax; i++) {
				if (playerBullets[i].isActive) {
					float dx = (playerBullets[i].pos.x) - (boss.pos.x);
					float dy = (playerBullets[i].pos.y) - (boss.pos.y);
					float distance = sqrtf(dx * dx + dy * dy);
					if (distance < 25.0f) { 
						playerBullets[i].isActive = false;
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
		Novice::DrawBox(
			int(player.pos.x),
			int(player.pos.y),
			int(player.width),
			int(player.height),
			0.0f,
			WHITE,
			kFillModeSolid
		);	

		Novice::DrawLine((int)player.pos.x + (int) player.width/2, (int)player.pos.y + (int)player.height/2, mouseX, mouseY, WHITE);

		for (int i = 0; i < playerBulletMax; i++) {
			if (playerBullets[i].isActive) {
				Novice::DrawEllipse((int)playerBullets[i].pos.x , (int)playerBullets[i].pos.y, 5, 5, 0.0f, BLUE, kFillModeSolid);
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
