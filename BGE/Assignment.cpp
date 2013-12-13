#include "Assignment.h"
#include "PhysicsController.h"
#include "Sphere.h"
#include "PhysicsCamera.h"
#include "Box.h"
#include "Cylinder.h"
#include "Steerable3DController.h"
#include "Ground.h"
#include "Content.h"
#include <btBulletDynamicsCommon.h>
#include <gtc/quaternion.hpp>
#include <gtx/quaternion.hpp>
#include <gtx/euler_angles.hpp>
#include <gtx/norm.hpp>
#include "VectorDrawer.h"
#include "Utils.h"

using namespace BGE;

Assignment::Assignment(void)
{
	physicsFactory = NULL;
	dynamicsWorld = NULL;
	broadphase = NULL;
	dispatcher = NULL;
	solver = NULL;
	fullscreen = false;

}

Assignment::~Assignment(void)
{
}

shared_ptr<PhysicsController> assignmentController;
std::shared_ptr<GameComponent> assignmentStation;

bool Assignment::Initialise() 
{
	riftEnabled = false;
	// Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
 
    // The world.
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	broadphase = new btAxisSweep3(worldMin,worldMax);
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,0,0));
	width=800;
	height = 600;

	

	physicsFactory = make_shared<PhysicsFactory>(dynamicsWorld);
	
	physicsFactory->CreateGroundPhysics();
	physicsFactory->CreateCameraPhysics();

	Assignment::Layout();
	
	physicsFactory->RigidBody(5,5,630);

	fountainTheta = 0.0f;
	for (int i = 0 ; i < FOUNTAINS  ; i ++)
	{
		fountainTheta = ((glm::pi<float>() * 2.0f) / FOUNTAINS ) * i;
		shared_ptr<FountainEffect> fountain = make_shared<FountainEffect>(500);
		if (i % 2 == 0)
		{
			fountain->diffuse = glm::vec3(1,0,0);
		}
		else
		{
			fountain->diffuse = glm::vec3(0,1,0);
		}

		fountain->position.x = glm::sin(fountainTheta) * RADIUS;
		fountain->position.z = - glm::cos(fountainTheta) * RADIUS;
		
		fountains.push_back(fountain);
		Attach(fountain);
	}
	fountainTheta = 0.0f;
	
	ships = physicsFactory->CreateFromModel("cobramk3", glm::vec3(5, 2, 10),glm::quat(), glm::vec3(1));
	ships->rigidBody->setActivationState(DISABLE_DEACTIVATION);
	ships->parent;
	Attach(ships);

	ship2 = physicsFactory->CreateFromModel("cobramk3", glm::vec3(200, 10, -400),glm::quat(), glm::vec3(1));
	ship2->rigidBody->setActivationState(DISABLE_DEACTIVATION);
	ship2->parent;
	Attach(ship2);

	Assignment::knockOver();
	
	hud = false;
	
	if (!Game::Initialise()) {
		return false;
	}

	return true;
	
	camera->GetController()->position = glm::vec3(0, 4, 20);
}


