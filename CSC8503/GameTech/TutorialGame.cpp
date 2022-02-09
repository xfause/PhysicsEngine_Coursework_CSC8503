#include "TutorialGame.h"

#include <fstream>
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../../Common/Assets.h"

#include "../CSC8503Common/PositionConstraint.h"
#include "../CSC8503Common/AngularConstraint.h"
#include "../CSC8503Common/MotorConstraint.h"
#include "../CSC8503Common/FloatConstraint.h"

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame() {
	world = new GameWorld();
	renderer = new GameTechRenderer(*world);
	physics = new PhysicsSystem(*world);

	forceMagnitude = 10.0f;
	useGravity = false;
	inSelectionMode = false;
	testStateObject = nullptr;

	tutorial = true;
	LevelOne = false;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes,
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh", &cubeMesh);
	loadFunc("sphere.msh", &sphereMesh);
	loadFunc("Male1.msh", &charMeshA);
	loadFunc("courier.msh", &charMeshB);
	loadFunc("security.msh", &enemyMesh);
	loadFunc("coin.msh", &bonusMesh);
	loadFunc("capsule.msh", &capsuleMesh);

	basicTex = (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	
	InitCamera();
	//InitWorld();
}

TutorialGame::~TutorialGame() {
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {

	if (!menu) {
		if (!inSelectionMode) {
			world->GetMainCamera()->UpdateCamera(dt);
		}

		UpdateKeys();

		if (lockedObject && tutorial)
		{
			if (lockedObject->IsActive())
				LockedObjectMovement(dt);
		}
		else {
			DebugObjectMovement();
		}

		if (tutorial) {
			if (useGravity) {
				Debug::Print("(G)ravity on", Vector2(5, 95));
			}
			else {
				Debug::Print("(G)ravity off", Vector2(5, 95));
			}

			SelectObject();
			MoveSelectedObject();
		}

		physics->Update(dt);

		if (lockedObject != nullptr) {
			Vector3 objPos = lockedObject->GetTransform().GetPosition();
			Vector3 objOrient = lockedObject->GetTransform().GetOrientation() * lockedOffset;
			Vector3 camPos = objPos + objOrient;

			Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = temp.Inverse();

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera()->SetPosition(camPos);
			world->GetMainCamera()->SetPitch(angles.x);
			world->GetMainCamera()->SetYaw(angles.y);

			//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
		}

		// in level 2 
		if (!LevelOne && !tutorial) {
			renderer->DrawString("F1: RESTART", Vector2(75, 80), Vector4(1, 1, 1, 1), 20.0f);
			renderer->DrawString("F3: MAIN MENU", Vector2(75, 90), Vector4(1, 1, 1, 1), 20.0f);

			if (GetLevelTwoPlayer()) {
				PlayerGameObject* player = GetLevelTwoPlayer();

				// draw debug path length
			 	BehaviourTreeEnemy* enemy = GetLevelTwoEmeny();
				renderer->DrawString("Grid base length:" + std::to_string(enemy->gridBaseL), Vector2(10, 10), Vector4(1, 0, 0, 1), 15.0f);
				renderer->DrawString("Mesh String length:" + std::to_string(enemy->meshStringL), Vector2(10, 13), Vector4(1, 0.5, 0, 1), 15.0f);
				renderer->DrawString("Mesh Smooth length:" + std::to_string(enemy->meshSmoothL), Vector2(10, 16), Vector4(0, 1, 0, 1), 15.0f);
				renderer->DrawString("Speed:" + std::to_string(enemy->speedFactor), Vector2(10, 19), Vector4(1, 1, 1, 1), 15.0f);


				if (showNavMesh == true) {
					NavigationMesh* navMesh = new NavigationMesh("maze2.navmesh");
					navMesh->DrawMesh();
				}

				if (player->lost)
				{
					//gameOver = true;
					//renderer->DrawString("GAME OVER!", Vector2(30, 50), Vector4(1.0f, 0.0f, 0.0f, 0.0f), 50.0f);
					//renderer->DrawString("Press F1 to retry or F3 to return to menu!", Vector2(15, 65), Vector4(1.0f, 0.0f, 0.0f, 0.0f), 20.0f);
					InitLevelTwo();
				}
				if (player->won)
				{
					renderer->DrawString("YOU WON!", Vector2(35, 50), Vector4(0.0f, 1.0f, 0.0f, 0.0f), 50.0f);
					//renderer->DrawString("Press F1 to retry or F3 to return to menu!", Vector2(15, 65), Vector4(1.0f, 0.0f, 0.0f, 0.0f), 20.0f);
				}
			}
		}
		// in level 1
		if (LevelOne && !tutorial) {
			if (GetLevelOnePlayer()) {
				LevelOnePlayerGameObject* player = GetLevelOnePlayer();
				
				if (levelOneUseTime) {
					if (!player->lost && !player->won) {
						player->timeLeft -= dt;
						renderer->DrawString("LEFT TIME:" + std::to_string((int)floor(player->timeLeft)), Vector2(75, 25), Vector4(1.0f, 1.0f, 1.0f, 1.0f), 20.0f);
					}
					if (player->timeLeft <= 0.0) {
						player->lost = true;
						player->won = false;
					}
				}

				renderer->DrawString("SCORE:" + std::to_string(player->score) , Vector2(80, 20), Vector4(1.0f, 1.0f, 1.0f, 1.0f), 20.0f);

				renderer->DrawString("F1: RESTART", Vector2(75, 80), Vector4(1, 1, 1, 1), 20.0f);
				renderer->DrawString("F3: MAIN MENU", Vector2(75, 90), Vector4(1, 1, 1, 1), 20.0f);

				if (player->won)
				{
					renderer->DrawString("YOU WON!", Vector2(35, 50), Vector4(0.0f, 1.0f, 0.0f, 0.0f), 50.0f);
					string t = "USE " + std::to_string((int)floor(player->time)) + " s";
					renderer->DrawString(t, Vector2(35, 70), Vector4(0.0f, 1.0f, 0.0f, 0.0f), 50.0f);
					//renderer->DrawString("Press F1 to retry or F3 to return to menu!", Vector2(15, 85), Vector4(1.0f, 0.0f, 0.0f, 0.0f), 20.0f);
				}
				else if (player->lost) {
					renderer->DrawString("YOU LOSE!", Vector2(35, 50), Vector4(1.0f, 0.0f, 0.0f, 0.0f), 50.0f);
				}
				else {
					// check mouse click
					CheckLevelOneUserInput();
				}
			}
		}

		if (testStateObject)
		{
			testStateObject->Update(dt);
		}

		if (debugCheckObject != nullptr) {
			Vector3 objPosition = debugCheckObject->GetTransform().GetPosition();
			renderer->DrawString(debugCheckObject->GetName(), Vector2(10, 10), Vector4(1, 1, 1, 1), 15.0f);

			string posStr = "pos: " + std::to_string(objPosition.x) + ',' + std::to_string(objPosition.y) + ',' + std::to_string(objPosition.z);
			renderer->DrawString(posStr, Vector2(10, 13), Vector4(1, 1, 1, 1), 15.0f);

			Vector3 objForce = debugCheckObject->GetPhysicsObject()->GetForce();
			string forceStr = "force: " + std::to_string(objForce.x) + ',' + std::to_string(objForce.y) + ',' + std::to_string(objForce.z);
			renderer->DrawString(forceStr, Vector2(10, 16), Vector4(1, 1, 1, 1), 15.0f);

			Vector3 objLinearVelocity = debugCheckObject->GetPhysicsObject()->GetLinearVelocity();
			string lvStr = "linear: " + std::to_string(objLinearVelocity.x) + ',' + std::to_string(objLinearVelocity.y) + ',' + std::to_string(objLinearVelocity.z);
			renderer->DrawString(lvStr, Vector2(10, 19), Vector4(1, 1, 1, 1), 15.0f);

			Vector3 objAngularVelocity = debugCheckObject->GetPhysicsObject()->GetAngularVelocity();
			string avStr = "angular: " + std::to_string(objAngularVelocity.x) + ',' + std::to_string(objAngularVelocity.y) + ',' + std::to_string(objAngularVelocity.z);
			renderer->DrawString(avStr, Vector2(10, 22), Vector4(1, 1, 1, 1), 15.0f);
		}
	}
	else {
		world->GetMainCamera()->SetPosition(Vector3(0, 0, 0));
		renderer->DrawString("Main Menu", Vector2(30, 10), Vector4(1.0f, 0.5f, 0.5f, 0.0f), 50.0f);
		renderer->DrawString("1. Testing ", Vector2(10, 25), Vector4(1.0f, 1.0f, 0.0f, 0.0f), 40.0f);
		renderer->DrawString("2. Level 1", Vector2(10, 35), Vector4(1.0f, 1.0f, 0.0f, 0.0f), 40.0f);
		renderer->DrawString("3. Level 1- no time", Vector2(10, 45), Vector4(1.0f, 1.0f, 0.0f, 0.0f), 40.0f);
		renderer->DrawString("4. Level 2", Vector2(10, 55), Vector4(1.0f, 1.0f, 0.0f, 0.0f), 40.0f);
		renderer->DrawString("ESC. Exit", Vector2(10, 65), Vector4(1.0f, 1.0f, 0.0f, 0.0f), 40.0f);
		physics->Update(dt);
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
	//	InitWorld(); //We can reset the simulation at any time with F1
	//	selectionObject = nullptr;
	//	lockedObject	= nullptr;
	//}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}

	// show level 2 nav mesh
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::N)) {
		showNavMesh = !showNavMesh;
	}

	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}
}

