#pragma once
#include "..\CSC8503Common\GameObject.h"

namespace NCL
{
	namespace CSC8503
	{
		class LevelOnePlayerGameObject : public GameObject
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
			int score = 0;

			float timeLeft = 90;

			LevelOnePlayerGameObject(string name = "");
		};

	}
}