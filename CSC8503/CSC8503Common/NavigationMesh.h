#pragma once
#include "NavigationMap.h"
#include "../../Common/Plane.h"
#include <string>
#include <vector>
namespace NCL {
	namespace CSC8503 {

		class NavigationMesh : public NavigationMap {
		public:
			NavigationMesh();
			NavigationMesh(const std::string& filename);
			~NavigationMesh();

			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
			bool FindStringPath(const Vector3& from, const Vector3& to, NavigationPath& stringPath);
			void DrawMesh();
			void DrawTriPath(int p);
			void DrawPortals();

		protected:
			struct NavTri {
				Plane   triPlane;
				Vector3 centroid;
				float	area;
				NavTri* neighbours[3];

				int indices[3];

				NavTri() {
					area = 0.0f;
					neighbours[0] = nullptr;
					neighbours[1] = nullptr;
					neighbours[2] = nullptr;

					indices[0] = -1;
					indices[1] = -1;
					indices[2] = -1;
				}
			};

			struct Node {
				Node* parent;
				float f;
				float g;
				const NavTri* tri;

				Node() {
					f = 0;
					g = 0;
					parent = nullptr;
					tri = nullptr;
				}

				Node(const NavTri* t) {
					f = 0;
					g = 0;
					parent = nullptr;
					tri = t;
				}

				~Node() {	}
			};

			const NavTri* GetTriForPosition(const Vector3& pos) const;

			std::vector<NavTri>		allTris;
			std::vector<Vector3>	allVerts;

			std::vector<Vector3> stringVerts;

			std::vector<const NavTri*> TriPath;

			float Heuristic(Node* hNode, Node* endNode) const;
			bool NodeInList(const Node* n, std::vector<Node*>& list) const;
			Node* RemoveBestNode(std::vector<Node*>& list) const;
			void CalculatePortals();
			void StringPull(NavigationPath& outPath, const Vector3& from, const Vector3& to);
			vector<Vector3> leftVertices;
			vector<Vector3> rightVertices;
			vector<Vector3> apexRef;

			bool IsLeft(Vector3 A, Vector3 B)
			{
				Vector3 c = Vector3::Cross(A.Normalised(), B.Normalised());
				return(c.y <= 0.0f);
			}
		};
	}
}