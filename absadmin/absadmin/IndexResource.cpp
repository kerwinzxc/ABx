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


#include "IndexResource.h"
#include "Application.h"
#include <sa/StringHash.h>
#include "Version.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/AccountList.h>
#include <AB/Entities/CharacterList.h>

namespace Resources {

bool IndexResource::GetContext(LuaContext& objects)
{
    if (!TemplateResource::GetContext(objects))
        return false;
    bool loggedIn = session_->values_[sa::StringHashRt("logged_in")].GetBool();
    if (!loggedIn)
        return true;

    auto dataClient = GetSubsystem<IO::DataClient>();
    AB::Entities::AccountList al;
    if (!dataClient->Read(al))
        return false;

    kaguya::State& state = objects.GetState();
    state["total_accounts"] = al.uuids.size();

    AB::Entities::CharacterList cl;
    if (!dataClient->Read(cl))
        return false;
    state["total_chars"] = cl.uuids.size();

    return true;
}

IndexResource::IndexResource(std::shared_ptr<HttpsServer::Request> request) :
    TemplateResource(request)
{
    bool loggedIn = session_->values_[sa::StringHashRt("logged_in")].GetBool();
    if (loggedIn)
        template_ = "../templates/dashboard.lpp";
    else
        template_ = "../templates/login.lpp";
}

}
