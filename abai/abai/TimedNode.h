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

#pragma once

#include "Action.h"
#include <limits>
#include <stdint.h>

namespace AI {

inline constexpr uint32_t NOT_STARTED = std::numeric_limits<uint32_t>::max();

class TimedNode : public Action
{
protected:
    uint32_t millis_{ 0 };
protected:
    // Called by Action, but we don't use it
    Status DoAction(Agent&, uint32_t) override { return Status::CanNotExecute; }
public:
    explicit TimedNode(const ArgumentsType& arguments);
    ~TimedNode() override;
    Node::Status Execute(Agent& agent, uint32_t timeElapsed) final override;
    virtual Node::Status ExecuteStart(Agent& agent, uint32_t timeElapsed);
    virtual Node::Status ExecuteRunning(Agent& agent, uint32_t timeElapsed);
    virtual Node::Status ExecuteExpired(Agent& agent, uint32_t timeElapsed);
};

}
