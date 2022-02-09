#pragma once
#include "..\CSC8503Common\GameObject.h"

namespace NCL
{
	namespace CSC8503
	{
		class PlayerGameObject : public GameObject
		{
		protected:
			void OnCollisionBegin(GameObject* otherObject) override;
			void OnCollisionEnd(GameObject* otherObject) override;

			virtual void Update(float dt);

		public:
			float time = 0;
			bool won;
			bool lost;
			Vector3 force;

			bool isOpenDoor;

			bool isSpeedUp;
			float leftSpeedUpTime;
			void SetSpeedUpTime(float t) {
				leftSpeedUpTime = t;
			}

			PlayerGameObject(string name = "");
		};

	}
}