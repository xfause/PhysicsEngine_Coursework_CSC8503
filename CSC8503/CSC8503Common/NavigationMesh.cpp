#include "NavigationMesh.h"
#include "Debug.h"
#include "../../Common/Assets.h"
#include "../../Common/Maths.h"
#include <fstream>
using namespace NCL;
using namespace CSC8503;
using namespace std;

NavigationMesh::NavigationMesh()
{
}

NavigationMesh::NavigationMesh(const std::string& filename)
{
	ifstream file(Assets::DATADIR + filename);

	int numVertices = 0;
	int numIndices = 0;

	file >> numVertices;
	file >> numIndices;

	for (int i = 0; i < numVertices; ++i) {
		Vector3 vert;
		file >> vert.x;
		file >> vert.y;
		file >> vert.z;

		allVerts.emplace_back(vert);
	}

	allTris.resize(numIndices / 3);

	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		file >> tri->indices[0];
		file >> tri->indices[1];
		file >> tri->indices[2];

		tri->centroid = allVerts[tri->indices[0]] +
			allVerts[tri->indices[1]] +
			allVerts[tri->indices[2]];

		tri->centroid = allTris[i].centroid / 3.0f;

		tri->triPlane = Plane::PlaneFromTri(allVerts[tri->indices[0]], allVerts[tri->indices[1]], allVerts[tri->indices[2]]);

		tri->area = Maths::CrossAreaOfTri(allVerts[tri->indices[0]], allVerts[tri->indices[1]], allVerts[tri->indices[2]]);
	}
	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		for (int j = 0; j < 3; ++j) {
			int index = 0;
			file >> index;
			if (index != -1) {
				tri->neighbours[j] = &allTris[index];
			}
		}
	}
}

NavigationMesh::~NavigationMesh()
{
}

bool NavigationMesh::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	const NavTri* start = GetTriForPosition(from);
	const NavTri* end = GetTriForPosition(to);

	if (start == nullptr || end == nullptr) { //check both locations are in a tri
		return false;
	}

	std::vector<Node*>  openList;
	std::vector<Node*>  closedList;

	Node* sNode = new Node();
	Node* eNode = new Node();
	sNode->tri = start;
	eNode->tri = end;

	openList.emplace_back(sNode);
	Node* currentBestNode = nullptr;

	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);

		if (currentBestNode->tri == eNode->tri) {			//we've found the path!
			Node* node = currentBestNode;
			outPath.PushWaypoint(to); //end position
			TriPath.push_back(eNode->tri);

			while (node != nullptr) {
				outPath.PushWaypoint(node->tri->centroid);
				TriPath.push_back(node->tri);
				node = node->parent;
			}

			outPath.PushWaypoint(from);

			CalculatePortals();

			for (Node* n : openList) { delete n; }
			for (Node* n : closedList) { delete n; }

			return true;
		}

		else {
			for (int i = 0; i < 3; ++i) {

				Node* neighbour = new Node(currentBestNode->tri->neighbours[i]);
				if (neighbour->tri == nullptr) { //might not be connected...
					delete neighbour;
					continue;
				}
				bool inClosed = NodeInList(neighbour, closedList);
				if (inClosed) {
					delete neighbour;
					continue; //already discarded this neighbour...
				}

				float h = Heuristic(neighbour, eNode);
				float g = currentBestNode->g + (neighbour->tri->centroid - currentBestNode->tri->centroid).Length();
				float f = h + g;

				bool inOpen = NodeInList(neighbour, openList);

				if (!inOpen) { //first time we've seen this neighbour
					openList.emplace_back(neighbour);
				}

				if (!inOpen || f < neighbour->f) {//might be a better route to this neighbour
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}
			closedList.emplace_back(currentBestNode);
		}
	}
	// if no path open list is empty
	return false;
}

bool NavigationMesh::FindStringPath(const Vector3& from, const Vector3& to, NavigationPath& stringPath) {
	if (stringVerts.size() == 0) {
		return true;
	}
	StringPull(stringPath, from, to); //sting pull path
	stringPath.PushWaypoint(to);
	stringPath.Reverse();
	return true;
}

