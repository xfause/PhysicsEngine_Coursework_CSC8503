#include "AngularConstraint.h"
#include "PhysicsSystem.h"

#define _USE_MATH_DEFINES
#include <math.h>

using namespace NCL;
using namespace CSC8503;

void AngularConstraint::UpdateConstraint(float dt) {

	Vector3 currentOrientationA = axis ? objectA->GetTransform().GetUp() : objectA->GetTransform().GetForward();
	Vector3 currentOrientationB = axis ? objectB->GetTransform().GetUp() : objectB->GetTransform().GetForward();

	float dot = Vector3::Dot(currentOrientationA, currentOrientationB);	//no need to divide by the lengths of the vectors due to both vectors having a length of 1
	float currentAngle = acos(dot) * 180 / M_PI;
	float offset = angle - currentAngle;

	PhysicsObject* physA = objectA->GetPhysicsObject();
	PhysicsObject* physB = objectB->GetPhysicsObject();

	float constrainMass = physA->GetInverseMass() + physB->GetInverseMass();

	if (abs(offset) > 0.0f)
	{
		Vector3 axis = Vector3::Cross(currentOrientationA, currentOrientationB);
		float biasFactor = 0.01f;
		float bias = -(biasFactor / dt) * angle;
		float lambda = -(dot + bias) / constrainMass;

		physA->ApplyAngularImpulse(axis * lambda);
		physB->ApplyAngularImpulse(-axis * lambda);
	}
}