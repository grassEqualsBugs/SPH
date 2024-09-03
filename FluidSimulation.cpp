#include "include/FluidSimulation.hpp"
#include "include/parallel.hpp"
#include "include/raylib.h"
#include "include/raymath.h"

void FluidSimulation::initParticlesRandomly() {
	Vector2 halfBoundsSize=Vector2SubtractValue(
		Vector2Scale(boundsSize, 0.5),
		particleSize);

	for (int i=0; i<numParticles; i++) {
		positions[i] = (Vector2){
			(float)GetRandomValue(-halfBoundsSize.x, halfBoundsSize.x),
			(float)GetRandomValue(-halfBoundsSize.y, halfBoundsSize.y)};
		velocities[i] = (Vector2){0, 0};
	}
}

void FluidSimulation::initParticlesInSquare() {
	int particlesPerRow=(int)sqrt(numParticles);
	int particlesPerCol=(numParticles - 1) / particlesPerRow + 1;
	float spacing = particleSize * 2 + particleSpacing;

	for (int i=0; i<numParticles; i++) {
		positions[i]=(Vector2){
			(i%particlesPerRow-particlesPerRow/2.f+0.5f)*spacing,
			(i/particlesPerRow-particlesPerCol/2.f+0.5f)*spacing
		};
		velocities[i]=(Vector2){0, 0};
	}
}

void FluidSimulation::Start() {
	positions.clear(); positions.resize(numParticles);
	velocities.clear(); velocities.resize(numParticles);
	densities.clear(); densities.resize(numParticles);
	predictedPositions.clear(); predictedPositions.resize(numParticles);
	spatialLookup.Resize(numParticles);
	mass=1.f;
	initParticlesInSquare();
	// initParticlesRandomly();
	spatialLookup.UpdateSpatialLookup(positions, smoothingRadius);
}

float FluidSimulation::densityToPressure(float density) {
	float densityError = density - targetDensity;
	return densityError * pressureMultiplier;
}

void FluidSimulation::resolveCollisions(Vector2& position, Vector2& velocity) {
	Vector2 halfBoundsSize=Vector2SubtractValue(
		Vector2Scale(boundsSize, 0.5),
		particleSize);
	if (abs(position.x)>halfBoundsSize.x) {
		position.x=halfBoundsSize.x*(2*(position.x>=0)-1);
		velocity.x *= -1 * collisionDamping;
	}
	if (abs(position.y)>halfBoundsSize.y) {
		position.y=halfBoundsSize.y*(2*(position.y>=0)-1);
		velocity.y *= -1 * collisionDamping;
	}
}

float FluidSimulation::smoothingKernel(float distance) {
	if (distance>=smoothingRadius) return 0;
	float volume=PI*pow(smoothingRadius,4)/6;
	return (smoothingRadius-distance)*(smoothingRadius-distance)/volume;
}

float FluidSimulation::smoothingKernelDerivative(float distance) {
	if (distance>=smoothingRadius) return 0;
	return (distance-smoothingRadius)*12/(smoothingRadius*smoothingRadius*smoothingRadius*smoothingRadius*PI);
}

float FluidSimulation::calculateDensity(Vector2 sampleParticle) {
	float density=0.f;

	std::vector<int> particlesWithinRadius=spatialLookup.GetPointsWithinRadius(sampleParticle);
	for (int i : particlesWithinRadius) {
		float distance=Vector2Distance(sampleParticle, positions[i]);
		float influence=smoothingKernel(distance);
		density+=influence*mass;
	}

	return density;
}

Vector2 getRandomDirection() {
	int degrees=GetRandomValue(0, 359);
	float radians=PI*degrees/180;
	Vector2 result=(Vector2){
		cos(radians),
		sin(radians)
	};
	return result;
}

