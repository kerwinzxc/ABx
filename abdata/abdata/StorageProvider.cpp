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


#include "StorageProvider.h"
#include "DBAll.h"
#include <AB/Entities/Party.h>
#include <abscommon/Dispatcher.h>
#include <abscommon/Profiler.h>
#include <abscommon/Scheduler.h>
#include <abscommon/ThreadPool.h>
#include <sstream>

inline constexpr size_t KEY_CHARACTERS_HASH = sa::StringHash(AB::Entities::Character::KEY());
inline constexpr size_t KEY_SERVICE_HASH = sa::StringHash(AB::Entities::Service::KEY());
inline constexpr size_t KEY_SERVICELIST_HASH = sa::StringHash(AB::Entities::ServiceList::KEY());
inline constexpr size_t KEY_GAMEINSTANCES_HASH = sa::StringHash(AB::Entities::GameInstance::KEY());
inline constexpr size_t KEY_PARTIES_HASH = sa::StringHash(AB::Entities::Party::KEY());
inline constexpr size_t KEY_ACCOUNTITEMLIST_HASH = sa::StringHash(AB::Entities::AccountItemList::KEY());
inline constexpr size_t KEY_PLAYERITEMLIST_HASH = sa::StringHash(AB::Entities::PlayerItemList::KEY());
inline constexpr size_t KEY_INVENTORYITEMLIST_HASH = sa::StringHash(AB::Entities::InventoryItems::KEY());
inline constexpr size_t KEY_EQUIPPEDITEMLIST_HASH = sa::StringHash(AB::Entities::EquippedItems::KEY());
inline constexpr size_t KEY_TYPEDITEMLIST_HASH = sa::StringHash(AB::Entities::TypedItemList::KEY());
inline constexpr size_t KEY_ITEMPRICE_HASH = sa::StringHash(AB::Entities::ItemPrice::KEY());

StorageProvider::StorageProvider(size_t maxSize, bool readonly) :
    flushInterval_(FLUSH_CACHE_MS),
    cleanInterval_(CLEAN_CACHE_MS),
    readonly_(readonly),
    running_(true),
    maxSize_(maxSize),
    currentSize_(0),
    cache_()
{
    InitEnitityClasses();
    auto sched = GetSubsystem<Asynch::Scheduler>();
    sched->Add(
        Asynch::CreateScheduledTask(FLUSH_CACHE_MS, std::bind(&StorageProvider::FlushCacheTask, this))
    );
    sched->Add(
        Asynch::CreateScheduledTask(CLEAN_CACHE_MS, std::bind(&StorageProvider::CleanTask, this))
    );
    sched->Add(
        Asynch::CreateScheduledTask(CLEAR_PRICES_MS, std::bind(&StorageProvider::ClearPricesTask, this))
    );
}

