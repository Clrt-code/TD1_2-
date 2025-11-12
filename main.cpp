#include <Novice.h>
#include <corecrt_math.h>

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
		float width;
		float height;
		float speed;
	};
	struct Bullet {
		Vector2 pos;
		Vector2 velocity;
		bool isActive;
	};

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	

	Player player = {
		.pos = {64.0f, 640.0f},
		.width = 20.0f,
		.height = 20.0f,
		.speed = 4.0f,

	};


	const int bulletMax = 10;
	Bullet bullets[bulletMax] = {};

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
				for (int i = 0; i < bulletMax; i++) {
					if (!bullets[i].isActive) {
						bullets[i].isActive = true;
						bullets[i].pos = {
							player.pos.x + player.width / 2,
							player.pos.y + player.height / 2
						};
						// プレイヤーからマウスへの方向ベクトルを正規化
						float dx = mouseX - player.pos.x;
						float dy = mouseY - player.pos.y;
						float len = sqrtf(dx * dx + dy * dy);
						if (len != 0) {
							bullets[i].velocity = { (dx / len) * 10.0f, (dy / len) * 10.0f };
						}

						break;
					}
				}
			}
			for (int i = 0; i < bulletMax; i++) {
				if (bullets[i].isActive) {
					bullets[i].pos.x += bullets[i].velocity.x;
					bullets[i].pos.y += bullets[i].velocity.y;

					// 画面外で無効化
					if (bullets[i].pos.x < 0 || bullets[i].pos.x > kWindowWidth ||
						bullets[i].pos.y < 0 || bullets[i].pos.y > kWindowHeight) {
						bullets[i].isActive = false;
					}
				}
			}

			


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		Novice::DrawBox(
			int(player.pos.x),
			int(player.pos.y),
			int(player.width),
			int(player.height),
			0.0f,
			WHITE,
			kFillModeSolid
		);	

		Novice::DrawLine((int)player.pos.x, (int)player.pos.y, mouseX, mouseY, WHITE);

		for (int i = 0; i < bulletMax; i++) {
			if (bullets[i].isActive) {
				Novice::DrawEllipse((int)bullets[i].pos.x, (int)bullets[i].pos.y, 5, 5, 0.0f, BLACK, kFillModeSolid);
			}
		}
		Novice::DrawEllipse(mouseX, mouseY, 4, 4, 0.0f, RED, kFillModeSolid);

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
