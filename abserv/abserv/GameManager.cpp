#include "stdafx.h"
#include "GameManager.h"

GameManager GameManager::Instance;

GameManager::GameManager()
{
}


GameManager::~GameManager()
{
}

void GameManager::Start(ServiceManager* serviceManager)
{
    serviceManager_ = serviceManager;
}
