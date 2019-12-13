#include "stdafx.h"
#include "DBPlayerQuest.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBPlayerQuest::Create(AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `player_quests` (`uuid`, `quests_uuid`, `player_uuid`, `completed`, `rewarded, `progress`";
    query << ") VALUES (";

    query << db->EscapeString(g.uuid) << ", ";
    query << db->EscapeString(g.questUuid) << ", ";
    query << db->EscapeString(g.playerUuid) << ", ";
    query << (g.completed ? 1 : 0) << ", ";
    query << (g.rewarded ? 1 : 0) << ", ";
    query << g.progress;

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBPlayerQuest::Load(AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    std::ostringstream query;
    Database* db = GetSubsystem<Database>();
    query << "SELECT * FROM `player_quests` WHERE ";
    query << "`uuid` = " << db->EscapeString(g.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    g.uuid = result->GetString("uuid");
    g.playerUuid = result->GetString("player_uuid");
    g.questUuid = result->GetString("quests_uuid");
    g.completed = result->GetUInt("completed") != 0;
    g.rewarded = result->GetUInt("rewarded") != 0;
    g.progress = result->GetString("progress");

    return true;
}

bool DBPlayerQuest::Save(const AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `player_quests` SET ";

    // Only these may be changed
    query << " `completed` = " << (g.completed ? 1 : 0) << ", ";
    query << " `rewarded` = " << (g.rewarded ? 1 : 0) << ", ";
    query << " `progress` = " << db->EscapeString(g.progress);

    query << " WHERE `uuid` = " << db->EscapeString(g.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBPlayerQuest::Delete(const AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `player_quests` WHERE `uuid` = " << db->EscapeString(g.uuid);
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBPlayerQuest::Exists(const AB::Entities::PlayerQuest& g)
{
    if (Utils::Uuid::IsEmpty(g.uuid) || Utils::Uuid::IsEmpty(g.questUuid))
    {
        LOG_ERROR << "UUID required" << std::endl;
        return false;
    }
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `player_quests` WHERE ";
    query << "`uuid` = " << db->EscapeString(g.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}