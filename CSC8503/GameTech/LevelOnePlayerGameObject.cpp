#include "PlayerGameObject.h"
#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

LevelOnePlayerGameObject::LevelOnePlayerGameObject(string name)
{
	time = 0;
	won = false;

	force = Vector3(10, 0, 10);
}

void LevelOnePlayerGameObject::OnCollisionBegin(GameObject* otherObject)
{

	if (GetType() == levelOnePlayerType && otherObject->GetType() == levelOneEndBlockType)
	{
		won = true;
	}
	if (GetType() == levelOnePlayerType && otherObject->GetType() == levelOneMovingPlatformType) {
		MovingPlatform* platform = (MovingPlatform*)otherObject;
		GetPhysicsObject()->SetLinearVelocity(platform->currentLinearVelocity);
	}
	if (GetType() == levelOnePlayerType && otherObject->GetType() == levelOneRampType) {
		GetPhysicsObject()->AddForceAtPosition(Vector3(0, -9.8, 0), GetTransform().GetPosition() + Vector3(-4, 0, 0));
	}
	if (GetType() == levelOnePlayerType && otherObject->GetType() == levelOneBonusType) {
		otherObject->SetBoundingVolume(nullptr);
		otherObject->SetIsActive(false);
		GetPhysicsObject()->AddForce(Vector3(-50, 0, 0));
		score += 10;
	}
}

void LevelOnePlayerGameObject::OnCollisionEnd(GameObject* otherObject)
{
	// todo
	if (GetType() == levelOnePlayerType && otherObject->GetType() == levelOneStartBlockType)
	{
		//GetPhysicsObject()->AddForce(Vector3(0, 3600, 0));
		GetPhysicsObject()->SetLinearVelocity(Vector3(0, 70, 0));
		GetPhysicsObject()->SetAngularVelocity(Vector3(10,10,10));
	}
	
}

void LevelOnePlayerGameObject::Update(float dt)
{
	if (isActive)
	{
		if (!won) {
			time += dt;
		}

		Vector3 finalForce = Vector3(10, 10, 10);

		Vector3 fwdAxis = GetTransform().GetOrientation() * Vector3(0, 0, 1);

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * finalForce);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			GetPhysicsObject()->AddForce(Vector3(0, -1, 0) * finalForce);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT))
		{
			GetPhysicsObject()->AddForce(Vector3(1, 0, 0) * finalForce);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT))
		{
			GetPhysicsObject()->AddForce(Vector3(-1, 0, 0) * finalForce);
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::K))
		{
			GetPhysicsObject()->AddForce(Vector3(0, 0, 1) * finalForce);
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::L))
		{
			GetPhysicsObject()->AddForce(Vector3(0, 0, -1) * finalForce);
		}
	}

}