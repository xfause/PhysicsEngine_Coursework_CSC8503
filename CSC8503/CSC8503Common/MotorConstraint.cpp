#include "MotorConstraint.h"
#include "PhysicsSystem.h"

using namespace NCL;
using namespace CSC8503;

void MotorConstraint::UpdateConstraint(float dt)
{
	PhysicsObject* physA = objectA->GetPhysicsObject();
	Vector3 AngVel = physA->GetAngularVelocity();

	Vector3 motorForce = objectA->GetTransform().GetOrientation() * Vector3(SpeedX, SpeedY, SpeedZ);


	if (AngVel != motorForce)
	{
		Vector3 difference = motorForce - AngVel;

		Vector3 impulse = difference / physA->GetInverseMass();
		physA->ApplyAngularImpulse(impulse);
	}
}