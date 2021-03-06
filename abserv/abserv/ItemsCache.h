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

#include <sa/IdGenerator.h>
#include <eastl.hpp>

namespace Game {

class Item;

// The owner of all items. Because Items move around alot, e.g. people trade items,
// store them in chest inventory, i.e. would change ownership, and changing ownerhip
// of unique_ptr's is a bit cumbersome, the real owners of the items store only the
// IDs in this cache.
class ItemsCache
{
private:
    static sa::IdGenerator<uint32_t> itemIds_;
    ea::unordered_map<uint32_t, ea::unique_ptr<Item>> itemCache_;
    ea::unordered_map<std::string, uint32_t> concreteIds_;
public:
    Item* Get(uint32_t id);
    uint32_t GetConcreteId(const std::string& uuid) const;
    uint32_t Add(ea::unique_ptr<Item>&& item);
    // Item IDs are not reused, so removing an item from the cache, the item also
    // disapperas from the players inventory/chest
    void Remove(uint32_t id);
    void RemoveConcrete(const std::string& uuid);
};

}
