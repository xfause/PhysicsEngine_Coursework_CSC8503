#pragma once
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"
#include "../../Common/Window.h"

#include "TutorialGame.h"
#include "GameStates.h"

using namespace NCL;
using namespace CSC8503;

class GameStateManager
{
public:
	void Update(float dt);
	GameStateManager();
	~GameStateManager();

	bool isRunning() { return running; }

protected:
	TutorialGame* g;
	PushdownMachine* machine;
	bool running;
};