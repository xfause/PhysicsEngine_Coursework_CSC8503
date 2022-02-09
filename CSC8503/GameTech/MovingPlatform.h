#pragma once
#include "..\CSC8503Common\GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class MovingPlatform : public GameObject
		{
		public:
			MovingPlatform();
			~MovingPlatform();

			virtual void Update(float dt);

			Vector3 currentLinearVelocity;

		protected:
			void MoveLeft(float dt);
			void MoveRight(float dt);

			StateMachine* stateMachine;
			float counter;
		};
	}
}