#include "PlayerGameObject.h"
#include "TutorialGame.h"

using namespace NCL;
using namespace CSC8503;

PlayerGameObject::PlayerGameObject(string name)
{
	time = 0;
	won = false;
	lost = false;
	isOpenDoor = false;
}

void PlayerGameObject::OnCollisionBegin(GameObject* otherObject)
{

	if (GetType() == playerType && otherObject->GetType() == levelTwoEndPosType)
	{
		won = true;
		lost = false;
	}

	if (GetType() == playerType && otherObject->GetType() == enemyType)
	{
		won = false;
		lost = true;
	}
	// todo
}

void PlayerGameObject::OnCollisionEnd(GameObject* otherObject)
{
	// todo
	if (otherObject->GetType() == levelTwoSpeedUpBlockType) {
		isSpeedUp = true;
		SetSpeedUpTime(5.0f);
	}
	if (GetType() == playerType && otherObject->GetType() == levelTwoDoorSwitchType) {
		isOpenDoor = !isOpenDoor;
	}
}

void PlayerGameObject::Update(float dt)
{
	if (isActive)
	{
		time += dt;
		if (time > 1.0 && won == false)
		{
			time = 0;
		}

		SetSpeedUpTime(leftSpeedUpTime - dt);
		if (leftSpeedUpTime < 0.0f) {
			isSpeedUp = false;
		}

		int speedFactor = isSpeedUp ? 50 : 30;

		Vector3 fwdAxis = GetTransform().GetOrientation() * Vector3(0, 0, 1);

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			//GetPhysicsObject()->AddForce(-fwdAxis * force);
			GetTransform().SetPosition(GetTransform().GetPosition() + (Vector3(0, 0, -1) * speedFactor * dt));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			GetTransform().SetPosition(GetTransform().GetPosition() + (Vector3(0, 0, 1) * speedFactor * dt));
		}

		//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
		//	GetPhysicsObject()->AddForce(Vector3(0, 3000.0f, 0));// *jumpForce);
		//}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT))
		{
			GetTransform().SetPosition(GetTransform().GetPosition() + (Vector3(1, 0, 0) * speedFactor * dt));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT))
		{
			GetTransform().SetPosition(GetTransform().GetPosition() + (Vector3(-1, 0, 0) * speedFactor * dt));
		}
	}

}