void TutorialGame::LockedObjectMovement(float dt) {
	Matrix4 view = world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld = view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	}
}

void TutorialGame::DebugObjectMovement() {
	//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitStartMenu()
{
	world->ClearAndErase();
	physics->Clear();

	selectionObject = nullptr;

	useGravity = false;
	physics->UseGravity(useGravity);

	menu = true;
	tutorial = false;
	gameOver = false;

	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	gameOver = false;
	tutorial = true;
	menu = false;

	lockedObject = nullptr;

	Window::GetWindow()->ShowOSPointer(false);
	Window::GetWindow()->LockMouseToWindow(true);

	world->ClearAndErase();
	world->GetMainCamera()->SetIsActive(true);

	physics->Clear();

	// test generate object
	/*InitMixedGridWorld(5, 5, 3.5f, 3.5f);
	InitGameExamples();*/

	// test collision response
	/*Vector3 cubeDims = Vector3(1, 1, 1);
	Vector3 position = Vector3(5,5,5);
	AddSphereToWorld(position, 2, 10.0, 0.8);*/

	// test position constraint
	//BridgeConstraintTest();

	// test motor constraint
	//GameObject* cube = AddOBBCubeToWorld(Vector3(10, 10, 0), Vector3(1, 1, 1), 0.5f, 0.5f, Vector4(1, 1, 1, 1), 0.3f);
	//MotorConstraint* constraint = new MotorConstraint(cube, 0, 1, 0);
	//world->AddConstraint(constraint);

	// test moving platform
	/*MovingPlatform* platform = AddMovingPlatform(Vector3(100, 0, 0 + 140));
	FloatConstraint* constraint = new FloatConstraint(platform, 0);
	world->AddConstraint(constraint);*/

	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));

	// test mode
	// need to show
	// 
	// AABB floor
	{
		GameObject* AabbFloor = new GameObject("aabb Floor");

		Vector3 floorSize = Vector3(50, 2, 50);
		AABBVolume* volume = new AABBVolume(floorSize * Vector3(1, 0.5, 1));
		AabbFloor->SetBoundingVolume((CollisionVolume*)volume);
		AabbFloor->GetTransform()
			.SetScale(floorSize)
			.SetPosition(Vector3(0, -2, 0));

		AabbFloor->SetRenderObject(new RenderObject(&AabbFloor->GetTransform(), cubeMesh, basicTex, basicShader));
		AabbFloor->SetPhysicsObject(new PhysicsObject(&AabbFloor->GetTransform(), AabbFloor->GetBoundingVolume()));

		AabbFloor->GetPhysicsObject()->SetInverseMass(0);
		AabbFloor->GetPhysicsObject()->InitCubeInertia();

		AabbFloor->GetRenderObject()->SetColour(Vector4(1, 0.7, 0.4, 1));

		AabbFloor->SetType(floorType);

		world->AddGameObject(AabbFloor);
	}

	GameObject* normalAABBCube = AddCubeToWorld(Vector3(0, 20, 0), Vector3(2, 2, 2));
	normalAABBCube->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	GameObject* springAABBCube = AddCubeToWorld(Vector3(10, 20, 10), Vector3(2, 2, 2));
	springAABBCube->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));
	springAABBCube->GetPhysicsObject()->SetUseSpring(true);
	springAABBCube->GetPhysicsObject()->SetLinearDamping(1.0f);

	GameObject* cap1 = AddCapsuleToWorld(Vector3(-10, 10, -10), 2, 1);
	GameObject* cap2 = AddCapsuleToWorld(Vector3(-5, 10, -5), 2, 1);

	// obb floor
	{
		GameObject* ObbFloor = new GameObject("obb Floor");

		Vector3 floorSize = Vector3(50, 2, 50);
		OBBVolume* volume = new OBBVolume(floorSize * Vector3(1, 0.5, 1));
		ObbFloor->SetBoundingVolume((CollisionVolume*)volume);
		ObbFloor->GetTransform()
			.SetScale(floorSize)
			.SetPosition(Vector3(50, -2, 50));

		ObbFloor->SetRenderObject(new RenderObject(&ObbFloor->GetTransform(), cubeMesh, basicTex, basicShader));
		ObbFloor->SetPhysicsObject(new PhysicsObject(&ObbFloor->GetTransform(), ObbFloor->GetBoundingVolume()));

		ObbFloor->GetPhysicsObject()->SetInverseMass(0);
		ObbFloor->GetPhysicsObject()->InitCubeInertia();

		ObbFloor->SetType(floorType);

		world->AddGameObject(ObbFloor);
	}

	GameObject* obbCube = AddOBBCubeToWorld(Vector3(50, 20, 50), Vector3(2, 2, 2),0.4,0.5, Vector4(0, 0, 1, 1),10.0f);
}
// level 1 no time mode stuff
void TutorialGame::InitLevelOneNoTimeMode() {
	gameOver = false;
	tutorial = false;
	LevelOne = true;
	menu = false;

	lockedObject = nullptr;

	world->ClearAndErase();
	physics->Clear();

	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(false);

	useGravity = true;
	physics->UseGravity(useGravity);
	physics->UseBroadPhase(true);

	// init camera
	world->GetMainCamera()->SetIsActive(true);
	world->GetMainCamera()->SetPosition(Vector3(100, -50, 150));
	world->GetMainCamera()->SetPitch(0);
	world->GetMainCamera()->SetYaw(0);

	levelOneUseTime = false;

	AddLevelOneBackGround();
	AddLevelOneEdge();
	AddLevelOnePlayer();
	AddLevelOneStartBlock();
	AddLevelOneMovingPlatform();
	AddLevelOneMotorPlane();
	AddLevelOneRamp();
	AddLevelOneEndBlock();
	AddLevelOneChain();
	AddLevelOneBonus();
}

