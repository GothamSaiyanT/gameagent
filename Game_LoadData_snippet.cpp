// ─────────────────────────────────────────────────────────────────────────────
//  HOW TO PLUG MinerActor INTO YOUR Game.cpp
//  Only the relevant changes are shown — your existing code stays the same.
// ─────────────────────────────────────────────────────────────────────────────

// 1. In Game.h — add one member:
//
//    #include "MinerActor.h"
//    class MinerActor* mMinerActor;   // <-- add this in the private section

// 2. In Game.cpp — add to the #includes at the top:
//
//    #include "MinerActor.h"

// 3. In Game::LoadData() — add this AFTER your tile map and Ship setup:

void Game::LoadData()
{
    // ... your existing tile/ship setup stays exactly the same ...

    // ── Spawn the Q-learning miner ────────────────────────────────────────
    mMinerActor = new MinerActor(this);

    // Tell him where each location is in YOUR world (adjust to match your map)
    mMinerActor->SetMinePosition  (Vector2(-300.0f,  100.0f));
    mMinerActor->SetBankPosition  (Vector2( 300.0f,  100.0f));
    mMinerActor->SetShackPosition (Vector2(   0.0f, -150.0f));
    mMinerActor->SetSaloonPosition(Vector2(   0.0f,  150.0f));

    // Optionally give him a sprite — uncomment and set your texture path:
    // SpriteComponent* sc = new SpriteComponent(mMinerActor, 200);
    // sc->SetTexture(mRenderer->GetTexture("Assets/Miner.png"));
}

// 4. In Game::UpdateGame() — optionally print debug info each second:
//    (paste this inside UpdateGame, after the actor update loop)

void Game::UpdateGame()
{
    // ... your existing update code stays exactly the same ...

    // ── Optional: print miner status to SDL log every second ─────────────
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;   // deltaTime is already computed above
    if (debugTimer >= 1.0f && mMinerActor)
    {
        debugTimer = 0.0f;
        SDL_Log("[Miner] Ep=%d  Action=%-10s  Location=%-12s  "
                "Gold=%d  Bank=%d  Thirst=%d  Fatigue=%d  eps=%.3f",
                mMinerActor->GetEpisode(),
                mMinerActor->GetActionName(),
                mMinerActor->GetLocationName(),
                mMinerActor->GetGold(),
                mMinerActor->GetWealth(),
                mMinerActor->GetThirst(),
                mMinerActor->GetFatigue(),
                mMinerActor->GetEpsilon());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  THAT'S IT. No changes needed to:
//    Actor.h / Actor.cpp      — MinerActor inherits cleanly
//    Renderer / Shader        — uses the same sprite pipeline
//    MoveComponent            — MinerActor does its own movement math
//                               (simpler than MoveComponent for this use case)
//    TileMapComponent         — unchanged
// ─────────────────────────────────────────────────────────────────────────────