void StorageProvider::InitEnitityClasses()
{
    AddEntityClass<DB::DBAccount, AB::Entities::Account>();
    AddEntityClass<DB::DBCharacter, AB::Entities::Character>();
    AddEntityClass<DB::DBGame, AB::Entities::Game>();
    AddEntityClass<DB::DBGameList, AB::Entities::GameList>();
    AddEntityClass<DB::DBIpBan, AB::Entities::IpBan>();
    AddEntityClass<DB::DBAccountBan, AB::Entities::AccountBan>();
    AddEntityClass<DB::DBBan, AB::Entities::Ban>();
    AddEntityClass<DB::DBFriendList, AB::Entities::FriendList>();
    AddEntityClass<DB::DBAccountKey, AB::Entities::AccountKey>();
    AddEntityClass<DB::DBAccountKeyAccounts, AB::Entities::AccountKeyAccounts>();
    AddEntityClass<DB::DBMail, AB::Entities::Mail>();
    AddEntityClass<DB::DBMailList, AB::Entities::MailList>();
    AddEntityClass<DB::DBProfession, AB::Entities::Profession>();
    AddEntityClass<DB::DBSkill, AB::Entities::Skill>();
    AddEntityClass<DB::DBEffect, AB::Entities::Effect>();
    AddEntityClass<DB::DBAttribute, AB::Entities::Attribute>();
    AddEntityClass<DB::DBSkillList, AB::Entities::SkillList>();
    AddEntityClass<DB::DBEffectList, AB::Entities::EffectList>();
    AddEntityClass<DB::DBProfessionList, AB::Entities::ProfessionList>();
    AddEntityClass<DB::DBVersion, AB::Entities::Version>();
    AddEntityClass<DB::DBAttributeList, AB::Entities::AttributeList>();
    AddEntityClass<DB::DBAccountList, AB::Entities::AccountList>();
    AddEntityClass<DB::DBCharacterList, AB::Entities::CharacterList>();
    AddEntityClass<DB::DBService, AB::Entities::Service>();
    AddEntityClass<DB::DBServicelList, AB::Entities::ServiceList>();
    AddEntityClass<DB::DBGuild, AB::Entities::Guild>();
    AddEntityClass<DB::DBGuildMembers, AB::Entities::GuildMembers>();
    AddEntityClass<DB::DBReservedName, AB::Entities::ReservedName>();
    AddEntityClass<DB::DBItem, AB::Entities::Item>();
    AddEntityClass<DB::DBItemList, AB::Entities::ItemList>();
    AddEntityClass<DB::DBVersionList, AB::Entities::VersionList>();
    AddEntityClass<DB::DBAccountKeyList, AB::Entities::AccountKeyList>();
    AddEntityClass<DB::DBMusic, AB::Entities::Music>();
    AddEntityClass<DB::DBMusicList, AB::Entities::MusicList>();
    AddEntityClass<DB::DBConcreteItem, AB::Entities::ConcreteItem>();
    AddEntityClass<DB::DBAccountItemList, AB::Entities::ChestItems>();
    AddEntityClass<DB::DBPlayerItemList, AB::Entities::InventoryItems>();
    AddEntityClass<DB::DBPlayerItemList, AB::Entities::EquippedItems>();
    AddEntityClass<DB::DBItemChanceList, AB::Entities::ItemChanceList>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsInsignia>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsRunes>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsWeaponPrefix>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsWeaponSuffix>();
    AddEntityClass<DB::DBTypedItemList, AB::Entities::TypedItemsWeaponInscription>();
    AddEntityClass<DB::DBInstance, AB::Entities::GameInstance>();
    AddEntityClass<DB::DBFriendedMe, AB::Entities::FriendedMe>();
    AddEntityClass<DB::DBQuest, AB::Entities::Quest>();
    AddEntityClass<DB::DBQuestList, AB::Entities::QuestList>();
    AddEntityClass<DB::DBPlayerQuest, AB::Entities::PlayerQuest>();
    AddEntityClass<DB::DBPlayerQuestList, AB::Entities::PlayerQuestList>();
    AddEntityClass<DB::DBPlayerQuestListRewarded, AB::Entities::PlayerQuestListRewarded>();
    AddEntityClass<DB::DBMerchantItemList, AB::Entities::MerchantItemList>();
    AddEntityClass<DB::DBMerchantItem, AB::Entities::MerchantItem>();
    AddEntityClass<DB::DBItemPrice, AB::Entities::ItemPrice>();
}

bool StorageProvider::Create(const IO::DataKey& key, ea::shared_ptr<StorageData> data)
{
    auto _data = cache_.find(key);

    if (_data != cache_.end())
    {
        // If there is a deleted record we must delete it from DB now or we may get
        // a constraint violation.
        if (IsDeleted((*_data).second.flags))
            FlushData(key);
        else
            // Already exists
            return false;
    }

    std::string table;
    uuids::uuid _id;
    if (!key.decode(table, _id))
    {
        LOG_ERROR << "Unable to decode key, key size " << key.data_.size() << std::endl;
        return false;
    }
    if (_id.nil())
    {
        LOG_ERROR << "UUID is nil, table is " << table << std::endl;
        return false;
    }

    CacheData(table, _id, data, 0);

    // Unfortunately we must flush the data for create operations. Or we find a way
    // to check constraints, unique columns etc.
    if (!FlushData(key))
    {
        // Failed delete from cache
        RemoveData(key);
        return false;
    }
    return true;
}

bool StorageProvider::Update(const IO::DataKey& key, ea::shared_ptr<StorageData> data)
{
    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }

    if (id.nil())
        // Does not exist
        return false;

    auto _data = cache_.find(key);
    // The only possibility to update a not yet created entity is when its in cache and not flushed.
    bool isCreated = true;
    if (_data != cache_.end())
        isCreated = IsCreated((*_data).second.flags);

    // The client sets the data so this is not stored in DB
    CacheData(table, id, data, CacheFlag::Modified | (isCreated ? CacheFlag::Created : 0));
    return true;
}

