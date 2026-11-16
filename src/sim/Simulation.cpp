#include "Simulation.h"
#include "Powerups.h"
#include "Drone.h"
#include "Fighter.h"
#include "Heavy.h"
#include "Difficulty.h"
#include "relativity/Aberration.h"
#include "relativity/Doppler.h"
#include "relativity/Lorentz.h"
#include <raylib.h>
#include <cstdlib>
#include <cmath>
#include <algorithm>

static float frand() { return static_cast<float>(rand() % 10000) / 10000.0f; }

Simulation::Simulation(float c_game) {
    reset(c_game);
}

void Simulation::reset(float c_game) {
    ship_.reset();
    // spawner_.spawnWave(enemies_, 1);  // formation enemies — re-enable to test
    enemies_.clear();
    hostiles_.clear();
    bolts_.clear();
    enemyBolts_.clear();
    powerups_.clear();
    explosions_.clear();
    muzzleFlashes_.clear();
    hitEffects_.clear();
    events_.clear();
    labTime_        = 0.0f;
    shipProperTime_ = 0.0f;
    fireCooldown_   = 0.0f;
    formationVelX_  = 60.0f;
    twinShotTime_   = 0.0f;
    playerHitTimer_ = 0.0f;
    score_          = 0;
    wave_           = 1;
    health_         = PLAYER_MAX_HEALTH;
    gameOver_       = false;
    ship_.gammaMax  = 1.0328f;
    {
        float beta0 = std::sqrt(1.0f - 1.0f / (ship_.gammaMax * ship_.gammaMax));
        float vmax0 = c_game * beta0;
        ship_.timeToMax = vmax0 / Ship::BASE_ACCEL;
    }
    enemySpawnTimer_   = nextSpawnInterval();
    powerupSpawnTimer_ = POWERUP_INTERVAL;
}

int Simulation::threatLevel() const {
    return difficulty::threatLevel(labTime_);
}

void Simulation::update(float dt, float c_game, bool lorentzianMode) {
    events_.clear();
    labTime_ += dt;

    updateMuzzleFlashes(dt);   // before any entity moves, so spawn-frame flashes aren't double-stepped
    if (gameOver_) ship_.vel = {0.0f, 0.0f};   // wreck: no more control
    else           ship_.update(dt, c_game);
    BoostFrame boost = BoostFrame::fromVelocity(ship_.vel, c_game);

    // Proper time on the ship's wristwatch.  Ship-borne timers (twin cannons)
    // run on this clock, so they last longer in lab time at high gamma.
    float dTau = lorentzianMode ? dt / boost.gamma : dt;
    shipProperTime_ += dTau;
    if (twinShotTime_  > 0.0f) twinShotTime_  = std::max(0.0f, twinShotTime_ - dTau);
    if (playerHitTimer_ > 0.0f) playerHitTimer_ = std::max(0.0f, playerHitTimer_ - dt);

    fireCooldown_ -= dt;
    if (!gameOver_ && IsKeyPressed(KEY_SPACE) && fireCooldown_ <= 0.0f) {
        spawnBolt(c_game, lorentzianMode);
        emit(SimEventType::PlayerFired, ship_.pos);
        fireCooldown_ = 0.25f;
    }

    for (auto& b : bolts_) {
        if (b.alive) b.pos = b.pos + b.vel * dt;
    }

    // updateFormation(dt);  // formation enemies — re-enable to test
    updateHostiles(dt);
    updatePowerups(dt);
    updateExplosions(dt);
    resolveCollisions(lorentzianMode, boost);
    resolvePlayerHits(lorentzianMode, boost);
    updateHitEffects(dt);
    pruneDead();

    // Formation wave-clear and breach check — re-enable with formation block above
    // bool anyAlive = false;
    // for (const auto& e : enemies_) anyAlive |= e.alive;
    // if (!anyAlive) {
    //     ++wave_;
    //     formationVelX_ = 60.0f + wave_ * 8.0f;
    //     spawner_.spawnWave(enemies_, wave_);
    //     bolts_.clear();
    // }
    // for (const auto& e : enemies_) {
    //     if (e.alive && e.pos.y > 460.0f) {
    //         gameOver_ = true;
    //         return;
    //     }
    // }
}