void NavigationMesh::DrawMesh() {
	for (NavTri t : allTris) {
		Debug::DrawLine(allVerts[t.indices[0]], allVerts[t.indices[1]], Vector4(1, 1, 1, 1));
		Debug::DrawLine(allVerts[t.indices[0]], allVerts[t.indices[2]], Vector4(1, 1, 1, 1));
		Debug::DrawLine(allVerts[t.indices[1]], allVerts[t.indices[2]], Vector4(1, 1, 1, 1));

	}
	//Debug::DrawLine(Vector3(49,0,38), Vector3(49, 80, 38), Vector4(1, 0, 1, 1));
}

void NavigationMesh::DrawTriPath(int p) {
	for (const NavTri* t : TriPath) {
		Debug::DrawLine(allVerts[t->indices[0]], allVerts[t->indices[1]], Vector4(0, 0, 1, 1));
		Debug::DrawLine(allVerts[t->indices[0]], allVerts[t->indices[2]], Vector4(0, 0, 1, 1));
		Debug::DrawLine(allVerts[t->indices[1]], allVerts[t->indices[2]], Vector4(0, 0, 1, 1));

	}

	if (p >= 0 && p < leftVertices.size()) {
		Debug::DrawLine(leftVertices[p], leftVertices[p] + Vector3(0, 40, 0), Vector4(1, 0, 0, 1));
		Debug::DrawLine(rightVertices[p], rightVertices[p] + Vector3(0, 40, 0), Vector4(0, 1, 0, 1));

		Debug::DrawLine(apexRef[p] + Vector3(0, 2, 0), leftVertices[p] + Vector3(0, 2, 0), Vector4(1, 0.64, 0, 1));
		Debug::DrawLine(apexRef[p] + Vector3(0, 2, 0), rightVertices[p] + Vector3(0, 2, 0), Vector4(1, 0.64, 0, 1));

		//Debug::Print("left: " + std::to_string(leftVertices[p].x) + ", " + std::to_string(leftVertices[p].z), Vector2(5, 5));
		//Debug::Print("right: " + std::to_string(rightVertices[p].x) + ", " + std::to_string(rightVertices[p].z), Vector2(5, 15));
	}
}

void NavigationMesh::CalculatePortals() {

	for (int i = TriPath.size() - 1; i > 1; i--) {
		const NavTri* current = TriPath.at(i);
		const NavTri* next = TriPath.at(i - 1);
		int points[] = { 0,1,2 };
		for (int j = 0; j < 3; j++) { //checking which points the shared point in the next portal to make it the first point in the next pair
			const int* found = std::find(std::begin(next->indices), std::end(next->indices), current->indices[j]);
			if (found == std::end(next->indices)) {
				int* pbegin = points;
				int* pend = points + sizeof(points) / sizeof(int);
				pend = std::remove(pbegin, pend, j);
				break;
			}
		}

		if (i == TriPath.size() - 1) {
			stringVerts.push_back(allVerts[current->indices[points[0]]]);
			stringVerts.push_back(allVerts[current->indices[points[1]]]);
		}

		else {
			Vector3 first = stringVerts[stringVerts.size() - 1];
			Vector3 second = stringVerts[stringVerts.size() - 2];

			if (allVerts[current->indices[points[0]]] == first || allVerts[current->indices[points[0]]] == second) { //first point shared
				stringVerts.push_back(allVerts[current->indices[points[0]]]);
				stringVerts.push_back(allVerts[current->indices[points[1]]]);
			}

			else { //second point shared
				stringVerts.push_back(allVerts[current->indices[points[1]]]);
				stringVerts.push_back(allVerts[current->indices[points[0]]]);
			}
		}
	}
}

void NavigationMesh::DrawPortals() {
	for (int i = 0; i < stringVerts.size() - 2; i++) {
		Debug::DrawLine(stringVerts[i], stringVerts[i + 1], Vector4(0.1, 0, 0.3, 1));
	}
}

