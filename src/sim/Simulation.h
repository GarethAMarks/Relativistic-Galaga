#pragma once
#include "Ship.h"
#include "Enemy.h"
#include "Bolt.h"
#include "Powerup.h"
#include "Explosion.h"
#include "MuzzleFlash.h"
#include "HitEffect.h"
#include "SimEvent.h"
#include "Spawner.h"
#include "relativity/BoostFrame.h"
#include <memory>
#include <vector>

class Simulation {
public:
    explicit Simulation(float c_game);

    void reset(float c_game);
    void update(float dt, float c_game, bool lorentzianMode);

    using EnemyList   = std::vector<std::unique_ptr<Enemy>>;
    using PowerupList = std::vector<std::unique_ptr<Powerup>>;

    const Ship&                     ship()          const { return ship_; }
    const std::vector<Enemy>&       enemies()       const { return enemies_; }   // legacy formation
    const EnemyList&                hostiles()      const { return hostiles_; }  // drones, fighters, heavies
    const std::vector<Bolt>&        bolts()         const { return bolts_; }
    const std::vector<Bolt>&        enemyBolts()    const { return enemyBolts_; }
    const PowerupList&              powerups()      const { return powerups_; }
    const std::vector<Explosion>&   explosions()    const { return explosions_; }
    const std::vector<MuzzleFlash>& muzzleFlashes() const { return muzzleFlashes_; }
    const std::vector<HitEffect>&   hitEffects()    const { return hitEffects_; }
    // Events from the most recent update() (cleared at the start of each update).
    const std::vector<SimEvent>&    events()        const { return events_; }

    float labTime()         const { return labTime_; }
    float shipProperTime()  const { return shipProperTime_; }
    int   score()           const { return score_; }
    int   health()          const { return health_; }
    bool  isGameOver()      const { return gameOver_; }
    float twinShotTime()    const { return twinShotTime_; }    // ship proper seconds left
    float playerHitTimer()  const { return playerHitTimer_; }  // >0 just after taking a hit
    int   threatLevel()     const;

    static constexpr float PLAYER_HIT_FLASH = 0.35f;
    static constexpr int   PLAYER_MAX_HEALTH = 3;

    // Called by Powerup subclasses on pickup.
    void applyHeal(int hp);
    void applyBoost(float delta);
    void applyTwinShot(float properSeconds);

private:
    Ship                     ship_;
    std::vector<Enemy>       enemies_;      // legacy formation (currently disabled)
    EnemyList                hostiles_;
    std::vector<Bolt>        bolts_;
    std::vector<Bolt>        enemyBolts_;
    PowerupList              powerups_;
    std::vector<Explosion>   explosions_;
    std::vector<MuzzleFlash> muzzleFlashes_;
    std::vector<HitEffect>   hitEffects_;
    std::vector<SimEvent>    events_;
    std::vector<EnemyShot>   pendingShots_; // scratch buffer reused each frame
    Spawner                  spawner_;

    float labTime_            = 0.0f;
    float shipProperTime_     = 0.0f;
    float fireCooldown_       = 0.0f;
    float formationVelX_      = 60.0f;
    float enemySpawnTimer_    = 0.0f;
    float powerupSpawnTimer_  = 0.0f;
    float twinShotTime_       = 0.0f;
    float playerHitTimer_     = 0.0f;
    int   score_              = 0;
    int   wave_               = 1;
    int   health_             = PLAYER_MAX_HEALTH;
    bool  gameOver_           = false;

    static constexpr float BOLT_MUZZLE         = 420.0f;
    static constexpr float TWIN_OFFSET_X       = 7.0f;
    static constexpr float POWERUP_INTERVAL    = 10.0f;
    static constexpr float SHIP_GAMMA_MAX_CAP  = 3.0f;

    void spawnBolt(float c_game, bool lorentzianMode);
    void spawnEnemy();
    void spawnPowerup();
    void spawnExplosion(Vec2 pos, float scale = 1.0f);
    void spawnMuzzleFlash(Vec2 pos, Vec2 vel, Color color, bool trackShip);
    void spawnHitEffect(Vec2 pos, Vec2 vel, bool onPlayer);
    float nextSpawnInterval() const;
    void updateFormation(float dt);
    void updateHostiles(float dt);
    void updatePowerups(float dt);
    void updateExplosions(float dt);
    void updateMuzzleFlashes(float dt);
    void updateHitEffects(float dt);
    void resolveCollisions(bool lorentzianMode, const BoostFrame& boost);
    void resolvePlayerHits(bool lorentzianMode, const BoostFrame& boost);
    void pruneDead();
    void emit(SimEventType t, Vec2 pos, EnemyKind k = EnemyKind::Drone) {
        events_.push_back({t, pos, k});
    }
};
