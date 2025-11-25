#include <Novice.h>
#ifdef USE_IMGUI
#include"imgui.h"
#endif
const char kWindowTitle[] = "GC1C_10_ミャッ_フォン_マウン";

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);


	struct Vector2 {
		float x;
		float y;
	};

	struct Map {

		Vector2 topLeft = { -1280,-720 };
		Vector2 topRight = { 1280 * 2,-720 };
		Vector2 bottomLeft = { -1280,720 * 2 };
		Vector2 bottomRight = { 1280 * 2,720 * 2 };

		Vector2 objectTopLeft[15];
		Vector2 objectTopRight[15];
		Vector2 objectBottomLeft[15];
		Vector2 objectBottomRight[15];
		Vector2 object[15] = { 9999,9999 };
		int objectWidth[15];
		int objectHeight[15];







	};


	Vector2 world = { 0,0 };




	struct Player {
		Vector2 pos;
		Vector2 velocity;
		float width;
		float height;
		float speed;
	};


	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };


	Player player = {
		.pos = {64.0f, 640.0f},
		.velocity = {0.0f, 0.0f},
		.width = 64.0f,
		.height = 40.0f,
		.speed = 4.0f,
	};

	Map map = {};




	int sprite[10] = {

		Novice::LoadTexture("./sprite/map_background.png"),
		Novice::LoadTexture("./sprite/object_wood.png"),
		Novice::LoadTexture("./sprite/itemTest.png"),

	};

	map.object[0].x = 340;
	map.object[0].y = 160;





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
			
			for (int o = 0;o < 15;o++) {
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

			for (int o = 0;o < 15;o++) {
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

			for (int o = 0;o < 15;o++) {
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
			for (int o = 0;o < 15;o++) {
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


		//オブジェクト の 座標(object position)
		for (int i = 0;i < 15;i++) {


			map.objectTopLeft[i].x = map.object[i].x;
			map.objectTopLeft[i].y = map.object[i].y;
			map.objectTopRight[i].x = map.object[i].x + 90 + 81;
			map.objectTopRight[i].y = map.object[i].y;
			map.objectBottomLeft[i].x = map.object[i].x;
			map.objectBottomLeft[i].y = map.object[i].y + 90 + 192;//192(128*1.5)
			map.objectBottomRight[i].x = map.object[i].x + 90 + 81;
			map.objectBottomRight[i].y = map.object[i].y + 90 + 192;
		}



		ImGui::Begin("MAP");
		ImGui::SliderFloat("world map x", &world.x, -1280.0f, 1280.0f);
		ImGui::SliderFloat("world map y", &world.y, -720.0f, 720.0f);
		ImGui::End();
		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		//背景
		for (int i = -1;i < 2;i++) {
			for (int k = -1;k < 2;k++) {
				Novice::DrawSprite(-(int)world.x + 1280 * k, (int)world.y + 720 * i, sprite[0], 1, 1, 0.0f, WHITE);
			}
		}

		//オブジェクト
		for (int i = 0;i < 15;i++) {
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



		//マップ範囲描画
		Novice::DrawLine((int)map.topLeft.x - (int)world.x, (int)map.topLeft.y + (int)world.y, (int)map.topRight.x - (int)world.x, (int)map.topRight.y + (int)world.y, RED);
		Novice::DrawLine((int)map.bottomLeft.x - (int)world.x, (int)map.bottomLeft.y + (int)world.y, (int)map.bottomRight.x - (int)world.x, (int)map.bottomRight.y + (int)world.y, RED);
		Novice::DrawLine((int)map.topLeft.x - (int)world.x, (int)map.topLeft.y + (int)world.y, (int)map.bottomLeft.x - (int)world.x, (int)map.bottomLeft.y + (int)world.y, RED);
		Novice::DrawLine((int)map.topRight.x - (int)world.x, (int)map.topRight.y + (int)world.y, (int)map.bottomRight.x - (int)world.x, (int)map.bottomRight.y + (int)world.y, RED);

		//マップオブジェクト当たり判定のデバッグ
		
		for (int i = 0;i < 15;i++) {
			Novice::DrawLine((int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y + (int)world.y, (int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y + (int)world.y, GREEN);
			Novice::DrawLine((int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y + (int)world.y, (int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y + (int)world.y, GREEN);
			Novice::DrawLine((int)map.objectTopLeft[i].x - (int)world.x, (int)map.objectTopLeft[i].y + (int)world.y, (int)map.objectBottomLeft[i].x - (int)world.x, (int)map.objectBottomLeft[i].y + (int)world.y, GREEN);
			Novice::DrawLine((int)map.objectTopRight[i].x - (int)world.x, (int)map.objectTopRight[i].y + (int)world.y, (int)map.objectBottomRight[i].x - (int)world.x, (int)map.objectBottomRight[i].y + (int)world.y, GREEN);
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
