#include "stdafx.h"
#include "Party.h"
#include "Player.h"
#include "Chat.h"

namespace Game {

uint32_t Party::partyIds_ = 0;

Party::Party(std::shared_ptr<Player> leader) :
    leader_(leader),
    maxMembers_(1)
{
    id_ = GetNewId();
    chatChannel_ = std::dynamic_pointer_cast<PartyChatChannel>(Chat::Instance.Get(ChannelParty, id_));
    chatChannel_->party_ = this;
    members_.push_back(leader);
}

Party::~Party()
{
    Chat::Instance.Remove(ChannelParty, id_);
}

bool Party::Add(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (members_.size() == maxMembers_)
        return false;
    if (IsMember(player))
        return false;

    // Remove from existing party
    player->GetParty()->Remove(player);
    // TODO: shared_from_this -> Exception?
    members_.push_back(player);
    player->SetParty(shared_from_this());
    RemoveInvite(player);
    return true;
}

bool Party::Remove(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    auto it = std::find_if(members_.begin(), members_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    if (it == members_.end())
        return false;
    members_.erase(it);
    player->SetParty(std::shared_ptr<Party>());
    if (auto l = leader_.lock())
    {
        if (player->id_ == l->id_)
        {
            // Need new leader
            if (!members_.empty())
            {
                leader_ = members_.front().lock();
            }
        }
    }
    return true;
}

bool Party::Invite(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    if (IsMember(player) || IsInvited(player))
        return false;

    invited_.push_back(player);
    return true;
}

bool Party::RemoveInvite(std::shared_ptr<Player> player)
{
    if (!player)
        return false;

    auto it = std::find_if(invited_.begin(), invited_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    if (it == invited_.end())
        return false;
    invited_.erase(it);
    return true;
}

void Party::ClearInvites()
{
    invited_.clear();
}

void Party::WriteToMembers(const Net::NetworkMessage& message)
{
    for (auto& wm : members_)
    {
        if (auto sm = wm.lock())
            sm->client_->WriteToOutput(message);
    }
}

void Party::SetPartySize(uint32_t size)
{
    while (members_.size() > size)
    {
        auto it = members_.back();
        Remove(it.lock());
    }
}

bool Party::IsMember(std::shared_ptr<Player> player) const
{
    auto it = std::find_if(members_.begin(), members_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    return it != members_.end();
}

bool Party::IsInvited(std::shared_ptr<Player> player) const
{
    auto it = std::find_if(invited_.begin(), invited_.end(), [&player](const std::weak_ptr<Player>& current)
    {
        if (const auto& c = current.lock())
        {
            return c->id_ == player->id_;
        }
        return false;
    });
    return it != invited_.end();
}

bool Party::IsLeader(const Player* const player)
{
    if (auto l = leader_.lock())
        return l.get() == player;
    return false;
}

}
