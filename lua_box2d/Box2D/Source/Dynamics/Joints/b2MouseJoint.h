/*
* Copyright (c) 2006-2007 Erin Catto http://www.gphysics.com
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_MOUSE_JOINT_H
#define B2_MOUSE_JOINT_H

#include "b2Joint.h"

/// Mouse joint definition. This requires a world target point,
/// tuning parameters, and the time step.
struct b2MouseJointDef : public b2JointDef
{
	b2MouseJointDef()
	{
		type = e_mouseJoint;
		target.Set(0.0f, 0.0f);
		maxForce = 0.0f;
		frequencyHz = 5.0f;
		dampingRatio = 0.7f;
		timeStep = 1.0f / 60.0f;
	}

	/// The initial world target point. This is assumed
	/// to coincide with the body anchor initially.
	b2Vec2 target;

	/// The maximum constraint force that can be exerted
	/// to move the candidate body. Usually you will express
	/// as some multiple of the weight (multiplier * mass * gravity).
	float32 maxForce;

	/// The response speed.
	float32 frequencyHz;

	/// The damping ratio. 0 = no damping, 1 = critical damping.
	float32 dampingRatio;

	/// The time step used in the simulation.
	float32 timeStep;
};

/// A mouse joint is used to make a point on a body track a
/// specified world point. This a soft constraint with a maximum
/// force. This allows the constraint to stretch and without
/// applying huge forces.
class b2MouseJoint : public b2Joint
{
public:

	/// Implements b2Joint.
	b2Vec2 GetAnchor1() const;

	/// Implements b2Joint.
	b2Vec2 GetAnchor2() const;

	/// Implements b2Joint.
	b2Vec2 GetReactionForce() const;

	/// Implements b2Joint.
	float32 GetReactionTorque() const;

	/// Use this to update the target point.
	void SetTarget(const b2Vec2& target);

	//--------------- Internals Below -------------------

	b2MouseJoint(const b2MouseJointDef* def);

	void InitVelocityConstraints(const b2TimeStep& step);
	void SolveVelocityConstraints(const b2TimeStep& step);
	bool SolvePositionConstraints()
	{
		return true;
	}

	b2Vec2 m_localAnchor;
	b2Vec2 m_target;
	b2Vec2 m_impulse;

	b2Mat22 m_mass;		// effective mass for point-to-point constraint.
	b2Vec2 m_C;				// position error
	float32 m_maxForce;
	float32 m_beta;			// bias factor
	float32 m_gamma;		// softness
};

#endif
