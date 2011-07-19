#ifndef PARTICLESTAGE_H_
#define PARTICLESTAGE_H_

#include "math/Vector4.h"

#include "iparticles.h"

#include <iostream>
#include "parser/DefTokeniser.h"

#include "ParticleParameter.h"

namespace particles
{

/**
 * Representation of a single particle stage. Each stage consists of a set of
 * particles with the same properties (texture, acceleration etc).
 *
 * Most of the member descriptions are directly taken from the D3 SDK.
 */
class ParticleStage :
	public IParticleStage
{
public:
private:
	friend std::ostream& operator<< (std::ostream&, const ParticleStage&);

	// Number of particles
	int _count;						// total number of particles, although some may be invisible at a given time

	// Material to render onto each quad
	std::string _material;

	float _duration;				// Duration in seconds
	float _cycles;					// allows things to oneShot ( 1 cycle ) or run for a set number of cycles
									// on a per stage basis
	float _bunching;				// 0.0 = all come out at first instant, 1.0 = evenly spaced over cycle time

	float _timeOffset;				// time offset from system start for the first particle to spawn
	float _deadTime;				// time after particleLife before respawning

	int _cycleMsec;					// calculated as ( _duration + _deadTime ) in msec, read-only for public

	Vector4 _colour;				// Render colour
	Vector4 _fadeColour;			// either 0 0 0 0 for additive, or 1 1 1 0 for blended materials

	float _fadeInFraction;			// in 0.0 to 1.0 range
	float _fadeOutFraction;			// in 0.0 to 1.0 range
	float _fadeIndexFraction;		// in 0.0 to 1.0 range, causes later index smokes to be more faded

	int	_animationFrames;			// if > 1, subdivide the texture S axis into frames and crossfade
	float _animationRate;			// frames per second

	float _initialAngle;			// in degrees, random angle is used if zero ( default )

	ParticleParameter _rotationSpeed; // half the particles will have negative rotation speeds,
									  // this is measured in degrees/sec

	float _boundsExpansion;			// user tweak to fix poorly calculated bounds

	bool _randomDistribution;		// randomly orient the quad on emission ( defaults to true )
	bool _entityColor;				// force color from render entity ( fadeColor is still valid )

	float _gravity;					// can be negative to float up
	bool _applyWorldGravity;		// apply gravity in world space

	Vector3 _offset;				// offset from origin to spawn all particles, also applies to customPath

	OrientationType _orientationType;	// view, aimed, or axis fixed
	float _orientationParms[4];			// Orientation parameters

	// Standard path parms

	DistributionType _distributionType;	// Distribution type
	float _distributionParms[4];		// Distribution parameters

	DirectionType _directionType;	// Direction type
	float _directionParms[4];		// Direction parameters

	ParticleParameter _speed;		// Speed

	// Custom path will completely replace the standard path calculations

	CustomPathType _customPathType;	// use custom C code routines for determining the origin
	float _customPathParms[8];		// custom path parameters

	ParticleParameter _size;		// Size

	ParticleParameter _aspect;		// greater than 1 makes the T axis longer

public:
	// Create an empty particle stage with default values
	ParticleStage();

	// Create a particle stage from the given token stream
	ParticleStage(parser::DefTokeniser& tok);

	// Resets/clears all values to default. This is called by parseFromTokens().
	void reset();

	void copyFrom(const IParticleStage& other);

	/**
	 * Get the shader name.
	 */
	const std::string& getMaterialName() const { return _material; }

	/**
	 * Set the shader name.
	 */
	void setMaterialName(const std::string& material) { _material = material; }

	/**
	 * Get the particle count.
	 */
	int getCount() const { return _count; }

	/**
	 * Set the particle count.
	 */
	void setCount(int count) { _count = count; }

	/**
	 * Get the duration in seconds.
	 */
	float getDuration() const { return _duration; }

	/**
	 * Set the duration in seconds, updates cyclemsec.
	 */
	void setDuration(float duration) { _duration = duration; recalculateCycleMsec(); }

	/**
	 * Returns ( duration + deadTime ) in msec.
	 */
	int getCycleMsec() const { return _cycleMsec; }

	/**
	 * Get the cycles value.
	 */
	float getCycles() const { return _cycles; }

	/**
	 * Set the cycles value.
	 */
	void setCycles(float cycles) { _cycles = clampZeroOrPositive(cycles); }

	/**
	 * Get the bunching value [0..1]
	 */
	float getBunching() const { return _bunching; }

	/**
	 * Set the bunching value [0..1]
	 */
	void setBunching(float value) { _bunching = clampOneZero(value); }

	/**
	 * Get the time offset in seconds
	 */
	float getTimeOffset() const { return _timeOffset; }

	/**
	 * Set the time offset in seconds
	 */
	void setTimeOffset(float value) { _timeOffset = value; }

	/**
	 * Get the dead time in seconds
	 */
	float getDeadTime() const { return _deadTime; }

	/**
	 * Set the dead time in seconds, updates cyclemsec.
	 */
	void setDeadTime(float value) { _deadTime = value; recalculateCycleMsec(); }

	/**
	 * Get the particle render colour.
	 */
	const Vector4& getColour() const { return _colour; }

	/**
	 * Set the particle render colour.
	 */
	void setColour(const Vector4& colour) { _colour = colour; }

	/**
	 * Get the particle render colour.
	 */
	const Vector4& getFadeColour() const { return _fadeColour; }

	/**
	 * Set the particle render colour.
	 */
	void setFadeColour(const Vector4& colour) { _fadeColour = colour; }

	/**
	 * Get the fade in fraction [0..1]
	 */
	float getFadeInFraction() const { return _fadeInFraction; }

	/**
	 * Set the fade in fraction [0..1]
	 */
	void setFadeInFraction(float fraction) { _fadeInFraction = clampOneZero(fraction); }

	/**
	 * Get the fade out fraction [0..1]
	 */
	float getFadeOutFraction() const { return _fadeOutFraction; }

	/**
	 * Set the fade out fraction [0..1]
	 */
	void setFadeOutFraction(float fraction) { _fadeOutFraction = clampOneZero(fraction); }

	/**
	 * Get the fade index fraction [0..1]
	 */
	float getFadeIndexFraction() const { return _fadeIndexFraction; }

	/**
	 * Set the fade index fraction [0..1]
	 */
	void setFadeIndexFraction(float fraction) { _fadeIndexFraction = clampOneZero(fraction); }

	/**
	 * Get the animation frames.
	 */
	int getAnimationFrames() const { return _animationFrames; }

	/**
	 * Set the animation frames.
	 */
	void setAnimationFrames(int animationFrames) { _animationFrames = animationFrames; }

	/**
	 * Get the animation rate.
	 */
	float getAnimationRate() const { return _animationRate; }

	/**
	 * Set the animation frames.
	 */
	void setAnimationRate(float animationRate) { _animationRate = animationRate; }

	/**
	 * Get the initial angle.
	 */
	float getInitialAngle() const { return _initialAngle; }

	/**
	 * Set the initial angle.
	 */
	void setInitialAngle(float angle) { _initialAngle = angle; }

	/**
	 * Get the bounds expansion value.
	 */
	float getBoundsExpansion() const { return _boundsExpansion; }

	/**
	 * Set the bounds expansion value.
	 */
	void setBoundsExpansion(float value) { _boundsExpansion = value; }

	/**
	 * Get the random distribution flag.
	 */
	bool getRandomDistribution() const { return _randomDistribution; }

	/**
	 * Set the random distribution flag.
	 */
	void setRandomDistribution(bool value) { _randomDistribution = value; }

	/**
	 * Get the "use entity colour" flag.
	 */
	bool getUseEntityColour() const { return _entityColor; }

	/**
	 * Set the "use entity colour" flag.
	 */
	void setUseEntityColour(bool value) { _entityColor = value; }

	/**
	 * Get the gravity factor.
	 */
	float getGravity() const { return _gravity; }

	/**
	 * Set the gravity factor.
	 */
	void setGravity(float value) { _gravity = value; }

	/**
	 * Get the "apply gravity in world space" flag.
	 */
	bool getWorldGravityFlag() const { return _applyWorldGravity; }

	/**
	 * Get the "apply gravity in world space" flag.
	 */
	void setWorldGravityFlag(bool value) { _applyWorldGravity = value; }

	/**
	 * Get the offset vector.
	 */
	const Vector3& getOffset() const { return _offset; }

	/**
	 * Set the offset vector.
	 */
	void setOffset(const Vector3& value) { _offset = value; }

	/**
	 * Get the orientation type.
	 */
	OrientationType getOrientationType() const { return _orientationType; }

	/**
	 * Set the orientation type.
	 */
	void setOrientationType(OrientationType value) { _orientationType = value; }

	/**
	 * Get the orientation parameter with the given index [0..3]
	 */
	float getOrientationParm(int parmNum) const
	{
		assert(parmNum >= 0 && parmNum < 4);
		return _orientationParms[parmNum];
	}

	/*
	 * Set the orientation parameter with the given index [0..3].
	 */
	void setOrientationParm(int parmNum, float value)
	{
		assert(parmNum >= 0 && parmNum < 4);
		_orientationParms[parmNum] = value;
	}

	/**
	 * Get the distribution type.
	 */
	DistributionType getDistributionType() const { return _distributionType; }

	/**
	 * Set the distribution type.
	 */
	void setDistributionType(DistributionType value) { _distributionType = value; }

	/**
	 * Get the distribution parameter with the given index [0..3]
	 */
	float getDistributionParm(int parmNum) const
	{
		assert(parmNum >= 0 && parmNum < 4);
		return _distributionParms[parmNum];
	}

	/*
	 * Set the distribution parameter with the given index [0..3].
	 */
	void setDistributionParm(int parmNum, float value)
	{
		assert(parmNum >= 0 && parmNum < 4);
		_distributionParms[parmNum] = value;
	}

	/**
	 * Get the direction type.
	 */
	DirectionType getDirectionType() const { return _directionType; }

	/**
	 * Set the direction type.
	 */
	void setDirectionType(DirectionType value) { _directionType = value; }

	/**
	 * Get the direction parameter with the given index [0..3]
	 */
	float getDirectionParm(int parmNum) const
	{
		assert(parmNum >= 0 && parmNum < 4);
		return _directionParms[parmNum];
	}

	/*
	 * Set the direction parameter with the given index [0..3].
	 */
	void setDirectionParm(int parmNum, float value)
	{
		assert(parmNum >= 0 && parmNum < 4);
		_directionParms[parmNum] = value;
	}

	/**
	 * Get the custom path type.
	 */
	CustomPathType getCustomPathType() const { return _customPathType; }

	/**
	 * Set the custom path type.
	 */
	void setCustomPathType(CustomPathType value) { _customPathType = value; }

	/**
	 * Get the custom path parameter with the given index [0..7]
	 */
	float getCustomPathParm(int parmNum) const
	{
		assert(parmNum >= 0 && parmNum < 8);
		return _customPathParms[parmNum];
	}

	/*
	 * Set the custom path parameter with the given index [0..7].
	 */
	void setCustomPathParm(int parmNum, float value)
	{
		assert(parmNum >= 0 && parmNum < 8);
		_customPathParms[parmNum] = value;
	}

	/**
	 * Get the particle size
	 */
	const ParticleParameter& getSize() const { return _size; }
	ParticleParameter& getSize() { return _size; }

	/**
	 * Get the aspect ratio.
	 */
	const ParticleParameter& getAspect() const { return _aspect; }
	ParticleParameter& getAspect() { return _aspect; }

	/**
	 * Get the particle speed.
	 */
	const ParticleParameter& getSpeed() const { return _speed; }
	ParticleParameter& getSpeed() { return _speed; }

	/**
	 * Get the particle rotation speed.
	 */
	const ParticleParameter& getRotationSpeed() const { return _rotationSpeed; }
	ParticleParameter& getRotationSpeed() { return _rotationSpeed; }

	bool operator==(const IParticleStage& other) const 
	{
		if (getMaterialName() != other.getMaterialName()) return false;

		if (getCount() != other.getCount()) return false;
		if (getDuration() != other.getDuration()) return false;
		if (getCycles() != other.getCycles()) return false;
		if (getBunching() != other.getBunching()) return false;
		if (getTimeOffset() != other.getTimeOffset()) return false;
		if (getDeadTime() != other.getDeadTime()) return false;
		if (getColour() != other.getColour()) return false;
		if (getFadeColour() != other.getFadeColour()) return false;
		if (getFadeInFraction() != other.getFadeInFraction()) return false;
		if (getFadeOutFraction() != other.getFadeOutFraction()) return false;
		if (getFadeIndexFraction() != other.getFadeIndexFraction()) return false;
		if (getAnimationFrames() != other.getAnimationFrames()) return false;
		if (getAnimationRate() != other.getAnimationRate()) return false;
		if (getInitialAngle() != other.getInitialAngle()) return false;
		if (getBoundsExpansion() != other.getBoundsExpansion()) return false;
		if (getRandomDistribution() != other.getRandomDistribution()) return false;
		if (getUseEntityColour() != other.getUseEntityColour()) return false;
		if (getGravity() != other.getGravity()) return false;
		if (getWorldGravityFlag() != other.getWorldGravityFlag()) return false;
		if (getOffset() != other.getOffset()) return false;
		if (getOrientationType() != other.getOrientationType()) return false;
		
		for (int i = 0; i < 3; ++i)
		{
			if (getOrientationParm(i) != other.getOrientationParm(i)) return false;
		}

		if (getDistributionType() != other.getDistributionType()) return false;
		
		for (int i = 0; i < 3; ++i)
		{
			if (getDistributionParm(i) != other.getDistributionParm(i)) return false;
		}

		if (getDirectionType() != other.getDirectionType()) return false;

		for (int i = 0; i < 3; ++i)
		{
			if (getDirectionParm(i) != other.getDirectionParm(i)) return false;
		}

		if (getCustomPathType() != other.getCustomPathType()) return false;

		for (int i = 0; i < 7; ++i)
		{
			if (getCustomPathParm(i) != other.getCustomPathParm(i)) return false;
		}

		if (getSize() != other.getSize()) return false;
		if (getAspect() != other.getAspect()) return false;
		if (getSpeed() != other.getSpeed()) return false;
		if (getRotationSpeed() != other.getRotationSpeed()) return false;

		// All checks passed => equal
		return true;
	}

	bool operator!=(const IParticleStage& other) const
	{
		return !operator==(other);
	}

	// Parser method, reads in all stage parameters from the given token stream
	// The initial opening brace { has already been parsed.
	// The routine will continue parsing until the matching closing } is encountered.
	void parseFromTokens(parser::DefTokeniser& tok);

private:
	void recalculateCycleMsec()
	{
		_cycleMsec = static_cast<int>((_duration + _deadTime) * 1000);
	}

	// Clamps the given float to the range [0..1]
	float clampOneZero(float input)
	{
		if (input < 0.0f) return 0.0f;
		if (input > 1.0f) return 1.0f;

		return input;
	}

	// Clamps the given float to positive values
	float clampZeroOrPositive(float input)
	{
		return (input < 0.0f) ? 0.0f : input;
	}

	Vector3 parseVector3(parser::DefTokeniser& tok);
	Vector4 parseVector4(parser::DefTokeniser& tok);
};

/**
 * Stream insertion operator for debugging.
 */
inline std::ostream& operator<< (std::ostream& os, const ParticleStage& stage) {
	os << "ParticleStage { count = " << stage._count << ", "
	   << "colour = " << stage._colour
	   << " }";
	return os;
}

}

#endif /*PARTICLESTAGE_H_*/
