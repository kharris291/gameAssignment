#pragma once
#include "Game.h"
#include "PhysicsController.h"
#include "PhysicsFactory.h"
#include "FountainEffect.h"
#include <btBulletDynamicsCommon.h>

#define RADIUS 900.0f
#define HEIGHT 10.0f
#define FOUNTAIN_POSITION 50.0f

#define FOUNTAINS 50

using namespace std;

namespace BGE
{
	class Assignment :
		public Game
	{
	private:
		btBroadphaseInterface* broadphase;
 
		// Set up the collision configuration and dispatcher
		btDefaultCollisionConfiguration * collisionConfiguration;
		btCollisionDispatcher * dispatcher;
 
		// The actual physics solver
		btSequentialImpulseConstraintSolver * solver;

	public:
		Assignment(void);
		~Assignment(void);
		bool Initialise();
		void Update(float timeDelta);
		void Cleanup();
		void CreateWall();
		void RagDoll();
		void Layout();
		shared_ptr<FountainEffect> fountain1;
		shared_ptr<FountainEffect> fountain2;
		
		shared_ptr<PhysicsController> ships,ship2;
		
		
		vector<shared_ptr<FountainEffect>> fountains;
		float fountainTheta;
		

		// The world.
		std::shared_ptr<PhysicsFactory> physicsFactory;
		
		btDiscreteDynamicsWorld * dynamicsWorld;


	};
}

