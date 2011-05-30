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

#ifndef B2_PRISMATIC_JOINT_H
#define B2_PRISMATIC_JOINT_H

#include "b2Joint.h"

/// Prismatic joint definition. This requires defining a line of
/// motion using an axis and an anchor point. The definition uses local
/// anchor points and a local axis so that the initial configuration
/// can violate the constraint slightly. The joint translation is zero
/// when the local anchor points coincide in world space. Using local
/// anchors and a local axis helps when saving and loading a game.
struct b2PrismaticJointDef : public b2JointDef
{
	b2PrismaticJointDef()
	{
		type = e_prismaticJoint;
		localAnchor1.SetZero();
		localAnchor2.SetZero();
		localAxis1.Set(1.0f, 0.0f);
		referenceAngle = 0.0f;
		enableLimit = false;
		lowerTranslation = 0.0f;
		upperTranslation = 0.0f;
		enableMotor = false;
		maxMotorForce = 0.0f;
		motorSpeed = 0.0f;
	}

	/// Initialize the bodies, anchors, axis, and reference angle using the world
	/// anchor and world axis.
	void Initialize(b2Body* body1, b2Body* body2, const b2Vec2& anchor, const b2Vec2& axis);

	/// The local anchor point relative to body1's origin.
	b2Vec2 localAnchor1;

	/// The local anchor point relative to body2's origin.
	b2Vec2 localAnchor2;

	/// The local translation axis in body1.
	b2Vec2 localAxis1;

	/// The constrained angle between the bodies: body2_angle - body1_angle.
	float32 referenceAngle;

	/// Enable/disable the joint limit.
	bool enableLimit;

	/// The lower translation limit, usually in meters.
	float32 lowerTranslation;

	/// The upper translation limit, usually in meters.
	float32 upperTranslation;

	/// Enable/disable the joint motor.
	bool enableMotor;

	/// The maximum motor torque, usually in N-m.
	float32 maxMotorForce;

	/// The desired motor speed in radians per second.
	float32 motorSpeed;
};

/// A prismatic joint. This joint provides one degree of freedom: translation
/// along an axis fixed in body1. Relative rotation is prevented. You can
/// use a joint limit to restrict the range of motion and a joint motor to
/// drive the motion or to model joint friction.
class b2PrismaticJoint : public b2Joint
{
public:
	b2Vec2 GetAnchor1() const;
	b2Vec2 GetAnchor2() const;

	b2Vec2 GetReactionForce() const;
	float32 GetReactionTorque() const;

	/// Get the current joint translation, usually in meters.
	float32 GetJointTranslation() const;

	/// Get the current joint translation speed, usually in meters per second.
	float32 GetJointSpeed() const;

	/// Is the joint limit enabled?
	bool IsLimitEnabled() const;

	/// Enable/disable the joint limit.
	void EnableLimit(bool flag);

	/// Get the lower joint limit, usually in meters.
	float32 GetLowerLimit() const;

	/// Get the upper joint limit, usually in meters.
	float32 GetUpperLimit() const;

	/// Set the joint limits, usually in meters.
	void SetLimits(float32 lower, float32 upper);

	/// Is the joint motor enabled?
	bool IsMotorEnabled() const;

	/// Enable/disable the joint motor.
	void EnableMotor(bool flag);

	/// Set the motor speed, usually in meters per second.
	void SetMotorSpeed(float32 speed);

	/// Get the motor speed, usually in meters per second.
	float32 GetMotorSpeed() const;

	/// Set the maximum motor force, usually in N.
	void SetMaxMotorForce(float32 force);

	/// Get the current motor force, usually in N.
	float32 GetMotorForce() const;

	//--------------- Internals Below -------------------

	b2PrismaticJoint(const b2PrismaticJointDef* def);

	void InitVelocityConstraints(const b2TimeStep& step);
	void SolveVelocityConstraints(const b2TimeStep& step);
	bool SolvePositionConstraints();

	b2Vec2 m_localAnchor1;
	b2Vec2 m_localAnchor2;
	b2Vec2 m_localXAxis1;
	b2Vec2 m_localYAxis1;
	float32 m_refAngle;

	b2Jacobian m_linearJacobian;
	float32 m_linearMass;				// effective mass for point-to-line constraint.
	float32 m_force;
	
	float32 m_angularMass;			// effective mass for angular constraint.
	float32 m_torque;

	b2Jacobian m_motorJacobian;
	float32 m_motorMass;			// effective mass for motor/limit translational constraint.
	float32 m_motorForce;
	float32 m_limitForce;
	float32 m_limitPositionImpulse;

	float32 m_lowerTranslation;
	float32 m_upperTranslation;
	float32 m_maxMotorForce;
	float32 m_motorSpeed;
	
	bool m_enableLimit;
	bool m_enableMotor;
	b2LimitState m_limitState;
};

inline float32 b2PrismaticJoint::GetMotorSpeed() const
{
	return m_motorSpeed;
}

#endif
