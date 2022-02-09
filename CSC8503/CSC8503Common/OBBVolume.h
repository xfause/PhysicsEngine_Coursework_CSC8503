#pragma once
#include "CollisionVolume.h"
#include "../../Common/Vector3.h"
namespace NCL {
	class OBBVolume : CollisionVolume
	{
	public:
		OBBVolume(const Maths::Vector3& halfDims) {
			type		= VolumeType::OBB;
			halfSizes	= halfDims;
		}
		~OBBVolume() {}

		Maths::Vector3 GetHalfDimensions() const {
			return halfSizes;
		}

		Vector3 OBBSupport(const CSC8503::Transform& worldTransform, Vector3 worldDir) const
		{
			Vector3 localDir = worldTransform.GetOrientation().Conjugate() * worldDir;
			Vector3 Vertex;
			Vertex.x = localDir.x < 0 ? -0.5f : 0.5f;
			Vertex.y = localDir.y < 0 ? -0.5f : 0.5f;
			Vertex.z = localDir.z < 0 ? -0.5f : 0.5f;
			return worldTransform.GetMatrix() * Vertex;
		}
	protected:
		Maths::Vector3 halfSizes;
	};
}