Vector2 FluidSimulation::calculatePressureForce(int particleIdx) {
	Vector2 pressureForce=(Vector2){0, 0};
	std::vector<int> particlesWithinRadius=spatialLookup.GetPointsWithinRadius(predictedPositions[particleIdx]);
	for (int otherParticleIdx : particlesWithinRadius) {
		if (otherParticleIdx==particleIdx) continue;
		Vector2 difference=Vector2Subtract(predictedPositions[otherParticleIdx],predictedPositions[particleIdx]);
		float distance=Vector2Length(difference);
		Vector2 direction=distance==0?getRandomDirection():Vector2Scale(difference,1.f/distance);
		float influenceMagnitude=smoothingKernelDerivative(distance);
		float density=densities[otherParticleIdx];
		float pressure=densityToPressure(density);
		float otherPressure=densityToPressure(densities[particleIdx]);
		float sharedPressure=(pressure+otherPressure)/2;
		float scalar=sharedPressure*influenceMagnitude*mass/density;
		pressureForce=Vector2Add(pressureForce, Vector2Scale(direction,scalar));
	}
	return pressureForce;
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

Vector2 FluidSimulation::calculateMouseForce(int particleIdx, Vector2 mousePos, float strength) {
	Vector2 force=(Vector2){0,0};
	Vector2 offset=Vector2Subtract(mousePos, positions[particleIdx]);
	float distance=Vector2Length(offset);
	if (distance<mouseRadius) {
		Vector2 directionToMouse=distance<=std::numeric_limits<float>::epsilon()?(Vector2){0,0}:Vector2Scale(offset,1/distance);
		float distDependantStrength=1-distance/mouseRadius;
		force=Vector2Add(force,Vector2Scale(Vector2Subtract(Vector2Scale(directionToMouse,strength),velocities[particleIdx]),distDependantStrength));
	}
	return force;
}

int FluidSimulation::findClosestParticle() {
	int j=0;
	float bestDst=100000;
	for (int i=0; i<numParticles; i++) {
		Vector2 mousePosition=Vector2Subtract(
			GetMousePosition(),
			Vector2Scale(boundsSize, 0.5f)
		);
		mousePosition.y=-mousePosition.y;
		float distanceToParticle=Vector2Distance(mousePosition,positions[i]);
		if (distanceToParticle<bestDst) {
			j=i;
			bestDst=distanceToParticle;
		}
	}
	return j;
}

void FluidSimulation::SimulationStep(float deltaTime) {
	PARALLEL_FOR_BEGIN(numParticles) {
		velocities[i].y-=gravity*deltaTime;
		predictedPositions[i]=Vector2Add(positions[i],Vector2Scale(velocities[i],0.75f));
	}PARALLEL_FOR_END();

	spatialLookup.UpdateSpatialLookup(predictedPositions, smoothingRadius);

	PARALLEL_FOR_BEGIN(numParticles) {
		densities[i]=calculateDensity(predictedPositions[i]);
	}PARALLEL_FOR_END();

	Vector2 mousePosition=Vector2Subtract(
		GetMousePosition(),
		Vector2Scale(boundsSize, 0.5f)
	);
	mousePosition.y=-mousePosition.y;
	PARALLEL_FOR_BEGIN(numParticles) {
		Vector2 pressureForce=calculatePressureForce(i);
		Vector2 acceleration=Vector2Scale(pressureForce,1.f/densities[i]);
		velocities[i]=Vector2Add(velocities[i], Vector2Scale(calculateMouseForce(i,mousePosition,2*forceType),mouseFlag));
		velocities[i]=Vector2Add(velocities[i], Vector2Scale(acceleration,deltaTime));
	}PARALLEL_FOR_END();

	PARALLEL_FOR_BEGIN(numParticles) {
		positions[i] = Vector2Add(positions[i], velocities[i]);
		resolveCollisions(positions[i], velocities[i]);
	}PARALLEL_FOR_END();
}

void FluidSimulation::Render() {
	Vector3 c1=(Vector3){0,0,255};
	Vector3 c2=(Vector3){0,255,0};
	for (int i=0; i<numParticles; i++) {
		float speed=Vector2Length(velocities[i]);
		speed/=8;
		Vector3 c=Vector3Lerp(c1, c2, speed);
		DrawCircleV(positions[i], particleSize, (Color){
			(unsigned char)((int)c.x),
			(unsigned char)((int)c.y),
			(unsigned char)((int)c.z),
			255
		});
	}
}