// ---- Enemy spawning -------------------------------------------------------

float Simulation::nextSpawnInterval() const {
    float base = difficulty::SPAWN_MIN +
                 frand() * (difficulty::SPAWN_MAX - difficulty::SPAWN_MIN);
    return std::max(difficulty::SPAWN_FLOOR,
                    base * difficulty::spawnIntervalScale(labTime_));
}

void Simulation::spawnEnemy() {
    float wD = difficulty::droneWeight(labTime_);
    float wF = difficulty::fighterWeight(labTime_);
    float wH = difficulty::heavyWeight(labTime_);
    float r  = frand() * (wD + wF + wH);

    if (r < wD) {
        float x = 30.0f + frand() * 740.0f;
        hostiles_.push_back(std::make_unique<Drone>(x, labTime_));
    } else if (r < wD + wF) {
        // Keep the whole weave on screen.
        float margin = Fighter::AMPLITUDE + Fighter::HALF_W + 5.0f;
        float cx     = margin + frand() * (800.0f - 2.0f * margin);
        float phase  = frand() * 6.2831853f;
        hostiles_.push_back(std::make_unique<Fighter>(cx, phase, labTime_));
    } else {
        float margin = Heavy::HALF_W + 10.0f;
        float x      = margin + frand() * (800.0f - 2.0f * margin);
        hostiles_.push_back(std::make_unique<Heavy>(x, labTime_));
    }
}

void Simulation::updateHostiles(float dt) {
    pendingShots_.clear();
    for (auto& e : hostiles_) {
        if (!e->alive) continue;
        size_t before = pendingShots_.size();
        e->tick(dt, labTime_, pendingShots_);
        if (pendingShots_.size() > before)
            emit(SimEventType::EnemyFired, e->pos, e->kind());
        for (size_t i = before; i < pendingShots_.size(); ++i) {
            const EnemyShot& s = pendingShots_[i];
            Bolt b;
            b.pos       = s.pos;
            b.vel       = s.vel;
            b.alive     = true;
            b.spawnTime = labTime_;
            b.doppler   = 1.0f;
            b.big       = s.big;
            b.hw        = s.big ? Bolt::BIG_HW : Bolt::HW;
            b.hh        = s.big ? Bolt::BIG_HH : Bolt::HH;
            enemyBolts_.push_back(b);
            spawnMuzzleFlash(s.pos, e->vel, ORANGE, /*trackShip=*/false);
        }
    }

    enemySpawnTimer_ -= dt;
    if (enemySpawnTimer_ <= 0.0f) {
        spawnEnemy();
        enemySpawnTimer_ = nextSpawnInterval();
    }

    for (auto& b : enemyBolts_) {
        if (!b.alive) continue;
        b.pos = b.pos + b.vel * dt;
        if (b.pos.y > 640.0f || b.pos.x < -40.0f || b.pos.x > 840.0f) b.alive = false;
    }
}

// ---- Collisions -----------------------------------------------------------

// Returns the axis-aligned half-extents of the Lorentz-contracted bounding box.
static std::pair<float,float> contractedBounds(float hw, float hh,
                                                const BoostFrame& boost) {
    const Vec2 offs[4] = {{-hw,-hh},{+hw,-hh},{+hw,+hh},{-hw,+hh}};
    float x0 = hw, x1 = -hw, y0 = hh, y1 = -hh;
    for (const auto& o : offs) {
        Vec2 c = relativity::contractOffset(o, boost);
        if (c.x < x0) x0 = c.x;
        if (c.x > x1) x1 = c.x;
        if (c.y < y0) y0 = c.y;
        if (c.y > y1) y1 = c.y;
    }
    return {(x1 - x0) * 0.5f, (y1 - y0) * 0.5f};
}

