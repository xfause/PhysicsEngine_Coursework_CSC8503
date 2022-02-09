#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "../CSC8503Common/NavigationGrid.h"

#include "StateGameObject.h"

#include "StateDoorGameObject.h"
#include "PlayerGameObject.h"
#include "LevelOnePlayerGameObject.h"
#include "BehaviourTreeEnemy.h"
#include "MovingPlatform.h"
#include "../CSC8503Common/MotorConstraint.h"
#include "MotorPlane.h"


namespace NCL {
	namespace CSC8503 {
		// object type
		enum OBJECT_TYPES { 
			bonus, 
			playerType,
			enemyType, 
			finish,
			wallType, 
			floorType, 
			levelTwoStartPosType, 
			levelTwoEndPosType,
			levelTwoDoorType,
			levelTwoDoorSwitchType,
			levelTwoSpeedUpBlockType,
			levelTwoMotorBlockType,

			levelOnePlayerType,
			levelOneBackgroundType,
			levelOneFrontGlassType,
			levelOneEdgeType,
			levelEndPosType,
			levelOneStartBlockType,
			levelOneEndBlockType,
			levelOneMovingPlatformType,
			levelOneMotorPlaneType,
			levelOneRampType,
			levelOneRampPillarType,
			levelOneChainEndType,
			levelOneBonusType
		};

		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			void InitStartMenu();
			void InitWorld();
			void InitLevelOne();
			void InitLevelOneNoTimeMode();
			void InitLevelTwo();

			// level 1 stuff
			void CheckLevelOneUserInput();
			LevelOnePlayerGameObject* GetLevelOnePlayer() { return levelOnePlayer; }
			GameObject* AddLevelOneBackGround();
			GameObject* AddLevelOneFrontGlass();
			LevelOnePlayerGameObject* AddLevelOnePlayer();
			void AddLevelOneEdge();
			GameObject* AddLevelOneStartBlock();
			GameObject* AddLevelOneMovingPlatform();
			GameObject* AddLevelOneMotorPlane();
			void AddLevelOneRamp();
			GameObject* AddLevelOneEndBlock();
			void AddLevelOneChain();
			void AddLevelOneBonus();
			bool isMotorPlaneRotate = false;
			MotorPlane* levelOneMotorPlane;
			MotorConstraint* motorPlaneConstraint;
			GameObject* levelOneRampPhillar;
			GameObject* levelOneMovableRamp;
			GameObject* debugCheckObject;
			bool levelOneUseTime = false;


			// level 2 stuff
			PlayerGameObject* GetLevelTwoPlayer() { return player; }
			BehaviourTreeEnemy* GetLevelTwoEmeny() { return enemy; }
			void GenerateLevelTwoMaze(const std::string& filename);
			BehaviourTreeEnemy* AddBehaviourEnemyToWorld(const Vector3& position);
			PlayerGameObject* AddLevelTwoPlayerToWorld(const Vector3& position);
			StateDoorGameObject* AddStateDoorToWorld(const Vector3& start, const Vector3& end);
			GameObject* AddLevelTwoWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 0.0f);
			void AddLevelTwoStartAndEndBlock();
			void AddLevelTwoSpeedUpBlockToWorld();
			void AddLevelTwoSwitchDoorBlock();
			void AddLevelTwoMotorBlock();
			bool showNavMesh = false;

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement(float dt);

			GameObject* AddFloorToWorld(const Vector3& size, const Vector3& position, const Quaternion& rotation=Quaternion::Quaternion(), const Vector3& offset = Vector3(0,0,0));
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, float elasticity = 1.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, float elasticity = 1.0f);

			GameObject* AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float elasticity, float friction, Vector4 colour, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f, float elasticity = 1.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}


			bool gameOver = false;
			bool win = false;
			float time = 0;

			bool tutorial = false;
			bool LevelOne = false;
			bool menu = false;

			Vector3 courseOffset = Vector3(0, 0, 0);

			PlayerGameObject* player = nullptr;
			LevelOnePlayerGameObject* levelOnePlayer = nullptr;
			BehaviourTreeEnemy* enemy = nullptr;

		};
	}
}