// level 1 stuff
void TutorialGame::InitLevelOne() {
	// todo
	gameOver = false;
	tutorial = false;
	LevelOne = true;
	menu = false;

	lockedObject = nullptr;

	world->ClearAndErase();
	physics->Clear();

	Window::GetWindow()->ShowOSPointer(true);
	Window::GetWindow()->LockMouseToWindow(false);

	useGravity = true;
	physics->UseGravity(useGravity);
	physics->UseBroadPhase(true);

	// init camera
	world->GetMainCamera()->SetIsActive(true);
	world->GetMainCamera()->SetPosition(Vector3(100, -50, 150));
	world->GetMainCamera()->SetPitch(0);
	world->GetMainCamera()->SetYaw(0);

	levelOneUseTime = true;

	AddLevelOneBackGround();
	AddLevelOneEdge();
	AddLevelOnePlayer();
	AddLevelOneStartBlock();
	AddLevelOneMovingPlatform();
	AddLevelOneMotorPlane();
	AddLevelOneRamp();
	AddLevelOneEndBlock();
	AddLevelOneChain();
	AddLevelOneBonus();
}

// level 1 stuff
void TutorialGame::CheckLevelOneUserInput() {
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::LEFT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node != nullptr) {
				GameObject* clickObject = (GameObject*)closestCollision.node;
				int clickObjectType = clickObject->GetType();

				// click the start block
				if (clickObjectType == levelOneStartBlockType) {
					std::cout << "click start block" << std::endl;
					CollisionDetection::CollisionInfo info;
					LevelOnePlayerGameObject* levelOnePlayer = GetLevelOnePlayer();
					if (CollisionDetection::ObjectIntersection(clickObject, levelOnePlayer, info)) {
						std::cout << "click start block and they are collision" << std::endl;
						levelOnePlayer->GetPhysicsObject()->AddForce(
							Vector3(0, 3600, 0)
						);
					}
				}
				if (clickObjectType == levelOneMotorPlaneType) {
					if (!isMotorPlaneRotate) {
						motorPlaneConstraint = new MotorConstraint(levelOneMotorPlane, 0, 0, 5);
						world->AddConstraint(motorPlaneConstraint);
					}
					else {
						world->RemoveConstraint(motorPlaneConstraint, true);
						levelOneMotorPlane->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
					}
					isMotorPlaneRotate = !isMotorPlaneRotate;
				}
				if (clickObjectType == levelOneRampPillarType) {
					levelOneRampPhillar->SetIsActive(false);
					levelOneRampPhillar->SetBoundingVolume((CollisionVolume*)new OBBVolume(Vector3(0, 0, 0)));
					levelOneMovableRamp->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, -0.07));
					//levelOneMovableRamp->GetPhysicsObject()->AddForceAtPosition(Vector3(0, -60000, 0), Vector3(170, -75, 7));
				}
				if (clickObjectType == levelOneChainEndType) {
					clickObject->GetPhysicsObject()->SetInverseMass(0.01f);
				}
			}
		}
	}
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node != nullptr) {
				debugCheckObject = (GameObject*)closestCollision.node;
			}
		}
	}
}