void NavigationMesh::StringPull(NavigationPath& outPath, const Vector3& from, const Vector3& to) {

	outPath.PushWaypoint(from);

	Vector3 apex = from;
	Vector3 left = stringVerts[0];
	Vector3 right = stringVerts[1];

	for (int i = 2; i < stringVerts.size() - 2; i = i + 2) {

		if (stringVerts[i] == right) { //right repeating, move left
			Vector3 tempLeft = stringVerts[i + 1];
			Vector3 newSide = tempLeft - apex;
			Vector3 oldSide = left - apex;
			// not wider
			if (!IsLeft(newSide, oldSide)) {

				Vector3 rightSide = right - apex;
				// cross
				if (!IsLeft(newSide, rightSide)) {
					apex = right;
					Vector3 direction = (apex - left).Normalised(); //portal direction
					Vector3 pathPoint = apex - direction * 2; //give waypoint offset
					outPath.PushWaypoint(pathPoint);
					i += 2; //set up at next portal
					left = tempLeft;
					right = (left == stringVerts[i]) ? stringVerts[i + 1] : stringVerts[i];
				}

				else left = tempLeft;
			}

			else {
				apex = left;

				Vector3 direction = (apex - right).Normalised();
				Vector3 pathPoint = apex - direction * 2;
				outPath.PushWaypoint(pathPoint);

				left = tempLeft;
			}
		}

		else { //left repeating, move right
			Vector3 tempRight = stringVerts[i + 1];
			Vector3 newSide = tempRight - apex;
			Vector3 oldSide = right - apex;
			// not widen
			if (IsLeft(newSide, oldSide)) {

				Vector3 leftSide = left - apex;
				// cross
				if (IsLeft(newSide, leftSide)) {
					apex = left;
					Vector3 direction = (apex - right).Normalised();
					Vector3 pathPoint = apex - direction * 2;
					outPath.PushWaypoint(pathPoint);
					i += 2;
					right = tempRight;
					left = (right == stringVerts[i]) ? stringVerts[i + 1] : stringVerts[i];

				}
				else right = tempRight;
			}

			else {
				apex = right;
				Vector3 direction = (apex - left).Normalised();
				Vector3 pathPoint = apex - direction * 2;
				outPath.PushWaypoint(pathPoint);
				right = tempRight;
			}
		}
	}
}


/*
If you have triangles on top of triangles in a full 3D environment, you'll need to change this slightly,
as it is currently ignoring height. You might find tri/plane raycasting is handy.
*/

const NavigationMesh::NavTri* NavigationMesh::GetTriForPosition(const Vector3& pos) const {
	for (const NavTri& t : allTris) {
		Vector3 planePoint = t.triPlane.ProjectPointOntoPlane(pos);

		float ta = Maths::CrossAreaOfTri(allVerts[t.indices[0]], allVerts[t.indices[1]], planePoint);
		float tb = Maths::CrossAreaOfTri(allVerts[t.indices[1]], allVerts[t.indices[2]], planePoint);
		float tc = Maths::CrossAreaOfTri(allVerts[t.indices[2]], allVerts[t.indices[0]], planePoint);

		float areaSum = ta + tb + tc;

		if (abs(areaSum - t.area) > 0.001f) {
			continue;
		}
		return &t;
	}
	return nullptr;
}

float NavigationMesh::Heuristic(Node* hNode, Node* endNode) const {
	return (hNode->tri->centroid - endNode->tri->centroid).Length();
}

bool NavigationMesh::NodeInList(const Node* n, std::vector<Node*>& list) const {
	for (Node* i : list) {
		if (i->tri == n->tri) {
			return true;
		}
	}

	return false;

}

NavigationMesh::Node* NavigationMesh::RemoveBestNode(std::vector<Node*>& list) const {
	std::vector<Node*>::iterator bestI = list.begin();

	Node* bestNode = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode = (*i);
			bestI = i;
		}
	}
	list.erase(bestI);

	return bestNode;
}
