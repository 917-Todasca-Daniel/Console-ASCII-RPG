#ifndef PARTICLES_H
#define PARTICLES_H

#include "ScreenObject.h"
#include "GarbageTruck.h"

#include "stdc++.h"

class Particle : public ScreenObject {
public:
	Particle(int priority, float energy, float energyFallOff, std::pair <float, float> pos, std::pair <float, float> velocity, std::pair <float, float> accel);

	virtual void loop(float delta) override;

	float getEnergy() { return energy; }

	virtual void update(float energy, float energyFallOff, std::pair <float, float> pos, std::pair <float, float> velocity, std::pair <float, float> accel);

protected:
	virtual ~Particle() {};

	float energy, energyFallOff;
	std::pair <float, float> pos;
	std::pair <float, float> velocity;
	std::pair <float, float> accel;
	std::pair <float, float> old_position;

	virtual void updatePhysics(float delta);
	virtual void updateShape(float delta);

	virtual void particleDestroy();
};

class WaterParticle : public Particle {
public:
	WaterParticle(int priority, float energy, float energyFallOff, std::pair <float, float> pos, std::pair <float, float> velocity, std::pair <float, float> accel);

protected:
	virtual void updateShape(float delta) override;
};

class KineticSequenceParticle : public Particle {
public:
	KineticSequenceParticle(std::vector <std::pair<float, std::string>> kineticMap, int priority, float energy, float energyFallOff, std::pair <float, float> pos, std::pair <float, float> velocity, std::pair <float, float> accel);

protected:
	std::vector <std::pair<float, std::string>> kineticMap;
	virtual void updateShape(float delta) override;
};

class FlashParticle : public Particle {
public:
	FlashParticle(int priority, float energy, float energyFallOff, std::pair <float, float> pos, std::pair <float, float> velocity, std::pair <float, float> accel);

protected:
	virtual void updateShape(float delta) override;
};


class PrimitiveParticleFactory : public GarbageTruck {
public:
	virtual Particle* spawn() = 0;
};
template <typename ParticleType, int priority>
class ParticleFactory : public PrimitiveParticleFactory {
public:
	virtual Particle* spawn() override;
};
template < int priority>
class KineticFactory : public PrimitiveParticleFactory {
public:
	KineticFactory(std::vector <std::pair <float, std::string>> kineticMap);
	virtual Particle* spawn() override;
private:
	std::vector <std::pair <float, std::string>> kineticMap;
};


enum EMITTER_PROPERTIES { ENERGY, ENERGY_FALL_OFF, OFFSET_X, OFFSET_Y, VELOCITY_X, VELOCITY_Y, ACCEL_X, ACCEL_Y, EMITTER_PROPERTIES_COUNT };


class ParticleEmitter : public PrimitiveObject {
public:
	ParticleEmitter(float particlesPerSecond, std::pair <float, float> pos, std::pair <float, float> props[EMITTER_PROPERTIES_COUNT], PrimitiveParticleFactory *factory);

	virtual void destroy() override;
	virtual void loop(float delta) override;

private:
	virtual ~ParticleEmitter() { delete factory; };

	std::pair <float, float> props[EMITTER_PROPERTIES_COUNT];
	std::pair <float, float> pos;
	
	float particlesPerSecond;
	float particlesAccumulator;

	virtual void spawnParticle();
	virtual Particle* allocateParticle();

	float generate(EMITTER_PROPERTIES property);

	int aliveCount;
	std::vector <Particle*> particlePool;
	PrimitiveParticleFactory  *factory;
};

#endif

template<typename ParticleType, int priority>
inline Particle* ParticleFactory<ParticleType, priority>::spawn() {
	return new ParticleType(priority, -1.0f, 0.0f, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f });
}

template<int priority>
inline KineticFactory<priority>::KineticFactory(std::vector<std::pair<float, std::string>> kineticMap) : kineticMap(kineticMap)
{}

template<int priority>
inline Particle* KineticFactory<priority>::spawn()
{
	return new KineticSequenceParticle(kineticMap, priority, -1.0f, 0.0f, { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f });
}
