#pragma once
#include "../../Common/Matrix4.h"
#include "../../Common/Matrix3.h"
#include "../../Common/Vector3.h"
#include "../../Common/Quaternion.h"

#include <vector>

using std::vector;

using namespace NCL::Maths;

namespace NCL {
	namespace CSC8503 {
		class Transform
		{
		public:
			Transform();
			~Transform();

			Transform& SetPosition(const Vector3& worldPos);
			Transform& SetScale(const Vector3& worldScale);
			Transform& SetOrientation(const Quaternion& newOr);

			Vector3 GetPosition() const {
				return position;
			}

			Vector3 GetScale() const {
				return scale;
			}

			Vector3 GetUp() const
			{
				return Vector3(2 * (orientation.x * orientation.y - orientation.w * orientation.z), 1 - 2 * (orientation.x * orientation.x + orientation.z * orientation.z), 2 * (orientation.y * orientation.z + orientation.w * orientation.x));
			}

			Vector3 GetForward() const
			{
				return Vector3(2 * (orientation.x * orientation.z + orientation.w * orientation.y), 2 * (orientation.y * orientation.z - orientation.w * orientation.x), 1 - 2 * (orientation.x * orientation.x + orientation.y * orientation.y));
			}

			Vector3 GetLeft() const
			{
				return Vector3::Cross(GetForward(), -GetUp());
			}

			Quaternion GetOrientation() const {
				return orientation;
			}

			Matrix4 GetMatrix() const {
				return matrix;
			}
			void UpdateMatrix();
		protected:
			Matrix4		matrix;
			Quaternion	orientation;
			Vector3		position;

			Vector3		scale;
		};
	}
}

