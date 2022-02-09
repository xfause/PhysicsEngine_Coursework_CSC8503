#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/GameWorld.h"

namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class StateDoorGameObject : public GameObject
		{
		public:
			StateDoorGameObject(const Vector3& start, const Vector3& end, GameWorld* a);
			~StateDoorGameObject();

			virtual void Update(float dt);

			CollisionVolume* preVolume;

		protected:
			void Open(float dt);
			void Close(float dt);

			StateMachine* stateMachine;
			GameWorld* worldRef;
		};
	}
}