#pragma once
#include "..\CSC8503Common\GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class MotorPlane : public GameObject
		{
		public:
			MotorPlane() {
				isRolling = false;
			};
			~MotorPlane() {};

		protected:
			bool isRolling = false;
		};
	}
}