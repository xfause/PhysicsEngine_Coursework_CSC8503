#pragma once
#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"
#include "../../Common/Window.h"

#include "TutorialGame.h"

namespace NCL {
	namespace CSC8503 {
		class MenuState :public PushdownState {
			public:
				MenuState(TutorialGame* a)
				{
					g = a;
				}
				PushdownResult OnUpdate(float dt, PushdownState** newState) override;

				void OnAwake() override;
			protected:
				TutorialGame* g;
		};

		class LevelOneState : public PushdownState {
			public:
				LevelOneState(TutorialGame* a)
				{
					g = a;
				}

				PushdownResult OnUpdate(float dt, PushdownState** newState) override;
				void OnAwake() override;

			protected:
				TutorialGame* g;
		};

		class LevelOneNoTimeModeState : public PushdownState {
		public:
			LevelOneNoTimeModeState(TutorialGame* a)
			{
				g = a;
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void OnAwake() override;

		protected:
			TutorialGame* g;
		};


		class LevelTwoState : public PushdownState
		{
			public:
				LevelTwoState(TutorialGame* a)
				{
					g = a;
				}

				PushdownResult OnUpdate(float dt, PushdownState** newState) override;

				void OnAwake() override;

			protected:
				TutorialGame* g;
		};

		class TestState : public PushdownState
		{
			public:
				TestState(TutorialGame* a)
				{
					g = a;
				}
				PushdownResult OnUpdate(float dt, PushdownState** newState) override;

				void OnAwake() override;

			protected:
				TutorialGame* g;
		};
	}
}