static std::pair<float,float> bounds(float hw, float hh, bool lorentzianMode,
                                     const BoostFrame& boost) {
    return lorentzianMode ? contractedBounds(hw, hh, boost)
                          : std::pair<float,float>{hw, hh};
}

void Simulation::resolvePlayerHits(bool lorentzianMode, const BoostFrame& boost) {
    auto [sHW, sHH] = bounds(Ship::HW, Ship::HH, lorentzianMode, boost);
    for (auto& b : enemyBolts_) {
        if (!b.alive || gameOver_) continue;
        auto [bHW, bHH] = bounds(b.hw, b.hh, lorentzianMode, boost);
        if (std::fabs(b.pos.x - ship_.pos.x) < sHW + bHW &&
            std::fabs(b.pos.y - ship_.pos.y) < sHH + bHH) {
            b.alive = false;
            --health_;
            playerHitTimer_ = PLAYER_HIT_FLASH;
            // Impact point: leading edge of the bolt, clamped onto the hull.
            Vec2 hit = { std::clamp(b.pos.x, ship_.pos.x - sHW, ship_.pos.x + sHW),
                         std::clamp(b.pos.y + b.hh * 0.5f, ship_.pos.y - sHH, ship_.pos.y + sHH) };
            spawnHitEffect(hit, ship_.vel, /*onPlayer=*/true);
            if (health_ <= 0) {
                health_ = 0;
                gameOver_ = true;
                spawnExplosion(ship_.pos, 1.5f);
                emit(SimEventType::PlayerDestroyed, ship_.pos);
            } else {
                emit(SimEventType::PlayerHit, hit);
            }
        }
    }
}

void Simulation::resolveCollisions(bool lorentzianMode, const BoostFrame& boost) {
    auto [eHW, eHH] = bounds(Enemy::HW, Enemy::HH, lorentzianMode, boost);

    for (auto& b : bolts_) {
        if (!b.alive) continue;
        if (b.pos.y < -20.0f || b.pos.y > 640.0f ||
            b.pos.x < -20.0f || b.pos.x > 820.0f) {
            b.alive = false;
            continue;
        }
        auto [bHW, bHH] = bounds(b.hw, b.hh, lorentzianMode, boost);

        for (auto& e : enemies_) {
            if (!e.alive) continue;
            if (std::fabs(b.pos.x - e.pos.x) < eHW + bHW &&
                std::fabs(b.pos.y - e.pos.y) < eHH + bHH) {
                b.alive = false;
                e.alive = false;
                score_ += e.scoreValue();
                spawnExplosion(e.pos);
                emit(SimEventType::EnemyDestroyed, e.pos, EnemyKind::Formation);
                break;
            }
        }
        if (!b.alive) continue;

        for (auto& ep : hostiles_) {
            Enemy& e = *ep;
            if (!e.alive) continue;
            auto [hw, hh] = bounds(e.halfW(), e.halfH(), lorentzianMode, boost);
            if (std::fabs(b.pos.x - e.pos.x) < hw + bHW &&
                std::fabs(b.pos.y - e.pos.y) < hh + bHH) {
                b.alive = false;
                Vec2 hit = { std::clamp(b.pos.x, e.pos.x - hw, e.pos.x + hw),
                             std::clamp(b.pos.y - b.hh * 0.5f, e.pos.y - hh, e.pos.y + hh) };
                spawnHitEffect(hit, e.vel, /*onPlayer=*/false);
                if (e.takeHit()) {
                    score_ += e.scoreValue();
                    float scale = (e.kind() == EnemyKind::Heavy)   ? 1.8f
                                : (e.kind() == EnemyKind::Fighter) ? 1.2f : 1.0f;
                    spawnExplosion(e.pos, scale);
                    emit(SimEventType::EnemyDestroyed, e.pos, e.kind());
                } else {
                    emit(SimEventType::EnemyHit, hit, e.kind());
                }
                break;
            }
        }
    }
}

