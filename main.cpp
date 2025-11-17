#include <Novice.h>
#include <math.h>
#include <vector>

const char kWindowTitle[] = "GC1C_10_ミャッ_フォン_マウン";

#define _USE_MATH_DEFINES

struct Vector2 {
    float x, y;
};

struct Bullet {
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

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

    Novice::Initialize(kWindowTitle, 1280, 720);
    char keys[256] = { 0 };
    char preKeys[256] = { 0 };

    Boss boss = { {600.0f, 300.0f}, 100, 1, 0 };
    std::vector<Bullet> bullets;

    Vector2 playerPos = { 640.0f, 600.0f }; // 仮のプレイヤー位置

    while (Novice::ProcessMessage() == 0) {
        Novice::BeginFrame();
        memcpy(preKeys, keys, 256);
        Novice::GetHitKeyStateAll(keys);

        ///
        /// ↓更新処理
        ///
        /// 
      
        // フェーズ判定
        if (boss.hp <= 70 && boss.phase == 1) {
            boss.phase = 2;
        }
        if (boss.hp <= 40 && boss.phase == 2) {
            boss.phase = 3;
        }

        boss.attackTimer++;

        int interval = (boss.phase == 1) ? 120 : (boss.phase == 2) ? 80 : 40;
        int shotCount = (boss.phase == 1) ? 1 : (boss.phase == 2) ? 2 : 3;

        // 弾発射
        if (boss.attackTimer >= interval) {
            for (int i = 0; i < shotCount; i++) {
                Vector2 dir = {
                    playerPos.x - boss.pos.x,
                    playerPos.y - boss.pos.y
                };
                float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
                dir.x /= length;
                dir.y /= length;

                Bullet b;
                b.pos = boss.pos;
                b.velocity = { dir.x * 5.0f, dir.y * 5.0f };
                b.isActive = true;
                bullets.push_back(b);
            }
            boss.attackTimer = 0;
        }

        // 弾更新
        for (auto& b : bullets) {
            if (b.isActive) {
                b.pos.x += b.velocity.x;
                b.pos.y += b.velocity.y;
                if (b.pos.x < 0 || b.pos.x > 1280 || b.pos.y < 0 || b.pos.y > 720) {
                    b.isActive = false;
                }
            }
        }

        ///
        /// ↓描画処理
        ///
        // ボス
        Novice::DrawEllipse((int)boss.pos.x, (int)boss.pos.y, 50, 50, 0.0f, RED, kFillModeSolid);

        // HP表示
        Novice::DrawBox(50, 50, boss.hp * 2, 20, 0.0f, GREEN, kFillModeSolid);

        // プレイヤー仮表示
        Novice::DrawBox((int)playerPos.x - 10, (int)playerPos.y - 10, 20, 20, 0.0f, BLUE, kFillModeSolid);

        // 弾
        for (auto& b : bullets) {
            if (b.isActive) {
                Novice::DrawEllipse((int)b.pos.x, (int)b.pos.y, 5, 5, 0.0f, RED, kFillModeSolid);
            }
        }

        Novice::EndFrame();

        if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
            break;
        }
    }

    Novice::Finalize();
    return 0;
}
