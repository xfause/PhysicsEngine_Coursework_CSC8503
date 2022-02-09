#pragma once
#include "Constraint.h"

namespace NCL
{
	namespace CSC8503
	{
		class GameObject;

		class AngularConstraint : public Constraint
		{
		public:
			AngularConstraint(GameObject* a, GameObject* b, float ang, bool ax)
			{
				objectA = a;
				objectB = b;
				angle = ang;
				axis = ax;
			}
			~AngularConstraint() {}

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			GameObject* objectB;
			float angle;
			bool axis;
		};
	}
}