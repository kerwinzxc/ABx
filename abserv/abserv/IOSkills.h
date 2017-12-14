#pragma once

#include "SkillManager.h"

namespace IO {

class IOSkills
{
public:
    IOSkills() = delete;

    static bool Load(Game::SkillManager& manager, const std::string fileName);
};

}