void StorageProvider::CacheData(const std::string& table, const uuids::uuid& id,
    ea::shared_ptr<StorageData> data, CacheFlags flags)
{
    size_t sizeNeeded = data->size();
    if (!EnoughSpace(sizeNeeded))
    {
        // Create space later
        GetSubsystem<Asynch::Dispatcher>()->Add(
            Asynch::CreateTask(std::bind(&StorageProvider::CreateSpace, this, sizeNeeded))
        );
    }

    const IO::DataKey key(table, id);

    if (cache_.find(key) == cache_.end())
    {
        index_.Add(key);
        currentSize_ += data->size();
    }
    else
    {
        index_.Refresh(key);
        currentSize_ = (currentSize_ - cache_[key].data->size()) + data->size();
    }

    cache_[key] = { flags, data };

    const size_t tableHash = sa::StringHashRt(table.c_str());
    // Special case for player names
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        AB::Entities::Character ch;
        if (GetEntity(*data, ch))
            namesCache_.Add(key, ch.name);
    }
}

bool StorageProvider::Read(const IO::DataKey& key, ea::shared_ptr<StorageData> data)
{
    auto _data = cache_.find(key);
    if (_data != cache_.end())
    {
        if (IsDeleted((*_data).second.flags))
            // Don't return deleted items that are in cache
            return false;
        data->assign((*_data).second.data->begin(), (*_data).second.data->end());
        return true;
    }

    std::string table;
    uuids::uuid _id;
    if (!key.decode(table, _id))
    {
        LOG_ERROR << "Unable to decode key" << std::endl;
        return false;
    }

    // Maybe in player names cache
    // Special case for player names
    size_t tableHash = sa::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        AB::Entities::Character ch;
        if (GetEntity(*data, ch) && !ch.name.empty())
        {
            auto* nameKey = namesCache_.LookupName(KEY_CHARACTERS_HASH, ch.name);
            if (nameKey != nullptr)
            {
                _data = cache_.find(*nameKey);
                if (_data != cache_.end())
                {
                    if (IsDeleted((*_data).second.flags))
                        // Don't return deleted items that are in cache
                        return false;
                    data->assign((*_data).second.data->begin(), (*_data).second.data->end());
                    return true;
                }
            }
        }
    }

    // Really not in cache
    if (!LoadData(key, data))
        return false;

    if (_id.nil())
        // If no UUID given in key (e.g. when reading by name) cache with the proper key
        _id = GetUuid(*data);
    const IO::DataKey newKey(table, _id);
    auto _newdata = cache_.find(newKey);
    if (_newdata == cache_.end())
    {
        CacheData(table, _id, data, CacheFlag::Created);
    }
    else
    {
        // Was already cached
        if (IsDeleted((*_newdata).second.flags))
            // Don't return deleted items that are in cache
            return false;
        // Return the cached object, it may have changed
        data->assign((*_newdata).second.data->begin(), (*_newdata).second.data->end());
    }
    return true;

}

bool StorageProvider::Delete(const IO::DataKey& key)
{
    // You can only delete what you've loaded before
    auto data = cache_.find(key);
    if (data == cache_.end())
    {
        return false;
    }
    sa::bits::set((*data).second.flags, CacheFlag::Deleted);
    return true;
}

bool StorageProvider::Invalidate(const IO::DataKey& key)
{
    if (!FlushData(key))
    {
        LOG_ERROR << "Error flushing " << key.format() << std::endl;
        return false;
    }
    return RemoveData(key);
}

void StorageProvider::PreloadTask(IO::DataKey key)
{
    // Dispatcher thread

    auto _data = cache_.find(key);
    if (_data == cache_.end())
        return;

    std::string table;
    uuids::uuid _id;
    if (!key.decode(table, _id))
    {
        LOG_ERROR << "Unable to decode key " << key.format() << std::endl;
        return;
    }
    ea::shared_ptr<StorageData> data = ea::make_shared<StorageData>(0);
    if (!LoadData(key, data))
        return;

    if (_id.nil())
        // If no UUID given in key (e.g. when reading by name) cache with the proper key
        _id = GetUuid(*data);
    IO::DataKey newKey(table, _id);

    auto _newdata = cache_.find(newKey);
    if (_newdata == cache_.end())
    {
        CacheData(table, _id, data, CacheFlag::Created);
    }
}

bool StorageProvider::Preload(const IO::DataKey& key)
{
    auto _data = cache_.find(key);
    if (_data == cache_.end())
        return true;

    // Load later
    GetSubsystem<Asynch::Dispatcher>()->Add(
        Asynch::CreateTask(std::bind(&StorageProvider::PreloadTask, this, key))
    );
    return true;
}

bool StorageProvider::Exists(const IO::DataKey& key, ea::shared_ptr<StorageData> data)
{
    auto _data = cache_.find(key);

    if (_data != cache_.end())
        return !IsDeleted((*_data).second.flags);

    return ExistsData(key, *data);
}