// ---- Player fire ----------------------------------------------------------

void Simulation::spawnBolt(float c_game, bool lorentzianMode) {
    Bolt proto;
    proto.alive     = true;
    proto.spawnTime = labTime_;

    if (!lorentzianMode) {
        proto.vel     = { ship_.vel.x, ship_.vel.y - BOLT_MUZZLE };
        proto.doppler = 1.0f;
    } else {
        BoostFrame boost = BoostFrame::fromVelocity(ship_.vel, c_game);
        Vec2 aimShip = { 0.0f, -1.0f };
        Vec2 labDir  = relativity::aberrate(aimShip, boost, /*toShip=*/false);
        proto.vel = labDir * c_game;
        float cosTheta = boost.n.dot(labDir);
        proto.doppler  = relativity::dopplerFactor(cosTheta, boost);
    }

    Vec2 muzzle = { ship_.pos.x, ship_.pos.y - 32.0f };
    if (twinShotTime_ > 0.0f) {
        for (float dx : { -TWIN_OFFSET_X, +TWIN_OFFSET_X }) {
            Bolt b = proto;
            b.pos  = { muzzle.x + dx, muzzle.y };
            bolts_.push_back(b);
            spawnMuzzleFlash(b.pos, ship_.vel, YELLOW, /*trackShip=*/true);
        }
    } else {
        Bolt b = proto;
        b.pos  = muzzle;
        bolts_.push_back(b);
        spawnMuzzleFlash(b.pos, ship_.vel, YELLOW, /*trackShip=*/true);
    }
}

void Simulation::updateFormation(float dt) {
    bool hitLeft = false, hitRight = false;
    for (const auto& e : enemies_) {
        if (!e.alive) continue;
        if (e.pos.x - Enemy::HW <= 50.0f)  hitLeft  = true;
        if (e.pos.x + Enemy::HW >= 750.0f) hitRight = true;
    }
    if ((hitLeft && formationVelX_ < 0.0f) || (hitRight && formationVelX_ > 0.0f)) {
        formationVelX_ = -formationVelX_;
        for (auto& e : enemies_) {
            if (e.alive) e.pos.y += 12.0f;
        }
    }

    for (auto& e : enemies_) {
        if (!e.alive) continue;
        e.vel = { formationVelX_, 0.0f };
        e.pos = e.pos + e.vel * dt;
        e.posTime = labTime_;
    }
}

// ---- Powerups -------------------------------------------------------------

void Simulation::spawnPowerup() {
    float x = 30.0f + frand() * 740.0f;
    // Weighted pick; heals are rarer at full health, boosts vanish at the cap.
    float wBoost = (ship_.gammaMax < SHIP_GAMMA_MAX_CAP - 1e-3f) ? 1.0f : 0.0f;
    float wHeal  = (health_ < PLAYER_MAX_HEALTH) ? 1.0f : 0.25f;
    float wTwin  = 1.0f;
    float r = frand() * (wBoost + wHeal + wTwin);
    if (r < wBoost)
        powerups_.push_back(std::make_unique<BoostPowerup>(x));
    else if (r < wBoost + wHeal)
        powerups_.push_back(std::make_unique<HealPowerup>(x));
    else
        powerups_.push_back(std::make_unique<TwinShotPowerup>(x));
}

