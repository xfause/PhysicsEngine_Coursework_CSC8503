#pragma once

namespace NCL {
	namespace CSC8503 {

		class Spring {
		public:
			Spring(float l, float k, float d = 0.8f){
				length = l;
				springConstant = k;
				damping = d;
			}

			float GetLength() const { return length; }
			float GetSpringConstant() const { return springConstant; }
			float GetDamping() const { return damping; }

			void SetDamping(float v) { damping = v; }
		protected:
			float length;
			float springConstant;
			float damping;
		};
	}
}