bool StorageProvider::Clear(const IO::DataKey&)
{
    std::vector<IO::DataKey> toDelete;

    for (const auto& ci : cache_)
    {
        const IO::DataKey& key = ci.first;
        std::string table;
        uuids::uuid id;
        if (!key.decode(table, id))
            continue;
        size_t tableHash = sa::StringHashRt(table.data());
        if (tableHash == KEY_GAMEINSTANCES_HASH || tableHash == KEY_SERVICE_HASH || tableHash == KEY_PARTIES_HASH)
            // Can not delete these
            continue;

        if (FlushData(key))
        {
            currentSize_ -= ci.second.data->size();
            toDelete.push_back(key);
        }
    }
    for (const auto& k : toDelete)
    {
        cache_.erase(k);
        index_.Delete(k);
    }
    namesCache_.Clear();
    LOG_INFO << "Cleared cache, removed " << toDelete.size() << " items" << std::endl;
    return true;
}

void StorageProvider::Shutdown()
{
    std::mutex mutex;
    // The only thing that not called from the dispatcher thread, so lock it.
    std::scoped_lock lock(mutex);
    running_ = false;
    for (const auto& c : cache_)
        FlushData(c.first);

    DB::DBAccount::LogoutAll();
}

void StorageProvider::CleanCache()
{
    // Delete deleted records from DB and remove them from cache.
    if (cache_.size() == 0)
        return;
    size_t oldSize = currentSize_;
    int removed = 0;
    auto i = cache_.begin();
    while ((i = ea::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        return IsDeleted(current.second.flags);
    })) != cache_.end())
    {
        bool ok = true;
        const IO::DataKey& key = (*i).first;
        if (IsCreated((*i).second.flags))
        {
            // If it's in DB (created == true) update changed data in DB
            ok = FlushData(key);
        }
        if (ok)
        {
            // Remove from players cache
            RemovePlayerFromCache(key);
            currentSize_ -= (*i).second.data->size();
            index_.Delete(key);
            cache_.erase(i++);
            ++removed;
        }
        else
        {
            // Error, break for now and try  the next time.
            // In case of lost connection it would try forever.
            break;
        }
    }

    if (removed > 0)
    {
        LOG_INFO << "Cleaned cache old size " << Utils::ConvertSize(oldSize) <<
            " current size " << Utils::ConvertSize(currentSize_) <<
            " removed " << removed << " record(s)" << std::endl;
    }
}

void StorageProvider::CleanTask()
{
    CleanCache();
    DB::DBGuildMembers::DeleteExpired(this);
    DB::DBReservedName::DeleteExpired(this);
//    DB::DBConcreteItem::Clean(this);
    if (running_)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(cleanInterval_, std::bind(&StorageProvider::CleanTask, this))
        );
    }
}

void StorageProvider::ClearPrices()
{
    // Remove prices from cache for force recalculate
    auto i = cache_.begin();
    while ((i = ea::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        std::string table;
        uuids::uuid id;
        current.first.decode(table, id);
        const size_t tableHash = sa::StringHashRt(table.c_str());
        return (tableHash == KEY_ITEMPRICE_HASH);
    })) != cache_.end())
    {
        RemoveData((*i).first);
    }
}

void StorageProvider::FlushCache()
{
    if (cache_.size() == 0)
        return;

    int written = 0;
    auto i = cache_.begin();
    while ((i = ea::find_if(i, cache_.end(), [](const auto& current) -> bool
    {
        // Don't return deleted, these are flushed in CleanCache()
        if ((IsModified(current.second.flags) || !IsCreated(current.second.flags)) &&
            !IsDeleted(current.second.flags))
            return true;

        return false;
    })) != cache_.end())
    {
        ++written;
        const IO::DataKey& key = (*i).first;
        bool res = FlushData(key);
        if (!res)
        {
            LOG_WARNING << "Error flushing " << key.format() << std::endl;
            // Error, break for now and try  the next time.
            // In case of lost connection it would try forever.
            break;
        }
    }
    if (written > 0)
    {
        LOG_INFO << "Flushed cache wrote " << written << " record(s)" << std::endl;
    }
}

void StorageProvider::ClearPricesTask()
{
    ClearPrices();
    if (running_)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(CLEAR_PRICES_MS, std::bind(&StorageProvider::ClearPricesTask, this))
        );
    }
}

void StorageProvider::FlushCacheTask()
{
    DB::Database* db = GetSubsystem<DB::Database>();
    db->CheckConnection();
    FlushCache();
    if (running_)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(
            Asynch::CreateScheduledTask(flushInterval_, std::bind(&StorageProvider::FlushCacheTask, this))
        );
    }
}

