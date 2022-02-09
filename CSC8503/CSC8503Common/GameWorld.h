#pragma once
#include <vector>
#include "Ray.h"
#include "CollisionDetection.h"
#include "QuadTree.h"

#include "../GameTech/PlayerGameObject.h"
#include "../GameTech/LevelOnePlayerGameObject.h"

namespace NCL {
		class Camera;
		using Maths::Ray;
	namespace CSC8503 {
		class GameObject;
		class Constraint;

		typedef std::function<void(GameObject*)> GameObjectFunc;
		typedef std::vector<GameObject*>::const_iterator GameObjectIterator;

		class GameWorld	{
		public:
			GameWorld();
			~GameWorld();

			void Clear();
			void ClearAndErase();

			void AddGameObject(GameObject* o);
			void RemoveGameObject(GameObject* o, bool andDelete = false);

			void AddConstraint(Constraint* c);
			void RemoveConstraint(Constraint* c, bool andDelete = false);

			Camera* GetMainCamera() const {
				return mainCamera;
			}

			void ShuffleConstraints(bool state) {
				shuffleConstraints = state;
			}

			void ShuffleObjects(bool state) {
				shuffleObjects = state;
			}

			bool Raycast(Ray& r, RayCollision& closestCollision, bool closestObject = false) const;

			virtual void UpdateWorld(float dt);

			void OperateOnContents(GameObjectFunc f);

			void GetObjectIterators(
				GameObjectIterator& first,
				GameObjectIterator& last) const;

			void GetConstraintIterators(
				std::vector<Constraint*>::const_iterator& first,
				std::vector<Constraint*>::const_iterator& last) const;

			GameObject* GetNearestSpeedUpBlock();
			GameObject* GetLevelTwoBonus();

			void SetPlayer(PlayerGameObject* p) {
				player = p;
			}
			PlayerGameObject* GetPlayer() {
				return player;
			}

			void SetLevelOnePlayer(LevelOnePlayerGameObject* p) {
				levelOnePlayer = p;
			}
			LevelOnePlayerGameObject* GetLevelOnePlayer() {
				return levelOnePlayer;
			}

		protected:
			PlayerGameObject* player;
			LevelOnePlayerGameObject* levelOnePlayer;

			std::vector<GameObject*> gameObjects;
			std::vector<Constraint*> constraints;

			Camera* mainCamera;

			bool	shuffleConstraints;
			bool	shuffleObjects;
			int		worldIDCounter;
		};
	}
}

