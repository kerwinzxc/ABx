#pragma once

#include <stdint.h>
#include <vector>
#include <memory>
#include "Damage.h"

namespace Net {
class NetworkMessage;
}

namespace Game {

class Actor;
class Skill;

namespace Components {

class DamageComp
{
private:
    struct DamageItem
    {
        DamageType type;
        DamagePos pos;
        int value;
        uint32_t actorId;
        // The skill/effect causing this damage. If 0 it's a melee damage.
        uint32_t index;
        int64_t tick;
    };
    Actor& owner_;
    std::vector<DamageItem> damages_;
    std::weak_ptr<Actor> lastDamager_;
    std::weak_ptr<Actor> lastMeleeDamager_;
public:
    DamageComp() = delete;
    explicit DamageComp(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    DamageComp(const DamageComp&) = delete;
    DamageComp& operator=(const DamageComp&) = delete;
    ~DamageComp() = default;

    void Update(uint32_t /* timeElapsed */) { }
    void ApplyDamage(Actor* source, uint32_t index, DamageType type, int value, float penetration, bool melee);
    /// Steal life from this actor. The source must add the returned value to its life.
    int DrainLife(Actor* source, uint32_t index, int value);
    void Touch();
    DamagePos GetDamagePos() const;
    // Return true if we are currently getting melee damage
    bool IsGettingMeleeDamage() const;
    /// How long the actor didn't get damage in ms
    uint32_t NoDamageTime() const;
    std::shared_ptr<Actor> GetLastDamager() const { return lastDamager_.lock(); }
    std::shared_ptr<Actor> GetLastMeleeDamager() const { return lastMeleeDamager_.lock(); }
    bool IsLastDamager(const Actor& actor);
    void Write(Net::NetworkMessage& message);

    int64_t lastDamage_{ 0 };
    int64_t lastMeleeDamage_{ 0 };
};

}
}

