/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "AiSelectVisible.h"
#include "../Npc.h"

namespace AI {
namespace Filters {

SelectVisible::SelectVisible(const ArgumentsType& arguments) :
    Filter(arguments)
{
    std::string ff;
    if (GetArgument<std::string>(arguments, 0, ff))
    {
        if (ff.compare("friend") == 0)
            class_ = Game::TargetClass::Friend;
        else if (ff.compare("foe") == 0)
            class_ = Game::TargetClass::Foe;
    }
    GetArgument<Game::Ranges>(arguments, 1, range_);
}

void SelectVisible::Execute(Agent& agent)
{
    auto& entities = agent.filteredAgents_;
    entities.clear();
    const Game::Npc& chr = GetNpc(agent);
    chr.VisitInRange(range_, [&](const Game::GameObject& o)
    {
        if (!Game::Is<Game::Actor>(o))
            return Iteration::Continue;

        const Game::Actor& actor = Game::To<Game::Actor>(o);

        if (!Game::TargetClassMatches(chr, class_, actor))
            return Iteration::Continue;

        if (chr.IsObjectInSight(o))
            entities.push_back(o.id_);
        return Iteration::Continue;
    });
    entities.erase(std::unique(entities.begin(), entities.end()), entities.end());
}

}
}
