#include "stdafx.h"
#include "DBServicelList.h"
#include "Database.h"

namespace DB {

bool DBServicelList::Create(AB::Entities::ServiceList&)
{
    return true;
}

bool DBServicelList::Load(AB::Entities::ServiceList& sl)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT `uuid` FROM `services`";
    for (std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        sl.uuids.push_back(result->GetString("uuid"));
    }
    return true;
}

bool DBServicelList::Save(const AB::Entities::ServiceList&)
{
    return true;
}

bool DBServicelList::Delete(const AB::Entities::ServiceList&)
{
    return true;
}

bool DBServicelList::Exists(const AB::Entities::ServiceList&)
{
    return true;
}

}
