#pragma once
#include "../CSC8503Common/GameObject.h"
#include "../CSC8503Common/GameWorld.h"
#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/BehaviourSequence.h"
#include "../CSC8503Common/BehaviourSelector.h"
#include "../CSC8503Common/BehaviourNodeWithChildren.h"
#include "../CSC8503Common/BehaviourNode.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "../CSC8503Common/NavigationMesh.h"

#include "PlayerGameObject.h"


namespace NCL
{
	namespace CSC8503
	{

		class BehaviourTreeEnemy : public PlayerGameObject
		{
		public:
			BehaviourTreeEnemy(GameWorld* a, const Vector3& initPosition);
			~BehaviourTreeEnemy();

			void Update(float dt) override;

			Vector3 prePos;

			// grid base
			NavigationGrid levelTwoGrid;
			vector<Vector3> levelTwoPathNodes = {};
			float gridBaseL = 0;

			 NavigationMesh* navMesh;
			// Mesh string path
			vector<Vector3> meshStringNodes = {};
			float meshStringL = 0;
			// Mesh smooth path
			vector<Vector3> meshSmoothNodes = {};
			float meshSmoothL = 0;

			vector<Vector3> shortestNodes = {};
			float shortestL = 0;

			int speedFactor = 0;
			int tempIndex = 0;

		protected:

			BehaviourSequence* rootSequence;
			BehaviourSequence* sequence;
			BehaviourSelector* selection;

			GameWorld* worldRef;

			BehaviourState state = Ongoing;

		};
	}
}