void Simulation::updatePowerups(float dt) {
    powerupSpawnTimer_ -= dt;
    if (powerupSpawnTimer_ <= 0.0f) {
        spawnPowerup();
        powerupSpawnTimer_ = POWERUP_INTERVAL;
    }
    for (auto& p : powerups_) {
        if (!p->alive) continue;
        p->tick(dt);
        float dx = p->pos.x - ship_.pos.x;
        float dy = p->pos.y - ship_.pos.y;
        if (dx*dx + dy*dy < (Powerup::RADIUS + Ship::HW) * (Powerup::RADIUS + Ship::HW)) {
            p->activate(*this);
            p->alive = false;
            emit(SimEventType::PowerupCollected, p->pos);
        }
    }
}

// ---- Effects --------------------------------------------------------------

void Simulation::spawnExplosion(Vec2 pos, float scale) {
    Explosion ex;
    ex.pos   = pos;
    ex.scale = scale;
    ex.seed  = static_cast<unsigned>(rand());
    explosions_.push_back(ex);
}

void Simulation::updateExplosions(float dt) {
    for (auto& ex : explosions_) {
        if (!ex.alive) continue;
        ex.age += dt;
        if (ex.age >= Explosion::DURATION) ex.alive = false;
    }
}

void Simulation::spawnMuzzleFlash(Vec2 pos, Vec2 vel, Color color, bool trackShip) {
    muzzleFlashes_.push_back({pos, vel, 0.0f, true, trackShip, color});
}

void Simulation::updateMuzzleFlashes(float dt) {
    for (auto& f : muzzleFlashes_) {
        if (!f.alive) continue;
        f.age += dt;
        if (f.age >= MuzzleFlash::DURATION) { f.alive = false; continue; }
        if (f.trackShip) f.vel = ship_.vel;
        f.pos = f.pos + f.vel * dt;
    }
}

void Simulation::spawnHitEffect(Vec2 pos, Vec2 vel, bool onPlayer) {
    HitEffect h;
    h.pos      = pos;
    h.vel      = vel;
    h.offset   = pos - ship_.pos;
    h.onPlayer = onPlayer;
    h.rotation = frand() * 360.0f;
    hitEffects_.push_back(h);
}

// Runs after collisions, so effects spawned this frame start at age 0 and
// their pos is already valid at labTime_.
void Simulation::updateHitEffects(float dt) {
    for (auto& h : hitEffects_) {
        if (!h.alive) continue;
        if (h.age > 0.0f || h.onPlayer) {
            if (h.onPlayer) {
                h.vel = ship_.vel;
                h.pos = ship_.pos + h.offset;
            } else {
                h.pos = h.pos + h.vel * dt;
            }
        }
        h.age += dt;
        if (h.age >= HitEffect::DURATION) h.alive = false;
    }
}

template <typename T, typename Pred>
static void eraseIf(std::vector<T>& v, Pred p) {
    v.erase(std::remove_if(v.begin(), v.end(), p), v.end());
}

void Simulation::pruneDead() {
    eraseIf(hostiles_,      [](const auto& e) { return !e->alive; });
    eraseIf(powerups_,      [](const auto& p) { return !p->alive; });
    eraseIf(bolts_,         [](const Bolt& b) { return !b.alive; });
    eraseIf(enemyBolts_,    [](const Bolt& b) { return !b.alive; });
    eraseIf(explosions_,    [](const Explosion& e) { return !e.alive; });
    eraseIf(muzzleFlashes_, [](const MuzzleFlash& f) { return !f.alive; });
    eraseIf(hitEffects_,    [](const HitEffect& h) { return !h.alive; });
}

// ---- Powerup callbacks ----------------------------------------------------

void Simulation::applyHeal(int hp) {
    health_ = std::min(health_ + hp, PLAYER_MAX_HEALTH);
}

void Simulation::applyBoost(float delta) {
    ship_.gammaMax = std::min(ship_.gammaMax + delta, SHIP_GAMMA_MAX_CAP);
}

void Simulation::applyTwinShot(float properSeconds) {
    // A second pickup refreshes the timer rather than stacking.
    twinShotTime_ = std::max(twinShotTime_, properSeconds);
}