GameObject* TutorialGame::AddLevelOneBackGround() {
	GameObject* levelOneBackground = new GameObject("level_one_background");

	Vector3 size = Vector3(200, 2, 100);
	Vector3 position = Vector3(0, 0, 0);
	Vector3 offset = Vector3(100, -50, 0);
	Quaternion rotation = Quaternion::Quaternion(Matrix4::Rotation(90, Vector3(1, 0, 0)));
	AABBVolume* volume = new AABBVolume(Vector3(200, 100, 2));
	levelOneBackground->SetBoundingVolume((CollisionVolume*)volume);
	levelOneBackground->GetTransform()
		.SetScale(size)
		.SetOrientation(rotation)
		.SetPosition(position + offset);

	levelOneBackground->SetRenderObject(new RenderObject(&levelOneBackground->GetTransform(), cubeMesh, basicTex, basicShader));
	levelOneBackground->SetPhysicsObject(new PhysicsObject(&levelOneBackground->GetTransform(), levelOneBackground->GetBoundingVolume()));

	levelOneBackground->GetPhysicsObject()->SetInverseMass(0);
	levelOneBackground->GetPhysicsObject()->InitCubeInertia();
	levelOneBackground->GetPhysicsObject()->SetElasticity(0.0f);

	levelOneBackground->SetType(levelOneBackgroundType);

	world->AddGameObject(levelOneBackground);

	return levelOneBackground;
}

void TutorialGame::AddLevelOneEdge() {
	Vector3 start_position[5] = {
		Vector3(0,10,0),Vector3(5, 10, 0), Vector3(195, 10, 0), Vector3(5, 10, 195), Vector3(55, 2, 25)
	};
	Vector3 end_position[5] = {
		Vector3(5, 0, 100), Vector3(200, 0, 5), Vector3(200, 0, 95),Vector3(195, 0, 200), Vector3(60,12,35)
	};
	Quaternion rotation = Quaternion::Quaternion(Matrix4::Rotation(90, Vector3(1, 0, 0)));

	for (int i = 0; i < 5; i++) {
		Vector3 start = start_position[i];
		Vector3 end = end_position[i];
		GameObject* wall = new GameObject("level_one_edge");
		OBBVolume* volume = new OBBVolume(
			Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
			* Vector3(0.5, 0.5, 0.5)
		);
		Vector3 finalPosition = Vector3(0.5 * std::abs(end.x - start.x), -0.5 * std::abs(end.z - start.z), std::abs(end.y - start.y)) + Vector3(start.x, 0, start.z);
		float invMass = 0.0f;
		if (i == 2) {
			finalPosition += Vector3(0, -5, 0);
		}
		if (i == 3) {
			finalPosition += Vector3(0, -95, -195);
		}
		if (i == 4) {
			finalPosition += Vector3(0, -40, -25);
		}
		finalPosition += Vector3(0, 0, -4);

		wall->SetBoundingVolume((CollisionVolume*)volume);
		wall->GetTransform()
			.SetPosition(finalPosition)
			//.SetPosition(Vector3(0.5 * std::abs(end.x - start.x), 0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)))
			.SetOrientation(rotation)
			.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
		wall->SetRenderObject(new RenderObject(&wall->GetTransform(), cubeMesh, basicTex, basicShader));
		wall->SetPhysicsObject(new PhysicsObject(&wall->GetTransform(), wall->GetBoundingVolume()));

		wall->GetPhysicsObject()->SetInverseMass(invMass);
		wall->GetPhysicsObject()->InitCubeInertia();

		wall->GetPhysicsObject()->SetElasticity(1.0f);
		wall->SetType(levelOneEdgeType);

		world->AddGameObject(wall);
	}
}

LevelOnePlayerGameObject* TutorialGame::AddLevelOnePlayer() {
	int radius = 4.0f;
	float inverseMass = 2.0f;
	Vector3 position = Vector3(10, -80, 5);

	Vector3 sphereSize = Vector3(radius, radius, radius);

	LevelOnePlayerGameObject* character = new LevelOnePlayerGameObject("level_one_player");

	SphereVolume* volume = new SphereVolume(radius);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), sphereMesh, basicTex, basicShader));
	character->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->SetFriction(0.1f);
	character->GetPhysicsObject()->SetElasticity(0.66f);
	character->GetPhysicsObject()->InitSphereInertia();

	character->SetType(levelOnePlayerType);
	character->SetName("level_one_player");
	world->AddGameObject(character);
	world->SetLevelOnePlayer(character);

	levelOnePlayer = character;
	return character;
}

