#pragma once
#include "Constraint.h"
#include "PhysicsSystem.h"

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		class FloatConstraint : public Constraint
		{
		public:
			FloatConstraint(GameObject* a, float y)
			{
				objectA = a;
				yLevel = Vector3(0, y, 0);
			}
			~FloatConstraint() {};

			void UpdateConstraint(float dt) override;

		protected:
			GameObject* objectA;
			Vector3 yLevel;
		};

	}
}