uuids::uuid StorageProvider::GetUuid(StorageData& data)
{
    // Get UUID from raw data. UUID is serialized first as string
    const std::string suuid(data.begin() + 1,
        data.begin() + AB::Entities::Limits::MAX_UUID + 1);
    return uuids::uuid(suuid);
}

bool StorageProvider::EnoughSpace(size_t size)
{
    return (currentSize_ + size) <= maxSize_;
}

void StorageProvider::CreateSpace(size_t size)
{
    // Create more than required space
    const size_t sizeNeeded = size * 2;
    while ((currentSize_ + sizeNeeded) > maxSize_)
    {
        const IO::DataKey key = index_.Next();
        if (FlushData(key))
            RemoveData(key);
        else
            break;
    }
}

bool StorageProvider::RemoveData(const IO::DataKey& key)
{
    auto data = cache_.find(key);
    if (data != cache_.end())
    {
        RemovePlayerFromCache(key);

        currentSize_ -= (*data).second.data->size();
        cache_.erase(key);
        index_.Delete(key);

        return true;
    }
    return false;
}

bool StorageProvider::LoadData(const IO::DataKey& key,
    ea::shared_ptr<StorageData> data)
{
    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
    {
        LOG_ERROR << "Unable to decode key " << key.format() << std::endl;
        return false;
    }

    size_t tableHash = sa::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_PLAYERITEMLIST_HASH:
        // Use one bellow
        ASSERT_FALSE();
    case KEY_TYPEDITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_PARTIES_HASH:
        // Not written to DB
        return false;
    default:
    {
        if (loadCallables_.Exists(tableHash))
            return loadCallables_.Call(tableHash, id, *data);
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }
    }

    return false;
}

bool StorageProvider::FlushData(const IO::DataKey& key)
{
    if (readonly_)
    {
        LOG_WARNING << "READONLY: Nothing is written to the database" << std::endl;
        return true;
    }

    auto data = cache_.find(key);
    if (data == cache_.end())
        // Not in cache so no need to flush anything
        return true;

    CacheItem& item = (*data).second;

    // No need to save to DB when not modified
    if (!IsModified(item.flags) && !IsDeleted(item.flags) && IsCreated(item.flags))
        return true;

    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
    {
        LOG_ERROR << "Unable to decode key " << key.format() << std::endl;
        return false;
    }

    size_t tableHash = sa::StringHashRt(table.data());
    bool succ = false;

    switch (tableHash)
    {
    case KEY_ACCOUNTITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_PLAYERITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_TYPEDITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_SERVICELIST_HASH:
    case KEY_INVENTORYITEMLIST_HASH:
    case KEY_EQUIPPEDITEMLIST_HASH:
    case KEY_PARTIES_HASH:
        // Not written to DB
        // Mark not modified and created or it will infinitely try to flush it
        item.flags = CacheFlag::Created;
        succ = true;
        break;
    default:
    {
        if (flushCallables_.Exists(tableHash))
            succ = flushCallables_.Call(tableHash, item);
        else
        {
            LOG_ERROR << "Unknown table " << table << std::endl;
            return false;
        }
    }
    }

    if (!succ)
        LOG_ERROR << "Unable to write data" << std::endl;
    return succ;
}

bool StorageProvider::ExistsData(const IO::DataKey& key, StorageData& data)
{
    std::string table;
    uuids::uuid id;
    if (!key.decode(table, id))
        return false;

    size_t tableHash = sa::StringHashRt(table.data());
    switch (tableHash)
    {
    case KEY_ACCOUNTITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_PLAYERITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_TYPEDITEMLIST_HASH:
        ASSERT_FALSE();
    case KEY_PARTIES_HASH:
        // Not written to DB. If we are here its not in cache so does not exist
        return false;
    default:
        if (exitsCallables_.Exists(tableHash))
            return exitsCallables_.Call(tableHash, data);
        LOG_ERROR << "Unknown table " << table << std::endl;
        break;
    }

    return false;
}

void StorageProvider::RemovePlayerFromCache(const IO::DataKey& key)
{
    std::string table;
    uuids::uuid playerUuid;
    if (!key.decode(table, playerUuid))
        return;

    size_t tableHash = sa::StringHashRt(table.data());
    if (tableHash == KEY_CHARACTERS_HASH)
    {
        auto _data = cache_.find(key);
        if (_data == cache_.end())
            return;
        namesCache_.Delete(key);
    }
}
