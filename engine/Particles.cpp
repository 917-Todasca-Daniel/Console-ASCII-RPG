#include "Particles.h"

#include "Debugger.h"

Particle::Particle(int priority, float energy, float energyFallOff, std::pair <float, float> pos, std::pair <float, float> velocity, std::pair <float, float> accel)
	: energy(energy), energyFallOff(energyFallOff), pos(pos), velocity(velocity), accel(accel), ScreenObject(pos.first, pos.second, priority) {
	hide();
}

void Particle::loop(float delta) {
	if (energy < 0) return;
	ScreenObject::loop(delta);
	updatePhysics(delta);

	if (energy < 0) {
		particleDestroy();
		return;
	}

	updateShape(delta);
}

void Particle::update(float energy, float energyFallOff, std::pair<float, float> pos, std::pair<float, float> velocity, std::pair<float, float> accel) {
	this->energy = energy;
	this->energyFallOff = energyFallOff;
	this->pos = pos;
	this->velocity = velocity;
	this->accel = accel;
}

void Particle::particleDestroy() {
	hide();
}

void Particle::updatePhysics(float delta) {
	velocity.first += accel.first * delta;
	velocity.second += accel.second * delta;

	old_position = pos;
	pos.first += velocity.first * delta;
	pos.second += velocity.second * delta;

	energy -= energyFallOff * delta;

	x = (int) pos.first;
	y = (int) pos.second;
}

void Particle::updateShape(float delta) {
	std::string shape = "*";
	ScreenObject::updateShape(shape);
}


ParticleEmitter::ParticleEmitter(float particlesPerSecond, std::pair <float, float> pos, std::pair<float, float> props[EMITTER_PROPERTIES_COUNT], PrimitiveParticleFactory* factory) 
	: pos(pos), particlesPerSecond(particlesPerSecond) {
	this->factory = factory;
	for (int i = 0; i < EMITTER_PROPERTIES_COUNT; ++i)
		this->props[i] = props[i];
	aliveCount = 0;
	particlesAccumulator = 0;
}

void ParticleEmitter::destroy() {
	for (auto& it : particlePool)
		if (it) it->destroy();
	PrimitiveObject::destroy();
}

void ParticleEmitter::loop(float delta) {
	if (bDestroyed) return;
	PrimitiveObject::loop(delta);
	particlesAccumulator += particlesPerSecond * delta;

	for (int i = 0; i < aliveCount; ++i) {
		while (particlePool[i]->getEnergy() < 0 && aliveCount > 0) {
			std::swap(particlePool[i], particlePool[aliveCount - 1]);
			--aliveCount;
		}
	}

	for (int i = 0; i < (int)particlesAccumulator; ++i) {
		spawnParticle();
	}
	particlesAccumulator -= (int) particlesAccumulator;
}

Particle* ParticleEmitter::allocateParticle() {
	return factory->spawn();
}

float ParticleEmitter::generate(EMITTER_PROPERTIES property) {
	return randBetween(props[property].first, props[property].second);
}

void ParticleEmitter::spawnParticle() {
	if (aliveCount == particlePool.size()) {
		particlePool.push_back(allocateParticle());
	}

	particlePool[aliveCount]->update(generate(ENERGY), generate(ENERGY_FALL_OFF), { pos.first + generate(OFFSET_X), pos.second + generate(OFFSET_Y) },
		{ generate(VELOCITY_X), generate(VELOCITY_Y) }, { generate(ACCEL_X), generate(ACCEL_Y) });
	++ aliveCount;
}

WaterParticle::WaterParticle(int priority, float energy, float energyFallOff, std::pair<float, float> pos, std::pair<float, float> velocity, std::pair<float, float> accel)
	: Particle(priority, energy, energyFallOff, pos, velocity, accel)
{}

void WaterParticle::updateShape(float delta) {
	if (velocity.first == 0) {
		ScreenObject::updateShape("_");
	}
	if (velocity.second < 0) {
		if (energy < 0.8f)
			ScreenObject::updateShape(";");
		else 
			ScreenObject::updateShape("|");
	}
	else {
		if (energy < 0.5f)
			ScreenObject::updateShape("'");
		else if (velocity.first > 0)
			ScreenObject::updateShape("\\");
		else
			ScreenObject::updateShape("/");
	}
}

KineticSequenceParticle::KineticSequenceParticle(std::vector <std::pair<float, std::string>> kineticMap, int priority, float energy, float energyFallOff, std::pair<float, float> pos, std::pair<float, float> velocity, std::pair<float, float> accel)
	: kineticMap(kineticMap), Particle(priority, energy, energyFallOff, pos, velocity, accel)
{}

void KineticSequenceParticle::updateShape(float delta) {
	std::pair <float, std::string> ans = { 2e9, "" };
	for (int i=0; i<kineticMap.size(); ++i) {
		auto it = kineticMap[i];
		if (it.first >= energy && it.first < ans.first) {
			ans = it;
		}
	}

	ScreenObject::updateShape(ans.second);
}

FlashParticle::FlashParticle(int priority, float energy, float energyFallOff, std::pair<float, float> pos, std::pair<float, float> velocity, std::pair<float, float> accel)
	: Particle(priority, energy, energyFallOff, pos, velocity, accel)
{}

void FlashParticle::updateShape(float delta) {
	show();
	data = LineDraw::draw((int) old_position.first, (int) old_position.second, x, y, "/\\..");
}
