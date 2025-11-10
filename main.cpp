#include <Novice.h>

const char kWindowTitle[] = "GC1C_10_ミャッ_フォン_マウン";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	struct Vector2 {
		float x;
		float y;
	};

	struct Player {
		Vector2 pos;
		Vector2 velocity;
		float width;
		float height;
		float speed;
	};


	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};


	Player player = {
		.pos = {64.0f, 640.0f},
		.velocity = {0.0f, 0.0f},
		.width = 64.0f,
		.height = 40.0f,
		.speed = 4.0f,
	};

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		if (keys[DIK_W]) {
			player.pos.y -= player.speed;
		}	
		if (keys[DIK_S]) {
			player.pos.y += player.speed;
		}
		if (keys[DIK_A]) {
			player.pos.x -= player.speed;
		}
		if (keys[DIK_D]) {
			player.pos.x += player.speed;
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
			0xFFFFFFFF,
			kFillModeSolid
		);	

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
