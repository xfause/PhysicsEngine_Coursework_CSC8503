#include "FloatConstraint.h"

void FloatConstraint::UpdateConstraint(float dt)
{
	yLevel.x = objectA->GetTransform().GetPosition().x;
	yLevel.z = objectA->GetTransform().GetPosition().z;

	Vector3 relativePos =
		objectA->GetTransform().GetPosition() - yLevel;

	float currentDistance = relativePos.Length();

	if (abs(currentDistance) > 0.0f)
	{
		Vector3 offsetDir = relativePos.Normalised();

		PhysicsObject* physA = objectA->GetPhysicsObject();
		Vector3 relativeVelocity = physA->GetLinearVelocity();

		float constraintMass = physA->GetInverseMass();
		if (constraintMass > 0.0f)
		{
			// how much of their relative force is affecting the constraint
			float velocityDot = Vector3::Dot(relativeVelocity, offsetDir);

			float biasFactor = 0.01f;
			float bias = -(biasFactor / dt) * -currentDistance;

			float lambda = -(velocityDot + bias) / constraintMass;

			Vector3 Impulse = offsetDir * lambda;

			physA->ApplyLinearImpulse(Impulse); // multiplied by mass here
			if (objectA->GetName() == "level_one_moving_platform") {
				physA->SetAngularVelocity(Vector3(0,0,0));
			}
			if (objectA->GetName() == "level_one_motor_plane") {
				Vector3 currAngularVelocity = physA->GetAngularVelocity();
				physA->SetAngularVelocity(Vector3(0, 0, currAngularVelocity.z));
				physA->SetLinearVelocity(Vector3(0, 0, 0));
			}
			if (objectA->GetName() == "level_two_motor_block") {
				Vector3 currAngularVelocity = physA->GetAngularVelocity();
				physA->SetAngularVelocity(Vector3(0, currAngularVelocity.y, 0));
				physA->SetLinearVelocity(Vector3(0, 0, 0));
			}
			if (objectA->GetName() == "level_one_ramp") {
				Vector3 currAngularVelocity = physA->GetAngularVelocity();
				if (currAngularVelocity.z > 0) {
					physA->SetAngularVelocity(Vector3(0, 0, 0));
				}
				else {
					physA->SetAngularVelocity(Vector3(0, 0, currAngularVelocity.z));
				}
				physA->SetLinearVelocity(Vector3(0, 0, 0));
			}
		}
	}
}