#include "stdafx.h"
#include "Client.h"
#include "ProtocolLogin.h"
#include "ProtocolGame.h"
#include "Connection.h"

namespace Client {

Client::Client() :
    loginHost_("127.0.0.1"),
    gameHost_("127.0.0.1"),
    loginPort_(2748),
    gamePort_(2749),
    protoLogin_(nullptr),
    protoGame_(nullptr),
    state_(StateDisconnected)
{
}

Client::~Client()
{
    Connection::Terminate();
}

void Client::OnGetCharlist(const Charlist& chars)
{
    gamePort_ = protoLogin_->gamePort_;
    if (!protoLogin_->gameHost_.empty())
        gameHost_ = protoLogin_->gameHost_;
    state_ = StateSelecChar;
    if (receiver_)
        receiver_->OnGetCharlist(chars);
}

void Client::OnEnterWorld(const std::string& mapName)
{
    state_ = StateWorld;
    mapName_ = mapName;
    if (receiver_)
        receiver_->OnEnterWorld(mapName);
}

void Client::OnError(const std::error_code& err)
{
    if (receiver_)
        receiver_->OnNetworkError(err);
}

void Client::OnProtocolError(uint8_t err)
{
    if (receiver_)
        receiver_->OnProtocolError(err);
}

void Client::Login(const std::string& name, const std::string& pass)
{
    accountName_ = name;
    password_ = pass;

    // 1. Login to login server -> get character list
    protoLogin_ = std::make_shared<ProtocolLogin>();
    protoLogin_->SetErrorCallback(std::bind(&Client::OnError, this, std::placeholders::_1));
    protoLogin_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    protoLogin_->Login(loginHost_, loginPort_, name, pass,
        std::bind(&Client::OnGetCharlist, this, std::placeholders::_1));
    Connection::Run();
}

void Client::Logout()
{
    if (protoGame_)
        protoGame_->Logout();
}

void Client::EnterWorld(const std::string& charName, const std::string& map)
{
    // 2. Login to game server
    protoGame_ = std::make_shared<ProtocolGame>();
    protoGame_->SetErrorCallback(std::bind(&Client::OnError, this, std::placeholders::_1));
    protoGame_->SetProtocolErrorCallback(std::bind(&Client::OnProtocolError, this, std::placeholders::_1));
    protoGame_->Login(accountName_, password_, charName, map, gameHost_, gamePort_,
        std::bind(&Client::OnEnterWorld, this, std::placeholders::_1));
    Connection::Run();
}

void Client::Update()
{
    Connection::Poll();
}

}