GameObject* TutorialGame::AddLevelOneStartBlock() {
	Vector3 start = Vector3(5, -90, 2);
	Vector3 end = Vector3(15, -95, 12);

	GameObject* block = new GameObject("level_one_start_block");
	AABBVolume* volume = new AABBVolume(
		Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
		* Vector3(0.5, 0.5, 0.5)
	);
	Vector3 finalPosition = Vector3(0.5 * std::abs(end.x - start.x), -0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + start;

	block->SetBoundingVolume((CollisionVolume*)volume);
	block->GetTransform()
		.SetPosition(finalPosition)
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	block->SetRenderObject(new RenderObject(&block->GetTransform(), cubeMesh, basicTex, basicShader));
	block->SetPhysicsObject(new PhysicsObject(&block->GetTransform(), block->GetBoundingVolume()));
	block->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	block->GetPhysicsObject()->SetInverseMass(0);
	block->GetPhysicsObject()->InitCubeInertia();

	block->GetPhysicsObject()->SetElasticity(0.0f);
	block->SetType(levelOneStartBlockType);

	world->AddGameObject(block);

	return block;
}

GameObject* TutorialGame::AddLevelOneMovingPlatform() {
	MovingPlatform* platform = new MovingPlatform();
	Vector3 start = Vector3(130, -20, 2);
	Vector3 end = Vector3(170, -25, 12);

	OBBVolume* volume = new OBBVolume(
		Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
		* Vector3(0.5, 0.5, 0.5)
	);
	Vector3 finalPosition = Vector3(0.5 * std::abs(end.x - start.x), -0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + start;
	finalPosition += Vector3(-50, 0, 0);

	platform->SetBoundingVolume((CollisionVolume*)volume);
	platform->GetTransform()
		.SetPosition(finalPosition)
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	platform->SetRenderObject(new RenderObject(&platform->GetTransform(), cubeMesh, basicTex, basicShader));
	platform->SetPhysicsObject(new PhysicsObject(&platform->GetTransform(), platform->GetBoundingVolume()));
	platform->GetRenderObject()->SetColour(Vector4(0.7, 0.5, 0, 1));

	platform->GetPhysicsObject()->SetInverseMass(0.01f);
	platform->GetPhysicsObject()->SetElasticity(0);
	platform->GetPhysicsObject()->SetFriction(1.0f);
	platform->GetPhysicsObject()->InitCubeInertia();

	platform->SetName("level_one_moving_platform");
	platform->SetType(levelOneMovingPlatformType);

	world->AddGameObject(platform);

	FloatConstraint* constraint = new FloatConstraint(platform, -25);
	world->AddConstraint(constraint);

	return platform;
}

GameObject* TutorialGame::AddLevelOneMotorPlane() {
	levelOneMotorPlane = new MotorPlane();
	Vector3 start = Vector3(160, -20, 2);
	Vector3 end = Vector3(175, -22, 12);

	OBBVolume* volume = new OBBVolume(
		Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
		* Vector3(0.5, 0.5, 0.5)
	);
	Vector3 finalPosition = Vector3(0.5 * std::abs(end.x - start.x), -0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + start;
	finalPosition += Vector3(-20, 10, 0);

	levelOneMotorPlane->SetBoundingVolume((CollisionVolume*)volume);
	levelOneMotorPlane->GetTransform()
		.SetPosition(finalPosition)
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	levelOneMotorPlane->SetRenderObject(new RenderObject(&levelOneMotorPlane->GetTransform(), cubeMesh, basicTex, basicShader));
	levelOneMotorPlane->SetPhysicsObject(new PhysicsObject(&levelOneMotorPlane->GetTransform(), levelOneMotorPlane->GetBoundingVolume()));
	levelOneMotorPlane->GetRenderObject()->SetColour(Vector4(0.8, 0.4, 0.9, 1));

	levelOneMotorPlane->GetPhysicsObject()->SetInverseMass(0.03f);
	levelOneMotorPlane->GetPhysicsObject()->SetElasticity(0);
	levelOneMotorPlane->GetPhysicsObject()->SetFriction(1.0f);
	levelOneMotorPlane->GetPhysicsObject()->InitCubeInertia();

	levelOneMotorPlane->SetName("level_one_motor_plane");
	levelOneMotorPlane->SetType(levelOneMotorPlaneType);

	world->AddGameObject(levelOneMotorPlane);

	FloatConstraint* fConstraint = new FloatConstraint(levelOneMotorPlane, -10);
	world->AddConstraint(fConstraint);

	return levelOneMotorPlane;
}

void TutorialGame::AddLevelOneRamp() {
	Vector3 starts[4] = { Vector3(70, -40, 2), Vector3(50, -70, 2),Vector3(160, -80,2), Vector3(50, -80, 2) };
	Vector3 ends[4] = { Vector3(190, -45, 12), Vector3(170, -75, 12), Vector3(165, -95,12), Vector3(55, -95, 12) };

	for (int i = 0; i < 3; i++) {
		Vector3 start = starts[i];
		Vector3 end = ends[i];
		int type = 0;
		string name = "";
		float inverseMass = 0.0f;
		float rotateAngle = 0.0f;
		float elasticity = 0.4f;
		if (i == 0) {
			type = levelOneRampType;
			name = "level_one_ramp";
			inverseMass = 0;
			rotateAngle = 10.0f;
		}
		if (i == 1) {
			type = levelOneRampType;
			name = "level_one_ramp";
			inverseMass = 0.01f;
			rotateAngle = -0.1f;
			elasticity = 2.0f;
		}
		if (i == 2) {
			// move available
			type = levelOneRampPillarType;
			name = "level_one_ramp_pillar";
			inverseMass = 10.0f;
			rotateAngle = 0.0f;
		}
		/*if (i == 3) {
			type = levelOneRampPillarType;
			name = "level_one_ramp_pillar";
			inverseMass = 0.0f;
			rotateAngle = 0.0f;
		}*/

		GameObject* ramp = new GameObject(name);
		OBBVolume* volume = new OBBVolume(
			Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
			* Vector3(0.5, 0.5, 0.5)
		);
		Vector3 finalPosition = Vector3(0.5 * std::abs(end.x - start.x), -0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + start;

		ramp->SetBoundingVolume((CollisionVolume*)volume);
		ramp->GetTransform()
			.SetPosition(finalPosition)
			.SetOrientation(Quaternion::Quaternion(Matrix4::Rotation(rotateAngle, Vector3(0, 0, 1))))
			.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
		ramp->SetRenderObject(new RenderObject(&ramp->GetTransform(), cubeMesh, basicTex, basicShader));
		ramp->SetPhysicsObject(new PhysicsObject(&ramp->GetTransform(), ramp->GetBoundingVolume()));
		if (i == 1) {
			ramp->GetRenderObject()->SetColour(Vector4(0.1, 1, 0.9, 1));
		}
		else {
			ramp->GetRenderObject()->SetColour(Vector4(0.5, 0.5, 0.5, 1));
		}

		ramp->GetPhysicsObject()->SetInverseMass(inverseMass);
		ramp->GetPhysicsObject()->InitCubeInertia();

		ramp->GetPhysicsObject()->SetElasticity(elasticity);
		ramp->GetPhysicsObject()->SetFriction(0.8f);
		ramp->SetType(type);

		if (i == 2) {
			levelOneRampPhillar = ramp;
		}
		// the second ramp
		// set the float constraint to lower to make it fall to one side
		if (i == 1) {
			levelOneMovableRamp = ramp;
			FloatConstraint* constraint = new FloatConstraint(levelOneMovableRamp, -82);
			world->AddConstraint(constraint);
		}

		world->AddGameObject(ramp);
	}
}

GameObject* TutorialGame::AddLevelOneEndBlock() {
	Vector3 start = Vector3(175, -93, 2);
	Vector3 end = Vector3(195, -95, 12);

	GameObject* block = new GameObject("level_one_end_block");
	OBBVolume* volume = new OBBVolume(
		Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
		* Vector3(0.5, 0.5, 0.5)
	);
	Vector3 finalPosition = Vector3(0.5 * std::abs(end.x - start.x), -0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + start;

	block->SetBoundingVolume((CollisionVolume*)volume);
	block->GetTransform()
		.SetPosition(finalPosition)
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	block->SetRenderObject(new RenderObject(&block->GetTransform(), cubeMesh, basicTex, basicShader));
	block->SetPhysicsObject(new PhysicsObject(&block->GetTransform(), block->GetBoundingVolume()));
	block->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	block->GetPhysicsObject()->SetInverseMass(0);
	block->GetPhysicsObject()->InitCubeInertia();

	block->GetPhysicsObject()->SetElasticity(0.0f);
	block->SetType(levelOneEndBlockType);

	world->AddGameObject(block);

	return block;
}

void TutorialGame::AddLevelOneChain() {

	Vector3 cubeSize = Vector3(0.5, 0.5, 0.5);

	float invCubeMass = 5; // how heavy the middle pieces are
	int numLinks = 5;
	float maxDistance = 2; // constraint distance
	float cubeDistance = 1; // distance between links

	Vector3 startPos = Vector3(50, -50, 7);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCapsuleToWorld(Vector3(35, -50, 7), 2.5, 1.5, 0.0f);
	end->SetType(levelOneChainEndType);
	end->GetTransform().SetOrientation(Quaternion::Quaternion(Matrix4::Rotation(90, Vector3(0, 0, 1))));
	end->GetRenderObject()->SetColour(Vector4(0.6, 0.5, 1, 1));

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i)
	{
		GameObject* block = AddCubeToWorld(startPos + Vector3(-(i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		world->AddConstraint(new AngularConstraint(previous, block, 0, true));
		world->AddConstraint(new AngularConstraint(previous, block, 0, false));
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::AddLevelOneBonus() {
	Vector3 startPos = Vector3(100, -40, 7);

	for (int i = 0; i < 3; i++) {
		GameObject* bonus = new GameObject("level_one_bonus");

		SphereVolume* volume = new SphereVolume(0.25f);
		bonus->SetBoundingVolume((CollisionVolume*)volume);
		bonus->GetTransform()
			.SetScale(Vector3(0.25, 0.25, 0.25))
			.SetPosition(startPos + Vector3(i*15, i* 3, 0));

		bonus->SetRenderObject(new RenderObject(&bonus->GetTransform(), bonusMesh, nullptr, basicShader));
		bonus->GetRenderObject()->SetColour(Vector4(0.9, 0.9, 0.2, 1));
		bonus->SetPhysicsObject(new PhysicsObject(&bonus->GetTransform(), bonus->GetBoundingVolume()));

		bonus->GetPhysicsObject()->SetInverseMass(0.0f);
		bonus->GetPhysicsObject()->InitSphereInertia();
		bonus->SetType(levelOneBonusType);
		bonus->SetName("level_one_bonus");

		world->AddGameObject(bonus);
	}
}

// level 2 stuff
void TutorialGame::InitLevelTwo() {
	// todo
	gameOver = false;
	tutorial = false;
	LevelOne = false;
	menu = false;

	lockedObject = nullptr;

	world->ClearAndErase();
	physics->Clear();

	useGravity = true;
	physics->UseGravity(useGravity);
	physics->UseBroadPhase(true);

	// init camera
	world->GetMainCamera()->SetPosition(Vector3(100, 300, 100));
	world->GetMainCamera()->SetPitch(-90);
	world->GetMainCamera()->SetYaw(0);
	//world->GetMainCamera()->SetIsActive(false);

	GenerateLevelTwoMaze("maze2.txt");
}

void TutorialGame::GenerateLevelTwoMaze(const std::string& filename) {

	// floor
	{
		GameObject* floor = new GameObject("floor");

		Vector3 floorSize = Vector3(100, 2, 100);
		AABBVolume* volume = new AABBVolume(floorSize);
		floor->SetBoundingVolume((CollisionVolume*)volume);
		floor->GetTransform()
			.SetScale(floorSize * 2)
			.SetPosition(Vector3(95, -3.01, 95))
			;
		floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
		floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

		floor->GetPhysicsObject()->SetInverseMass(0);
		floor->GetPhysicsObject()->InitCubeInertia();

		world->AddGameObject(floor);
	}
	
	// maze wall
	{
		std::ifstream infile(Assets::DATADIR + filename);

		int nodeSize;
		int gridWidth;
		int gridHeight;

		infile >> nodeSize;
		infile >> gridWidth;
		infile >> gridHeight;

		for (int y = 0; y < gridHeight; ++y) {
			for (int x = 0; x < gridWidth; ++x) {
				char type = 0;
				infile >> type;
				if (type == 'x' && y != 0 && x != 0 && x != gridWidth - 1 && y != gridHeight - 1) { //not doing outside walls to prevent fps drop
					AddLevelTwoWallToWorld(Vector3(x * 10, 4, y * 10), Vector3(5, 5, 5));
				}
			}
		}
	}

	//edge
	{
		AddLevelTwoWallToWorld(Vector3(0, 4, 95), Vector3(5, 5, 100));
		AddLevelTwoWallToWorld(Vector3(190, 4, 95), Vector3(5, 5, 100));
		AddLevelTwoWallToWorld(Vector3(95, 4, 0), Vector3(90, 5, 5));
		AddLevelTwoWallToWorld(Vector3(95, 4, 190), Vector3(90, 5, 5));
	}

	// add start and end block
	AddLevelTwoStartAndEndBlock();

	// add speed up block
	AddLevelTwoSpeedUpBlockToWorld();

	// add door switch block
	AddLevelTwoSwitchDoorBlock();

	// add bonus;
	GameObject* levelTwoBonus = AddBonusToWorld(Vector3(20, 6, 100));
	levelTwoBonus->SetName("level_two_bonus");
	levelTwoBonus->GetPhysicsObject()->SetInverseMass(0.0f);
	levelTwoBonus->GetRenderObject()->SetColour(Vector4(0.9, 1, 0.6, 1));

	// add player
	player = AddLevelTwoPlayerToWorld(Vector3(10, 5, 10));

	// add enemy
	enemy = AddBehaviourEnemyToWorld(Vector3(10, 5, 180));

	// add state open door
	AddStateDoorToWorld(Vector3(105, 14, 145), Vector3(115, 4, 155));

	// add motor block
	AddLevelTwoMotorBlock();

}

GameObject* TutorialGame::AddLevelTwoWallToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::AddLevelTwoSpeedUpBlockToWorld() {
	Vector3 start = Vector3(5, 0.001, 45);
	Vector3 end = Vector3(15, 0, 55);

	GameObject* block = new GameObject("speed_up_block");
	AABBVolume* volume = new AABBVolume(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	block->SetBoundingVolume((CollisionVolume*)volume);
	block->GetTransform()
		.SetPosition(Vector3(0.5 * std::abs(end.x - start.x), 0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + Vector3(start.x, -0.5, start.z))
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	block->SetRenderObject(new RenderObject(&block->GetTransform(), cubeMesh, basicTex, basicShader));
	block->SetPhysicsObject(new PhysicsObject(&block->GetTransform(), block->GetBoundingVolume()));

	block->GetPhysicsObject()->SetInverseMass(0);
	block->GetPhysicsObject()->InitCubeInertia();

	block->GetPhysicsObject()->SetElasticity(1.0f);
	block->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
	block->SetType(levelTwoSpeedUpBlockType);

	world->AddGameObject(block);
}

void TutorialGame::AddLevelTwoStartAndEndBlock() {
	char types[2] = { 's', 'e' };
	Vector3 starts[2] = {Vector3(5, 0.001, 5), Vector3(140, 0.001, 125) };
	Vector3 ends[2] = {Vector3(15, 0, 15), Vector3(150, 0, 135) };

	for (int i = 0; i < 2; i++) {
		char type = types[i];
		Vector3 start = starts[i];
		Vector3 end = ends[i];

		bool isStart = (type == 's');
		GameObject* block = new GameObject(isStart ? "levelTwoStartPos" : "levelTwoEndPos");
		AABBVolume* volume = new AABBVolume(
			Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
		);
		block->SetBoundingVolume((CollisionVolume*)volume);
		block->GetTransform()
			.SetPosition(Vector3(0.5 * std::abs(end.x - start.x), 0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + Vector3(start.x, -0.5, start.z))
			.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
		block->SetRenderObject(new RenderObject(&block->GetTransform(), cubeMesh, basicTex, basicShader));
		block->SetPhysicsObject(new PhysicsObject(&block->GetTransform(), block->GetBoundingVolume()));

		block->GetPhysicsObject()->SetInverseMass(0);
		block->GetPhysicsObject()->InitCubeInertia();

		block->GetPhysicsObject()->SetElasticity(1.0f);
		block->GetRenderObject()->SetColour(isStart ? Vector4(0, 1, 0, 1) : Vector4(1, 0, 0, 1));
		block->SetType(isStart ? levelTwoStartPosType : levelTwoEndPosType);

		world->AddGameObject(block);
	}
}

void TutorialGame::AddLevelTwoSwitchDoorBlock() {
	Vector3 start = Vector3(175, 0.001, 5);
	Vector3 end = Vector3(185, 0, 15);
	GameObject* block = new GameObject("end_door_switch_block");
	AABBVolume* volume = new AABBVolume(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	block->SetBoundingVolume((CollisionVolume*)volume);
	block->GetTransform()
		.SetPosition(Vector3(0.5 * std::abs(end.x - start.x), 0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + Vector3(start.x, -0.5, start.z))
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	block->SetRenderObject(new RenderObject(&block->GetTransform(), cubeMesh, basicTex, basicShader));
	block->SetPhysicsObject(new PhysicsObject(&block->GetTransform(), block->GetBoundingVolume()));

	block->GetPhysicsObject()->SetInverseMass(0);
	block->GetPhysicsObject()->InitCubeInertia();

	block->GetPhysicsObject()->SetElasticity(1.0f);
	block->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));
	block->SetType(levelTwoDoorSwitchType);

	world->AddGameObject(block);
}

BehaviourTreeEnemy* TutorialGame::AddBehaviourEnemyToWorld(const Vector3& position) {
	int radius = 2.0f;
	float inverseMass = 2.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);

	BehaviourTreeEnemy* character = new BehaviourTreeEnemy(world, position);

	SphereVolume* volume = new SphereVolume(radius);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), sphereMesh, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(1, 0, 0, 1));

	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	character->SetType(enemyType);
	character->SetName("level_two_enemy");
	world->AddGameObject(character);

	return character;
}

PlayerGameObject* TutorialGame::AddLevelTwoPlayerToWorld(const Vector3& position) {
	int radius = 2.0f;
	float inverseMass = 2.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);

	PlayerGameObject* character = new PlayerGameObject("player");

	SphereVolume* volume = new SphereVolume(radius);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), sphereMesh, nullptr, basicShader));
	character->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	character->SetType(playerType);
	character->SetName("level_two_player");
	world->AddGameObject(character);
	world->SetPlayer(character);

	return character;
}

StateDoorGameObject* TutorialGame::AddStateDoorToWorld(const Vector3& start, const Vector3& end) {
	StateDoorGameObject* door = new StateDoorGameObject(start, end, world);

	door->SetName("level_two_end_door");
	door->SetType(levelTwoDoorType);
	OBBVolume* volume = new OBBVolume(
		Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
		* Vector3(0.5, 0.5, 0.5)
	);
	door->SetBoundingVolume((CollisionVolume*)volume);
	door->preVolume = (CollisionVolume*)volume;

	door->GetTransform()
		.SetPosition(Vector3(0.5 * std::abs(end.x - start.x), 0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z)) + Vector3(start.x, 0, start.z))
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	door->SetRenderObject(new RenderObject(&door->GetTransform(), cubeMesh, basicTex, basicShader));
	door->GetRenderObject()->SetColour(Vector4(0, 1, 1, 1));

	door->SetPhysicsObject(new PhysicsObject(&door->GetTransform(), door->GetBoundingVolume()));

	door->GetPhysicsObject()->SetInverseMass(0);
	door->GetPhysicsObject()->InitCubeInertia();

	door->GetPhysicsObject()->SetElasticity(1.0f);
	door->SetType(wallType);

	world->AddGameObject(door);
	return door;
}

void TutorialGame::AddLevelTwoMotorBlock() {
	MotorPlane* levelTwoMotorBlock = new MotorPlane();
	Vector3 start = Vector3(60, 14, 100);
	Vector3 end = Vector3(61, 4, 120);

	OBBVolume* volume = new OBBVolume(
		Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z))
		* Vector3(0.5, 0.5, 0.5)
	);
	Vector3 finalPosition = Vector3(0.5 * std::abs(end.x - start.x), 0.5 * std::abs(end.y - start.y), 0.5 * std::abs(end.z - start.z))
		+ Vector3(start.x, 0, start.z);

	levelTwoMotorBlock->SetBoundingVolume((CollisionVolume*)volume);
	levelTwoMotorBlock->GetTransform()
		.SetPosition(finalPosition)
		.SetScale(Vector3(std::abs(end.x - start.x), std::abs(end.y - start.y), std::abs(end.z - start.z)));
	levelTwoMotorBlock->SetRenderObject(new RenderObject(&levelTwoMotorBlock->GetTransform(), cubeMesh, basicTex, basicShader));
	levelTwoMotorBlock->SetPhysicsObject(new PhysicsObject(&levelTwoMotorBlock->GetTransform(), levelTwoMotorBlock->GetBoundingVolume()));
	levelTwoMotorBlock->GetRenderObject()->SetColour(Vector4(0.8, 0.4, 0.9, 1));

	levelTwoMotorBlock->GetPhysicsObject()->SetInverseMass(0.3f);
	levelTwoMotorBlock->GetPhysicsObject()->SetElasticity(0);
	levelTwoMotorBlock->GetPhysicsObject()->SetFriction(1.0f);
	levelTwoMotorBlock->GetPhysicsObject()->InitCubeInertia();

	levelTwoMotorBlock->SetName("level_two_motor_block");
	levelTwoMotorBlock->SetType(levelTwoMotorBlockType);

	world->AddGameObject(levelTwoMotorBlock);

	FloatConstraint* fConstraint = new FloatConstraint(levelTwoMotorBlock, 3);
	world->AddConstraint(fConstraint);

	MotorConstraint* q = new MotorConstraint(levelTwoMotorBlock, 0, 5, 0);
	world->AddConstraint(q);
}

// toturial part
void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);

	float invCubeMass = 5; // how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(0, 0, 0);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i)
	{
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		world->AddConstraint(new AngularConstraint(previous, block, 0, true));
		world->AddConstraint(new AngularConstraint(previous, block, 0, false));
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& size, const Vector3& position, const Quaternion& rotation, const Vector3& offset) {
	GameObject* floor = new GameObject("Floor");

	Vector3 floorSize = Vector3(100, 2, 100);
	AABBVolume* volume = new AABBVolume(floorSize * Vector3(1, 0.5, 1));
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize)
		.SetOrientation(rotation)
		.SetPosition(position + offset);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	floor->SetType(floorType);

	world->AddGameObject(floor);

	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple'
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, float elasticity) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	sphere->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass, float elasticity) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius * 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	capsule->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, float elasticity) {
	GameObject* cube = new GameObject("AABB Cube");

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	cube->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBCubeToWorld(const Vector3& position, Vector3 dimensions, float elasticity, float friction, Vector4 colour, float inverseMass)
{
	GameObject* cube = new GameObject("OBB Cube");

	OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);


	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	cube->GetPhysicsObject()->SetElasticity(elasticity);

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(100, 2, 100), Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols + 1; ++x) {
		for (int z = 1; z < numRows + 1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(100, 2, 100), Vector3(0, -2, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position)
{
	StateGameObject* tStateObject = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	tStateObject->SetBoundingVolume((CollisionVolume*)volume);
	tStateObject->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	tStateObject->SetRenderObject(new RenderObject(&tStateObject->GetTransform(), bonusMesh, nullptr, basicShader));
	tStateObject->SetPhysicsObject(new PhysicsObject(&tStateObject->GetTransform(), tStateObject->GetBoundingVolume()));

	tStateObject->GetPhysicsObject()->SetInverseMass(1.0f);
	tStateObject->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(tStateObject);

	return tStateObject;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around.

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				lockedObject = nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if (selectionObject) {
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	renderer->DrawString("Click Force: " + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;
	}
	// push the selected object
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
}