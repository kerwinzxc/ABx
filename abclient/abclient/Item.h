#pragma once

#include <AB/Entities/Item.h>

class Item : public Object
{
    URHO3D_OBJECT(Item, Object);
public:
    Item(Context* context);
    ~Item();

    template <class T>
    T* GetObjectResource()
    {
        if (objectFile_.Empty())
            return nullptr;
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        return cache->GetResource<T>(objectFile_);
    }
    template <class T>
    T* GetIconResource()
    {
        if (iconFile_.Empty())
            return nullptr;
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        return cache->GetResource<T>(iconFile_);
    }

    String uuid_;
    uint32_t index_;
    String name_;
    /// Prefab file
    String objectFile_;
    /// UI icon file, somewhere in /Textures
    String iconFile_;
    AB::Entities::ItemType type_;
    AB::Entities::ModelClass modelClass_;
    bool stackAble_;
};

