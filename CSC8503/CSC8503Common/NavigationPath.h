#pragma once
#include "../../Common/Vector3.h"
#include "../CSC8503Common/GameWorld.h"
#include <vector>

namespace NCL {
	using namespace NCL::Maths;
	namespace CSC8503 {
		class NavigationPath		{
		public:
			NavigationPath() {}
			~NavigationPath() {}

			void	Clear() {
				waypoints.clear();
			}
			void	PushWaypoint(const Vector3& wp) {
				waypoints.emplace_back(wp);
			}
			bool	PopWaypoint(Vector3& waypoint) {
				if (waypoints.empty()) {
					return false;
				}
				waypoint = waypoints.back();
				waypoints.pop_back();
				return true;
			}

			std::vector<Vector3> GetWaypoints() {
				return waypoints;
			}

			void Reverse() {
				std::reverse(waypoints.begin(), waypoints.end());
			}


			// checkPoint = start point
			// currentPoint = next point in path
			// while !currentPoint->next
			//		if can walk(checkPoint, currentPoint->next)
			//			// straight line
			//			temp = currentPoint // in path
			//			currentPoint = currentPoint->next;
			//			delete temp
			//		else
			//			checkPoint = currentPoint
			//			currentPoint = currentPoint->next

			void Smoothing(GameWorld* world) {
				std::reverse(waypoints.begin(), waypoints.end());
				Vector3 dir;
				Vector3 checkPoint = waypoints.at(0);
				int CPindex = 1;
				Vector3 currentPoint = waypoints.at(CPindex);
				Vector3 nextPoint = waypoints.at(CPindex + 1);

				while (CPindex < waypoints.size() - 1) {

					nextPoint = waypoints.at(CPindex + 1);

					dir = nextPoint - checkPoint;
					dir.Normalise();

					Ray r = Ray(checkPoint, dir);
					RayCollision closestCollision;
					bool collided = world->Raycast(r, closestCollision, true);

					float distance = sqrt(pow(checkPoint.x - nextPoint.x, 2) + pow(checkPoint.z - nextPoint.z, 2));

					if (!collided || closestCollision.rayDistance > distance) {
						currentPoint = nextPoint;
						waypoints.erase(waypoints.begin() + CPindex);
					}

					else {
						checkPoint = currentPoint;
						currentPoint = nextPoint;
						CPindex++;
					}
				}
				std::reverse(waypoints.begin(), waypoints.end());
			}

		protected:

			std::vector <Vector3> waypoints;
		};
	}
}

