#include "StateDoorGameObject.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

using namespace NCL;
using namespace CSC8503;

StateDoorGameObject::StateDoorGameObject(const Vector3& start, const Vector3& end, GameWorld* a) {
	stateMachine = new StateMachine();
	worldRef = a;

	State* stateA = new State([&](float dt)->void
		{
			this->Open(dt);
		});
	State* stateB = new State([&](float dt)->void
		{
			this->Close(dt);
		});

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB,
		[&]()->bool
		{
			return worldRef->GetPlayer()->isOpenDoor == false ? true : false;
		}));
	stateMachine->AddTransition(new StateTransition(stateB, stateA,
		[&]()->bool
		{
			return worldRef->GetPlayer()->isOpenDoor == true ? true : false;
		}));
}

StateDoorGameObject::~StateDoorGameObject()
{
	delete stateMachine;
}

void StateDoorGameObject::Update(float dt) {
	stateMachine->Update(dt);
}

void StateDoorGameObject::Open(float dt) {
	isActive = false;
	SetBoundingVolume((CollisionVolume*)new OBBVolume(Vector3(0, 0, 0)));
}

void StateDoorGameObject::Close(float dt) {
	isActive = true;
	SetBoundingVolume((CollisionVolume*)preVolume);
}