void BGE::Assignment::Update(float timeDelta)
{
	ships->Yaw(0);
	ships->Pitch(0);
	for (int i = 0 ; i < fountains.size() ; i ++)
	{
		if (i % 2 == 0)
		{
			fountains[i]->position.x = FOUNTAIN_POSITION + (glm::sin(fountainTheta) * FOUNTAIN_POSITION);
		}
		else
		{
			fountains[i]->position.x = FOUNTAIN_POSITION - (glm::sin(fountainTheta) * FOUNTAIN_POSITION);
		}
	}

	fountainTheta += timeDelta;
	if (fountainTheta >= glm::pi<float>() * 6.0f)
	{
		fountainTheta = 0.0f;
	}

	if(keyState[SDL_SCANCODE_1]){
		camera->GetController()->position = glm::vec3(ships->position.x,ships->position.y+6, ships->position.z-5);
	}else if(keyState[SDL_SCANCODE_2]){
		camera->GetController()->position = glm::vec3(20,20,20);
	}
	else if(keyState[SDL_SCANCODE_3]){
		camera->GetController()->position = glm::vec3(200,20,200);
	}
	else if(keyState[SDL_SCANCODE_4]){
		camera->GetController()->position = glm::vec3(ship2->position.x,ship2->position.y+2,ship2->position.z-6);
	}


	
	if (keyState[SDL_SCANCODE_UP])
	{
		ships->rigidBody->applyCentralForce(GLToBtVector(-ships->look * speed));
	}
	if (keyState[SDL_SCANCODE_DOWN])
	{
		ships->rigidBody->applyCentralForce(GLToBtVector(ships->look * speed));
	}
	if (keyState[SDL_SCANCODE_LEFT])
	{
		ships->rigidBody->applyTorque(GLToBtVector(ships->up* speed));
	}
	if (keyState[SDL_SCANCODE_RIGHT])
	{
		ships->rigidBody->applyTorque(GLToBtVector(-ships->up * speed));
	}
	if((!keyState[SDL_SCANCODE_UP])&&(!keyState[SDL_SCANCODE_DOWN])){
		ships->rigidBody->applyCentralForce(GLToBtVector(ships->look * 0.0f));
	}

	if (keyState[SDL_SCANCODE_T])
	{
		ship2->rigidBody->applyCentralForce(GLToBtVector(-ship2->look * speed));
	}
	if (keyState[SDL_SCANCODE_G])
	{
		ship2->rigidBody->applyCentralForce(GLToBtVector(ship2->look * speed));
	}
	if (keyState[SDL_SCANCODE_F])
	{
		ship2->rigidBody->applyTorque(GLToBtVector(ship2->up* speed));
	}
	if (keyState[SDL_SCANCODE_H])
	{
		ship2->rigidBody->applyTorque(GLToBtVector(-ship2->up * speed));
	}




	if (keyState[SDL_SCANCODE_RSHIFT])
	{

		speed = 30.0f;
		
	}else if(keyState[SDL_SCANCODE_LSHIFT]){
		
		speed = 60.0f;
	}
	else
	{
		speed=10.0f;
	}
	
	if(ships->position.z>=605){
		PrintText("YOU WIN");
	}

	if(ships->position.y<8.0f){
		ships->position = glm::vec3(5, 2, 10);
	}else if(ships->position.y>4.0f){
		ships->position = glm::vec3(5, 2, 10);
	}

	dynamicsWorld->stepSimulation(timeDelta,100);
	Game::Update(timeDelta);
}

void BGE::Assignment::knockOver(){
	
	physicsFactory->RigidBody(200,3,190);
	physicsFactory->CreateWall(glm::vec3(186,1,230), 5, 5,5,5,5);
	physicsFactory->CreateWall(glm::vec3(190,1,225), 4, 5,5,5,5);
	physicsFactory->CreateWall(glm::vec3(194,1,220), 3, 5,5,5,5);
	physicsFactory->CreateWall(glm::vec3(197,1,215), 2, 5,5,5,5);
	physicsFactory->CreateWall(glm::vec3(200,1,210), 1, 5,5,5,5);
}


void BGE::Assignment::Layout(){
	//right walls
	physicsFactory->CreateKinenaticWall(glm::vec3(7.5f,0,0), 1, 1,16,10,1);
	
	physicsFactory->CreateKinenaticWall(glm::vec3(0,10,200), 1, 1,1,10,400);
	physicsFactory->CreateKinenaticWall(glm::vec3(-9.5,10,400), 1, 1,20,10,1);
	physicsFactory->CreateKinenaticWall(glm::vec3(-20,10,507), 1, 1,1,10,212);
	
	//left walls
	physicsFactory->CreateKinenaticWall(glm::vec3(15,10,200), 1, 1,1,10,400);
	physicsFactory->CreateKinenaticWall(glm::vec3(24.5,10,400), 1, 1,20,10,1);
	physicsFactory->CreateKinenaticWall(glm::vec3(36,10,507), 1, 1,1,10,212);

	//center walls
	physicsFactory->CreateKinenaticWall(glm::vec3(8,10,420), 1, 1,16,10,1);
	physicsFactory->CreateKinenaticWall(glm::vec3(9,10,601), 1, 1,16,10,1);
	
	//centerleft
	physicsFactory->CreateKinenaticWall(glm::vec3(18,10,510), 1, 1,1,10,180);

	//center right
	
	physicsFactory->CreateKinenaticWall(glm::vec3(0,10,510), 1, 1,1,10,180);

	//top wall
	physicsFactory->CreateKinenaticWall(glm::vec3(-9.5,10,614), 1, 1,20,10,1);
	physicsFactory->CreateKinenaticWall(glm::vec3(24.5,10,614), 1, 1,20,10,1);

	
}

void BGE::Assignment::RagDoll(){
	
}

void BGE::Assignment::Cleanup()
{
	Game::Cleanup();
}
