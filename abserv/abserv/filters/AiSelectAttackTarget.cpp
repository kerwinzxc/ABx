#include "stdafx.h"
#include "AiSelectAttackTarget.h"
#include "../Npc.h"
#include "../Mechanic.h"

namespace AI {
namespace Filters {

void SelectAttackTarget::Execute(Agent& agent)
{
    AgentIds candidates;
    Game::Npc& chr = GetNpc(agent);
    std::map<uint32_t, float> sorting;
    chr.VisitEnemiesInRange(Game::Ranges::Aggro, [&](const Game::Actor& o)
    {
        candidates.push_back(o.id_);
        // Bigger is more likely to be selected
        sorting[o.id_] = chr.GetAggro(&o);
        return Iteration::Continue;
    });

    auto& entities = agent.filteredAgents_;
    if (entities.size() == 1)
    {
        // Only one already selected
        uint32_t selected = entities.front();
        const auto it = std::find_if(candidates.begin(), candidates.end(), [&selected] (uint32_t current)
        {
            return selected == current;
        });
        // If this selected is also in the candidates list, use this, regardless of the score.
        // This prevents switching targets.
        if (it != candidates.end())
            return;
    }
    entities.clear();
    if (candidates.size() == 0)
        return;

    std::sort(candidates.begin(), candidates.end(), [&sorting](uint32_t i, uint32_t j)
    {
        const float& p1 = sorting[i];
        const float& p2 = sorting[j];
        return p1 > p2;
    });

    entities.push_back(candidates.front());
}

}
}
