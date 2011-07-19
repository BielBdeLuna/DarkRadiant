#pragma once

#include "imodule.h"
#include <boost/function.hpp>

#include "irenderable.h"

class RenderSystem;
class Matrix4;
template<typename Element> class BasicVector3;
typedef BasicVector3<float> Vector3;
class AABB;

namespace particles
{

// see iparticlestage.h for definition
class IParticleStage;

/**
 * Representation of a particles declaration.
 */
class IParticleDef
{
public:
	class Observer
	{
	public:
		virtual ~Observer() {}

		// Gets invoked after this particle def has been reloaded
		// from the .prt files. RenderableParticles are monitoring
		// this event to re-construct their render information
		virtual void onParticleReload() {} // empty default impl.
	};

    /**
	 * Destructor
	 */
	virtual ~IParticleDef() {}

	/**
	 * Get the name of the particle system.
	 */
	virtual const std::string& getName() const = 0;

	/**
	 * Get the name of the .prt file this particle is defined in.
	 * Might return an empty string if this particle def has not been saved yet.
	 */
	virtual const std::string& getFilename() const = 0;

	/**
	 * Set/get the depth hack flag
	 */
	virtual float getDepthHack() const = 0;
	virtual void setDepthHack(float value) = 0;

	/**
	 * Returns the number of stages for this particle system.
	 */
	virtual std::size_t getNumStages() const = 0;

	/**
	 * Return a specific particle stage (const version)
	 */
	virtual const IParticleStage& getParticleStage(std::size_t stageNum) const = 0;

	/**
	 * Return a specific particle stage (non-const version)
	 */
	virtual IParticleStage& getParticleStage(std::size_t stageNum) = 0;

	// Add or remove an observer to get notified on particle events
	virtual void addObserver(Observer* observer) = 0;
	virtual void removeObserver(Observer* observer) = 0;

	// Comparison operators - particle defs are considered equal if all properties (except the name!),
	// number of stages and stage contents are the equal
	virtual bool operator==(const IParticleDef& other) const = 0;
	virtual bool operator!=(const IParticleDef& other) const = 0;
};
typedef boost::shared_ptr<IParticleDef> IParticleDefPtr;

/**
 * A renderable particle, which is capable of compiling the
 * particle system into actual geometry usable for the backend rendersystem.
 *
 * As it derives from Renderable, this object can be added to a RenderableCollector
 * during the front-end render phase.
 */
class IRenderableParticle :
	public Renderable
{
public:
	/**
	 * Update the particle geometry using the given time in milliseconds.
	 * The rendersystem is needed for acquiring the shaders.
	 *
	 * @rotation: the matrix to orient themselves to the viewer.
	 */
	virtual void update(std::size_t time, RenderSystem& renderSystem, const Matrix4& viewRotation) = 0;

	/**
	 * Get the particle definition used by this renderable.
	 */
	virtual const IParticleDefPtr& getParticleDef() const = 0;

	/**
	 * Set the particle definition. You'll need to call update() after
	 * setting a new particle def.
	 */
	virtual void setParticleDef(const IParticleDefPtr& def) = 0;

	/**
	 * greebo: Particles have a main direction, usually defined by the
	 * emitter's rotation. For a stand-alone particle (without emitter)
	 * this direction defaults to <0,0,1>, but can be overridden here.
	 */
	virtual void setMainDirection(const Vector3& direction) = 0;

	/**
	 * Returns the bounding box taken by the entirety of quads in this particle.
	 * Make sure to call this after the update() method, as getAABB() will
	 * calculate and return the bounds at the time passed to update().
	 */
	virtual const AABB& getBounds() = 0;
};
typedef boost::shared_ptr<IRenderableParticle> IRenderableParticlePtr;

/**
 * Callback for evaluation particle defs.
 */
typedef boost::function< void (const IParticleDef&) > ParticleDefVisitor;

/**
 * Abstract interface for the ParticlesManager module.
 */
class IParticlesManager :
	public RegisterableModule
{
public:
	class Observer
	{
	public:
		virtual ~Observer() {}

		// Gets invoked after the module reloaded the .prt files
		// This gives any observers the chance to react to particle def changes
		virtual void onReloadParticles() {} // empty default impl.
	};

	// Add or remove an observer to get notified on particle events
	virtual void addObserver(Observer* observer) = 0;
	virtual void removeObserver(Observer* observer) = 0;

	/**
	 * Enumerate each particle def.
	 */
	virtual void forEachParticleDef(const ParticleDefVisitor&) const = 0;

	/**
	 * Get a named particle definition, returns NULL if not found.
	 */
	virtual IParticleDefPtr getParticle(const std::string& name) = 0;

	/**
	 * Create a renderable particle, which is capable of compiling the
	 * particle system into actual geometry usable for the backend rendersystem.
	 *
	 * @returns: the renderable particle instance or NULL if the named particle was not found.
	 */
	virtual IRenderableParticlePtr getRenderableParticle(const std::string& name) = 0;

	/**
	 * Reloads the definitions from the .prt files. Any existing references to IParticleDefs
	 * will remain valid, but their contents might change. Anything sensitive to these changes
	 * (like the renderable particles) should subscribe as Observer to this manager class to
	 * get notified on changes/reloads.
	 *
	 * If particle defs are removed from the .prt files, the corresponding IParticleDef instance
	 * will remain in memory, but will be empty after reload.
	 */
	virtual void reloadParticleDefs() = 0;
};

} // namespace

const char* const MODULE_PARTICLESMANAGER = "ParticlesManager";

// Accessor
inline particles::IParticlesManager& GlobalParticlesManager()
{
	// Cache the reference locally
	static particles::IParticlesManager& _particlesManager(
		*boost::static_pointer_cast<particles::IParticlesManager>(
			module::GlobalModuleRegistry().getModule(MODULE_PARTICLESMANAGER)
		)
	);
	return _particlesManager;
}
