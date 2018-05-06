#pragma once

#include "Skill.h"

namespace Game {

class SkillManager
{
private:
    std::map<uint32_t, std::unique_ptr<Skill>> skillCache_;
public:
    SkillManager();
    ~SkillManager() = default;

    std::shared_ptr<Skill> Get(uint32_t index);
public:
    static SkillManager Instance;
};

}
