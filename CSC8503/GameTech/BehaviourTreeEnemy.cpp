#include "BehaviourTreeEnemy.h"

BehaviourTreeEnemy::BehaviourTreeEnemy(GameWorld* a, const Vector3& initPosition)
{
	worldRef = a;

	leftSpeedUpTime = 0;
	isSpeedUp = false;
	prePos = initPosition;

	BehaviourAction* SeekBonus = new BehaviourAction
	("Seek Bonus", [&](float dt, BehaviourState state) -> BehaviourState
		{
			GameObject* levelTwoBonus = worldRef->GetLevelTwoBonus();
			PlayerGameObject* player = worldRef->GetPlayer();

			Vector3 currentPos = GetTransform().GetPosition();
			Vector3 targetPos = player->GetTransform().GetPosition();
			Vector3 levelTwoBonusPos = levelTwoBonus->GetTransform().GetPosition();

			float distToBonus = (levelTwoBonusPos - currentPos).Length();
			float distToPlayer = (targetPos - currentPos).Length();

			if (state == Initialise)
			{
				Vector3 offset = Vector3((levelTwoBonusPos - currentPos).x, 0, (levelTwoBonusPos - currentPos).z);
				Vector3 newStart = currentPos + offset.Normalised() * 3;
				Ray rayFromEnemy(newStart, (levelTwoBonusPos - newStart).Normalised());
				//Debug::DrawLine(newStart, levelTwoBonusPos, Vector4(1, 1, 1, 1), 1);
				RayCollision closestCollision;
				if (worldRef->Raycast(rayFromEnemy, closestCollision, true)) {
					if (closestCollision.node != nullptr) {
						GameObject* firstObject = (GameObject*)closestCollision.node;
						if (firstObject->GetName() == "level_two_bonus" && firstObject->IsActive() == true) {
							Vector3 startPos = currentPos;

							// calc all kinds paths
							try
							{
								{
									//grid base
									NavigationGrid levelTwoGrid("maze2.txt");
									NavigationPath gridOutPath;
									bool found = levelTwoGrid.FindPath(startPos, levelTwoBonusPos, gridOutPath);
									levelTwoPathNodes.clear();
									levelTwoPathNodes = gridOutPath.GetWaypoints();
									gridBaseL = 0;
									if (levelTwoPathNodes.size()) {
										for (int i = 0; i < levelTwoPathNodes.size() - 1; i++) {
											gridBaseL += (levelTwoPathNodes[i + 1] - levelTwoPathNodes[i]).Length();
										}
									}

									// mesh base
									navMesh = new NavigationMesh("maze2.navmesh");
									NavigationPath meshOutPath;
									found = navMesh->FindPath(startPos, levelTwoBonusPos, meshOutPath);

									//smooth path
									meshOutPath.Smoothing(worldRef);
									meshSmoothNodes.clear();
									meshSmoothNodes = meshOutPath.GetWaypoints();

									meshSmoothL = 0;
									if (meshSmoothNodes.size()) {
										for (int i = 0; i < meshSmoothNodes.size() - 1; i++) {
											meshSmoothL += (meshSmoothNodes[i + 1] - meshSmoothNodes[i]).Length();
										}
									}

									// string path
									NavigationPath meshStringPath;
									found = navMesh->FindStringPath(startPos, levelTwoBonusPos, meshStringPath);
									meshStringNodes.clear();
									meshStringNodes = meshStringPath.GetWaypoints();

									meshStringL = 0;
									if (meshStringNodes.size()) {
										for (int i = 0; i < meshStringNodes.size() - 1; i++) {
											meshStringL += (meshStringNodes[i + 1] - meshStringNodes[i]).Length();
										}
									}
								}
							}
							catch (const std::exception& e)
							{
								std::cout << "error: " << e.what() << std::endl;
								// give bias position to previous
								GetTransform().SetPosition(prePos);
								return Ongoing;
							}

							// calc shortest paths
							{
								if (gridBaseL < meshSmoothL) {
									shortestNodes = levelTwoPathNodes;
									shortestL = gridBaseL;
								}
								else {
									shortestNodes = meshSmoothNodes;
									shortestL = meshSmoothL;
								}
								if (shortestL > meshStringL) {
									shortestNodes = meshStringNodes;
									shortestL = meshStringL;
								}

								tempIndex = shortestNodes.size() - 2;
							}

							return Ongoing;
						}
						else {
							return Failure;
						}
					}
					else {
						return Failure;
					}
				}
			}
			else if (state == Ongoing)
			{
				Vector3 offset = Vector3((levelTwoBonusPos - currentPos).x, 0, (levelTwoBonusPos - currentPos).z);
				Vector3 newStart = currentPos + offset.Normalised() * 3;
				Ray rayFromEnemy(newStart, (levelTwoBonusPos - newStart).Normalised());
				//Debug::DrawLine(newStart, levelTwoBonusPos, Vector4(1, 1, 1, 1), 1);
				RayCollision closestCollision;
				if (worldRef->Raycast(rayFromEnemy, closestCollision, true)) {
					if (closestCollision.node != nullptr) {
						GameObject* firstObject = (GameObject*)closestCollision.node;
						if (firstObject->GetName() == "level_two_bonus" && firstObject->IsActive() == true) {
						
						}
						else {
							return Failure;
						}
					}
					else {
						return Failure;
					}
				}


				if (distToBonus > 2.5f) {

					// draw paths
					{
						if (levelTwoPathNodes.size() >= 2) {
							for (int i = 1; i <= levelTwoPathNodes.size() - 1; ++i) {
								Vector3 a = levelTwoPathNodes[i - 1] + Vector3(0, 1, 0);
								Vector3 b = levelTwoPathNodes[i] + Vector3(0, 1, 0);

								Debug::DrawLine(a, b, Vector4(1, 0, 0, 1));
							}
						}

						if (meshSmoothNodes.size() >= 2) {
							for (int i = 1; i <= meshSmoothNodes.size() - 1; ++i) {
								Vector3 a = meshSmoothNodes[i - 1];
								Vector3 b = meshSmoothNodes[i];

								Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
							}
						}

						if (meshStringNodes.size() >= 2) {
							for (int i = 1; i <= meshStringNodes.size() - 1; ++i) {
								Vector3 a = meshStringNodes[i - 1];
								Vector3 b = meshStringNodes[i];

								Debug::DrawLine(a, b, Vector4(1, 0.5, 0, 1));
							}
						}

					}

					if (shortestNodes.size() <= 0) {
						return Failure;
					}

					if ((currentPos - shortestNodes[tempIndex]).Length() > 2.5f) {
						speedFactor = isSpeedUp ? 40 : 30;

						Vector3 forward = (shortestNodes[tempIndex] - currentPos);
						SetSpeedUpTime(leftSpeedUpTime - dt);
						if (leftSpeedUpTime < 0.0f) {
							isSpeedUp = false;
						}
						forward.Normalise();
						prePos = GetTransform().GetPosition();
						GetTransform().SetPosition(GetTransform().GetPosition() + (forward * speedFactor * dt));
						return Ongoing;
					}
					else {
						tempIndex = tempIndex - 1 < 0 ? 0 : tempIndex - 1;
						return Ongoing;
					}

					return Ongoing;
				}
				return Failure;
			}
			return state;
		}
	);

	BehaviourAction* SeekSpeedUpBlock = new BehaviourAction
	("Seek Speed Up Block", [&](float dt, BehaviourState state) -> BehaviourState
		{
			GameObject* nearestSpeedUpBlock = worldRef->GetNearestSpeedUpBlock();
			PlayerGameObject* player = worldRef->GetPlayer();

			Vector3 currentPos = GetTransform().GetPosition();
			Vector3 targetPos = player->GetTransform().GetPosition();
			Vector3 nearstSpeedUpBlockPos = nearestSpeedUpBlock->GetTransform().GetPosition();

			float distToNearstSpeedUpBlock = (nearstSpeedUpBlockPos - currentPos).Length();
			float distToPlayer = (targetPos - currentPos).Length();

			if (state == Initialise)
			{
				if (isSpeedUp) {
					return Failure;
				}
				if (distToPlayer < distToNearstSpeedUpBlock) {
					return Failure;
				}


				Vector3 startPos = currentPos;

				// calc all kinds paths
				try
				{
					{
						//grid base
						NavigationGrid levelTwoGrid("maze2.txt");
						NavigationPath gridOutPath;
						bool found = levelTwoGrid.FindPath(startPos, nearstSpeedUpBlockPos, gridOutPath);
						levelTwoPathNodes.clear();
						levelTwoPathNodes = gridOutPath.GetWaypoints();
						gridBaseL = 0;
						if (levelTwoPathNodes.size()) {
							for (int i = 0; i < levelTwoPathNodes.size() - 1; i++) {
								gridBaseL += (levelTwoPathNodes[i + 1] - levelTwoPathNodes[i]).Length();
							}
						}

						// mesh base
						navMesh = new NavigationMesh("maze2.navmesh");
						NavigationPath meshOutPath;
						found = navMesh->FindPath(startPos, nearstSpeedUpBlockPos, meshOutPath);

						//smooth path
						meshOutPath.Smoothing(worldRef);
						meshSmoothNodes.clear();
						meshSmoothNodes = meshOutPath.GetWaypoints();

						meshSmoothL = 0;
						if (meshSmoothNodes.size()) {
							for (int i = 0; i < meshSmoothNodes.size() - 1; i++) {
								meshSmoothL += (meshSmoothNodes[i + 1] - meshSmoothNodes[i]).Length();
							}
						}

						// string path
						NavigationPath meshStringPath;
						found = navMesh->FindStringPath(startPos, nearstSpeedUpBlockPos, meshStringPath);
						meshStringNodes.clear();
						meshStringNodes = meshStringPath.GetWaypoints();

						meshStringL = 0;
						if (meshStringNodes.size()) {
							for (int i = 0; i < meshStringNodes.size() - 1; i++) {
								meshStringL += (meshStringNodes[i + 1] - meshStringNodes[i]).Length();
							}
						}
					}
				}
				catch (const std::exception& e)
				{
					std::cout << "error: " << e.what() << std::endl;
					// give bias position to previous
					GetTransform().SetPosition(prePos);
					return Ongoing;
				}
				
				// calc shortest paths
				{
					if (gridBaseL < meshSmoothL) {
						shortestNodes = levelTwoPathNodes;
						shortestL = gridBaseL;
					}
					else {
						shortestNodes = meshSmoothNodes;
						shortestL = meshSmoothL;
					}
					if (shortestL > meshStringL) {
						shortestNodes = meshStringNodes;
						shortestL = meshStringL;
					}

					tempIndex = shortestNodes.size() - 2;
				}
				
				return Ongoing;

			}
			else if (state == Ongoing)
			{
				if (isSpeedUp) {
					return Failure;
				}
				if (distToPlayer < distToNearstSpeedUpBlock) {
					return Failure;
				}
				if (distToNearstSpeedUpBlock > 0.0f) {

					// draw paths
					{
						if (levelTwoPathNodes.size() >= 2) {
							for (int i = 1; i <= levelTwoPathNodes.size() - 1; ++i) {
								Vector3 a = levelTwoPathNodes[i - 1] + Vector3(0, 1, 0);
								Vector3 b = levelTwoPathNodes[i] + Vector3(0, 1, 0);

								Debug::DrawLine(a, b, Vector4(1, 0, 0, 1));
							}
						}

						if (meshSmoothNodes.size()>=2) {
							for (int i = 1; i <= meshSmoothNodes.size() - 1; ++i) {
								Vector3 a = meshSmoothNodes[i - 1];
								Vector3 b = meshSmoothNodes[i];

								Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
							}
						}

						if (meshStringNodes.size() >= 2) {
							for (int i = 1; i <= meshStringNodes.size() - 1; ++i) {
								Vector3 a = meshStringNodes[i - 1];
								Vector3 b = meshStringNodes[i];

								Debug::DrawLine(a, b, Vector4(1, 0.5, 0, 1));
							}
						}
						
					}

					if (shortestNodes.size() <= 0) {
						return Ongoing;
					}

					if ((currentPos - shortestNodes[tempIndex]).Length() > 1.0f) {
						speedFactor = isSpeedUp ? 40 : 30;

						Vector3 forward = (shortestNodes[tempIndex] - currentPos);
						SetSpeedUpTime(leftSpeedUpTime - dt);
						if (leftSpeedUpTime < 0.0f) {
							isSpeedUp = false;
						}
						forward.Normalise();
						prePos = GetTransform().GetPosition();
						GetTransform().SetPosition(GetTransform().GetPosition() + (forward * speedFactor * dt));
						return Ongoing;
					}
					else {
						tempIndex = tempIndex - 1 < 0 ? 0 : tempIndex - 1;
						return Ongoing;
					}

					return Ongoing;
				}
				return Success;
			}
			return state;
		}
	);

	BehaviourAction* SeekPlayer = new BehaviourAction
	("Seek Player", [&](float dt, BehaviourState state) -> BehaviourState
		{
			GameObject* nearestSpeedUpBlock = worldRef->GetNearestSpeedUpBlock();
			PlayerGameObject* player = worldRef->GetPlayer();

			Vector3 currentPos = GetTransform().GetPosition();
			Vector3 targetPos = player->GetTransform().GetPosition();
			Vector3 nearstSpeedUpBlockPos = nearestSpeedUpBlock->GetTransform().GetPosition();

			float distToPlayer = (targetPos - currentPos).Length();
			float distToNearstSpeedUpBlock = (nearstSpeedUpBlockPos - currentPos).Length();

			if (state == Initialise)
			{
				if (!isSpeedUp && distToPlayer > distToNearstSpeedUpBlock) {
					return Failure;
				}

				return Ongoing;
			}
			else if (state == Ongoing)
			{

				Vector3 startPos = currentPos;

				// calc all kinds paths
				try
				{
					{
						//grid base
						NavigationGrid levelTwoGrid("maze2.txt");
						NavigationPath gridOutPath;
						bool found = levelTwoGrid.FindPath(startPos, targetPos, gridOutPath);
						levelTwoPathNodes.clear();
						levelTwoPathNodes = gridOutPath.GetWaypoints();
						gridBaseL = 0;
						if (levelTwoPathNodes.size()) {
							for (int i = 0; i < levelTwoPathNodes.size() - 1; i++) {
								gridBaseL += (levelTwoPathNodes[i + 1] - levelTwoPathNodes[i]).Length();
							}
						}

						// mesh base
						navMesh = new NavigationMesh("maze2.navmesh");
						NavigationPath meshOutPath;
						found = navMesh->FindPath(startPos, targetPos, meshOutPath);

						//smooth path
						meshOutPath.Smoothing(worldRef);
						meshSmoothNodes.clear();
						meshSmoothNodes = meshOutPath.GetWaypoints();

						meshSmoothL = 0;
						if (meshSmoothNodes.size()) {
							for (int i = 0; i < meshSmoothNodes.size() - 1; i++) {
								meshSmoothL += (meshSmoothNodes[i + 1] - meshSmoothNodes[i]).Length();
							}
						}

						// string path
						NavigationPath meshStringPath;
						found = navMesh->FindStringPath(startPos, targetPos, meshStringPath);
						meshStringNodes.clear();
						meshStringNodes = meshStringPath.GetWaypoints();

						meshStringL = 0;
						if (meshStringNodes.size()) {
							for (int i = 0; i < meshStringNodes.size() - 1; i++) {
								meshStringL += (meshStringNodes[i + 1] - meshStringNodes[i]).Length();
							}
						}
					}
				}
				catch (const std::exception& e)
				{
					std::cout << "error: " <<  e.what() << std::endl;
					// give bias position to previous
					GetTransform().SetPosition(prePos);
					return Ongoing;
				}
				
				// calc shortest paths
				{
					shortestNodes.clear();
					if (gridBaseL < meshSmoothL) {
						shortestNodes = levelTwoPathNodes;
						shortestL = gridBaseL;
					}
					else {
						shortestNodes = meshSmoothNodes;
						shortestL = meshSmoothL;
					}
					if (shortestL > meshStringL) {
						shortestNodes = meshStringNodes;
						shortestL = meshStringL;
					}

					tempIndex = shortestNodes.size() - 2;
				}

				// draw paths
				{
					if (levelTwoPathNodes.size() >= 2) {
						for (int i = 1; i <= levelTwoPathNodes.size() - 1; ++i) {
							Vector3 a = levelTwoPathNodes[i - 1] + Vector3(0, 1, 0);
							Vector3 b = levelTwoPathNodes[i] + Vector3(0, 1, 0);

							Debug::DrawLine(a, b, Vector4(1, 0, 0, 1));
						}
					}

					if (meshSmoothNodes.size() >= 2) {
						for (int i = 1; i <= meshSmoothNodes.size() - 1; ++i) {
							Vector3 a = meshSmoothNodes[i - 1];
							Vector3 b = meshSmoothNodes[i];

							Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
						}
					}

					if (meshStringNodes.size() >= 2) {
						for (int i = 1; i <= meshStringNodes.size() - 1; ++i) {
							Vector3 a = meshStringNodes[i - 1];
							Vector3 b = meshStringNodes[i];

							Debug::DrawLine(a, b, Vector4(1, 0.5, 0, 1));
						}
					}
				}

				if (shortestNodes.size() <= 0) {
					Vector3 forward = (targetPos - currentPos);
					forward.Normalise();
					prePos = GetTransform().GetPosition();
					GetTransform().SetPosition(GetTransform().GetPosition() + (forward * speedFactor * dt));
					return Ongoing;
				}

				SetSpeedUpTime(leftSpeedUpTime - dt);
				if (leftSpeedUpTime < 0.0f) {
					isSpeedUp = false;
				}

				if (isSpeedUp || distToPlayer < distToNearstSpeedUpBlock)
				{
					speedFactor = isSpeedUp ? 40 : 30;
					if (tempIndex <= 0 && (targetPos - currentPos).Length() > 1.0f) {
						Vector3 forward = (targetPos - currentPos);
						forward.Normalise();
						prePos = GetTransform().GetPosition();
						GetTransform().SetPosition(GetTransform().GetPosition() + (forward * speedFactor * dt));
						return Ongoing;
					}
					else {
						if ((currentPos - shortestNodes[tempIndex - 1]).Length() > 1.0f) {
							Vector3 forward = (shortestNodes[tempIndex - 1] - currentPos);
							forward.Normalise();
							prePos = GetTransform().GetPosition();
							GetTransform().SetPosition(GetTransform().GetPosition() + (forward * speedFactor * dt));
							return Ongoing;
						}
						else {
							tempIndex = tempIndex - 1 < 0 ? 0 : tempIndex - 1;
							return Ongoing;
						}
					}
					
					return Ongoing;
				}
				else
				{
					return Failure;
				}
			}
			return state;
		}
	);

	selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(SeekBonus);
	selection->AddChild(SeekPlayer);
	selection->AddChild(SeekSpeedUpBlock);

	rootSequence = new BehaviourSequence("Root Sequence");

	rootSequence->AddChild(selection);
}

BehaviourTreeEnemy::~BehaviourTreeEnemy()
{
	delete worldRef;

	delete rootSequence;
	delete sequence;
	delete selection;
}


void BehaviourTreeEnemy::Update(float dt)
{
	if (isActive)
	{
		state = rootSequence->Execute(dt);

		if (state != Ongoing)
		{
			rootSequence->Reset();
		}

		PlayerGameObject* player = worldRef->GetPlayer();
		if (player->won == true)
		{
			isActive = false;
			won = false;
			lost = true;
		}
		if (player->won == false && player->lost == true) {
			isActive = false;
			lost = false;
			won = true;
		}
	}
}