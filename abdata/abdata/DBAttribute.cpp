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

#include "DBAttribute.h"
#include <sa/TemplateParser.h>

namespace DB {

static std::string PlaceholderCallback(Database* db, const AB::Entities::Attribute& attr, const sa::templ::Token& token)
{
    switch (token.type)
    {
    case sa::templ::Token::Type::Variable:
        if (token.value == "uuid")
            return db->EscapeString(attr.uuid);
        if (token.value == "idk")
            return std::to_string(attr.index);
        if (token.value == "profession_uuid")
            return db->EscapeString(attr.professionUuid);
        if (token.value == "name")
            return db->EscapeString(attr.name);
        if (token.value == "is_primary")
            return std::to_string(attr.isPrimary ? 1 : 0);

        LOG_WARNING << "Unhandled placeholder " << token.value << std::endl;
        return "";
    default:
        return token.value;
    }
}

bool DBAttribute::Create(AB::Entities::Attribute&)
{
    // Do nothing
    return true;
}

bool DBAttribute::Load(AB::Entities::Attribute& attr)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT * FROM game_attributes WHERE ");

    if (!Utils::Uuid::IsEmpty(attr.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (attr.index != AB::Entities::INVALID_INDEX)
        parser.Append("idx = ${idx}", tokens);
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, attr, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;

    attr.uuid = result->GetString("uuid");
    attr.index = result->GetUInt("idx");
    attr.professionUuid = result->GetString("profession_uuid");
    attr.name = result->GetString("name");
    attr.isPrimary = result->GetUInt("is_primary") != 0;

    return true;
}

bool DBAttribute::Save(const AB::Entities::Attribute&)
{
    // Do nothing
    return true;
}

bool DBAttribute::Delete(const AB::Entities::Attribute&)
{
    // Do nothing
    return true;
}

bool DBAttribute::Exists(const AB::Entities::Attribute& attr)
{
    Database* db = GetSubsystem<Database>();

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("SELECT COUNT(*) AS count FROM game_attributes WHERE ");

    if (!Utils::Uuid::IsEmpty(attr.uuid))
        parser.Append("uuid = ${uuid}", tokens);
    else if (attr.index != AB::Entities::INVALID_INDEX)
        parser.Append("idx = ${idx}", tokens);
    else
    {
        LOG_ERROR << "UUID and index are empty" << std::endl;
        return false;
    }
    const std::string query = tokens.ToString(std::bind(&PlaceholderCallback, db, attr, std::placeholders::_1));

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query);
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
