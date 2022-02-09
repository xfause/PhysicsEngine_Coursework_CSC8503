#include "MovingPlatform.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

using namespace NCL;
using namespace CSC8503;

MovingPlatform::MovingPlatform() {
	counter = 0.0f;
	currentLinearVelocity = Vector3(0, 0, 0);
	stateMachine = new StateMachine();

	State* stateA = new State([&](float dt)->void
		{
			this->MoveLeft(dt);
		});
	State* stateB = new State([&](float dt)->void
		{
			this->MoveRight(dt);
		});

	stateMachine->AddState(stateA);
	stateMachine->AddState(stateB);

	stateMachine->AddTransition(new StateTransition(stateA, stateB,
		[&]()->bool
		{
			return this->counter > 3.0f;
		}));
	stateMachine->AddTransition(new StateTransition(stateB, stateA,
		[&]()->bool
		{
			return this->counter < -3.0f;
		}));
}

MovingPlatform::~MovingPlatform()
{
	delete stateMachine;
}

void MovingPlatform::Update(float dt) {
	stateMachine->Update(dt);
}

void MovingPlatform::MoveLeft(float dt) {
	GetPhysicsObject()->AddForce({ -2000, 0,0 });
	counter += dt;
	currentLinearVelocity = GetPhysicsObject()->GetLinearVelocity();
}

void MovingPlatform::MoveRight(float dt) {
	GetPhysicsObject()->AddForce({ 2000, 0,0 });
	counter -= dt;
	currentLinearVelocity = GetPhysicsObject()->GetLinearVelocity();
}