#include "GameStates.h"

#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"

using namespace NCL;
using namespace CSC8503;

PushdownState::PushdownResult MenuState::OnUpdate(float dt, PushdownState** newState) {
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM1))
	{
		*newState = new TestState(g);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM2))
	{
		*newState = new LevelOneState(g);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM3))
	{
		*newState = new LevelOneNoTimeModeState(g);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM4))
	{
		*newState = new LevelTwoState(g);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
	{
		return PushdownResult::Pop;
	}
	return PushdownResult::NoChange;
}

void MenuState::OnAwake()
{
	g->InitStartMenu();
}

PushdownState::PushdownResult LevelOneState::OnUpdate(float dt, PushdownState** newState)
{

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1))
	{
		g->InitLevelOne();
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F3))
	{
		return PushdownResult::Pop;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
	{
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}

void LevelOneState::OnAwake()
{
	g->InitLevelOne();
}

PushdownState::PushdownResult LevelOneNoTimeModeState::OnUpdate(float dt, PushdownState** newState)
{

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1))
	{
		g->InitLevelOneNoTimeMode();
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F3))
	{
		return PushdownResult::Pop;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
	{
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}

void LevelOneNoTimeModeState::OnAwake()
{
	g->InitLevelOneNoTimeMode();
}

PushdownState::PushdownResult LevelTwoState::OnUpdate(float dt, PushdownState** newState)
{

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1))
	{
		g->InitLevelTwo();
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F3))
	{
		return PushdownResult::Pop;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
	{
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}

void LevelTwoState::OnAwake()
{
	g->InitLevelTwo();
}


PushdownState::PushdownResult TestState::OnUpdate(float dt, PushdownState** newState)
{

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1))
	{
		g->InitWorld();
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F3))
	{
		return PushdownResult::Pop;
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE))
	{
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}

void TestState::OnAwake()
{
	g->InitWorld();
}