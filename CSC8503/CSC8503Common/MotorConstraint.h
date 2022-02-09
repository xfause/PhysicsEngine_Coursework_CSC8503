#pragma once
#include "Constraint.h"

namespace NCL
{
	namespace CSC8503
	{
		class GameObject;

		class MotorConstraint : public Constraint
		{
		public:
			MotorConstraint(GameObject* a, float sx, float sy, float sz)
			{
				objectA = a;
				SpeedX = sx;
				SpeedY = sy;
				SpeedZ = sz;
			}
			~MotorConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;

			float SpeedX;
			float SpeedY;
			float SpeedZ;
		};
	}
}