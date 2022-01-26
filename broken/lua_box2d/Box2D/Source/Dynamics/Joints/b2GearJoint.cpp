/*
* Copyright (c) 2007 Erin Catto http://www.gphysics.com
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

#include "b2GearJoint.h"
#include "b2RevoluteJoint.h"
#include "b2PrismaticJoint.h"
#include "../b2Body.h"
#include "../b2World.h"

// Gear Joint:
// C0 = (coordinate1 + ratio * coordinate2)_initial
// C = C0 - (cordinate1 + ratio * coordinate2) = 0
// Cdot = -(Cdot1 + ratio * Cdot2)
// J = -[J1 ratio * J2]
// K = J * invM * JT
//   = J1 * invM1 * J1T + ratio * ratio * J2 * invM2 * J2T
//
// Revolute:
// coordinate = rotation
// Cdot = angularVelocity
// J = [0 0 1]
// K = J * invM * JT = invI
//
// Prismatic:
// coordinate = dot(p - pg, ug)
// Cdot = dot(v + cross(w, r), ug)
// J = [ug cross(r, ug)]
// K = J * invM * JT = invMass + invI * cross(r, ug)^2

b2GearJoint::b2GearJoint(const b2GearJointDef* def)
: b2Joint(def)
{
	b2JointType type1 = def->joint1->GetType();
	b2JointType type2 = def->joint2->GetType();

	b2Assert(type1 == e_revoluteJoint || type1 == e_prismaticJoint);
	b2Assert(type2 == e_revoluteJoint || type2 == e_prismaticJoint);
	b2Assert(def->joint1->GetBody1()->IsStatic());
	b2Assert(def->joint2->GetBody1()->IsStatic());

	m_revolute1 = NULL;
	m_prismatic1 = NULL;
	m_revolute2 = NULL;
	m_prismatic2 = NULL;

	float32 coordinate1, coordinate2;

	m_ground1 = def->joint1->GetBody1();
	m_body1 = def->joint1->GetBody2();
	if (type1 == e_revoluteJoint)
	{
		m_revolute1 = (b2RevoluteJoint*)def->joint1;
		m_groundAnchor1 = m_revolute1->m_localAnchor1;
		m_localAnchor1 = m_revolute1->m_localAnchor2;
		coordinate1 = m_revolute1->GetJointAngle();
	}
	else
	{
		m_prismatic1 = (b2PrismaticJoint*)def->joint1;
		m_groundAnchor1 = m_prismatic1->m_localAnchor1;
		m_localAnchor1 = m_prismatic1->m_localAnchor2;
		coordinate1 = m_prismatic1->GetJointTranslation();
	}

	m_ground2 = def->joint2->GetBody1();
	m_body2 = def->joint2->GetBody2();
	if (type2 == e_revoluteJoint)
	{
		m_revolute2 = (b2RevoluteJoint*)def->joint2;
		m_groundAnchor2 = m_revolute2->m_localAnchor1;
		m_localAnchor2 = m_revolute2->m_localAnchor2;
		coordinate2 = m_revolute2->GetJointAngle();
	}
	else
	{
		m_prismatic2 = (b2PrismaticJoint*)def->joint2;
		m_groundAnchor2 = m_prismatic2->m_localAnchor1;
		m_localAnchor2 = m_prismatic2->m_localAnchor2;
		coordinate2 = m_prismatic2->GetJointTranslation();
	}

	m_ratio = def->ratio;

	m_constant = coordinate1 + m_ratio * coordinate2;

	m_force = 0.0f;
}

void b2GearJoint::InitVelocityConstraints(const b2TimeStep& step)
{
	b2Body* g1 = m_ground1;
	b2Body* g2 = m_ground2;
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	float32 K = 0.0f;
	m_J.SetZero();

	if (m_revolute1)
	{
		m_J.angular1 = -1.0f;
		K += b1->m_invI;
	}
	else
	{
		b2Vec2 ug = b2Mul(g1->GetXForm().R, m_prismatic1->m_localXAxis1);
		b2Vec2 r = b2Mul(b1->GetXForm().R, m_localAnchor1 - b1->GetLocalCenter());
		float32 crug = b2Cross(r, ug);
		m_J.linear1 = -ug;
		m_J.angular1 = -crug;
		K += b1->m_invMass + b1->m_invI * crug * crug;
	}

	if (m_revolute2)
	{
		m_J.angular2 = -m_ratio;
		K += m_ratio * m_ratio * b2->m_invI;
	}
	else
	{
		b2Vec2 ug = b2Mul(g2->GetXForm().R, m_prismatic2->m_localXAxis1);
		b2Vec2 r = b2Mul(b2->GetXForm().R, m_localAnchor2 - b2->GetLocalCenter());
		float32 crug = b2Cross(r, ug);
		m_J.linear2 = -m_ratio * ug;
		m_J.angular2 = -m_ratio * crug;
		K += m_ratio * m_ratio * (b2->m_invMass + b2->m_invI * crug * crug);
	}

	// Compute effective mass.
	b2Assert(K > 0.0f);
	m_mass = 1.0f / K;

	if (step.warmStarting)
	{
		// Warm starting.
		float32 P = B2FORCE_SCALE(step.dt) * m_force;
		b1->m_linearVelocity += b1->m_invMass * P * m_J.linear1;
		b1->m_angularVelocity += b1->m_invI * P * m_J.angular1;
		b2->m_linearVelocity += b2->m_invMass * P * m_J.linear2;
		b2->m_angularVelocity += b2->m_invI * P * m_J.angular2;
	}
	else
	{
		m_force = 0.0f;
	}
}

void b2GearJoint::SolveVelocityConstraints(const b2TimeStep& step)
{
	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	float32 Cdot = m_J.Compute(	b1->m_linearVelocity, b1->m_angularVelocity,
								b2->m_linearVelocity, b2->m_angularVelocity);

	float32 force = -B2FORCE_INV_SCALE(step.inv_dt) * m_mass * Cdot;
	m_force += force;

	float32 P = B2FORCE_SCALE(step.dt) * force;
	b1->m_linearVelocity += b1->m_invMass * P * m_J.linear1;
	b1->m_angularVelocity += b1->m_invI * P * m_J.angular1;
	b2->m_linearVelocity += b2->m_invMass * P * m_J.linear2;
	b2->m_angularVelocity += b2->m_invI * P * m_J.angular2;
}

bool b2GearJoint::SolvePositionConstraints()
{
	float32 linearError = 0.0f;

	b2Body* b1 = m_body1;
	b2Body* b2 = m_body2;

	float32 coordinate1, coordinate2;
	if (m_revolute1)
	{
		coordinate1 = m_revolute1->GetJointAngle();
	}
	else
	{
		coordinate1 = m_prismatic1->GetJointTranslation();
	}

	if (m_revolute2)
	{
		coordinate2 = m_revolute2->GetJointAngle();
	}
	else
	{
		coordinate2 = m_prismatic2->GetJointTranslation();
	}

	float32 C = m_constant - (coordinate1 + m_ratio * coordinate2);

	float32 impulse = -m_mass * C;

	b1->m_sweep.c += b1->m_invMass * impulse * m_J.linear1;
	b1->m_sweep.a += b1->m_invI * impulse * m_J.angular1;
	b2->m_sweep.c += b2->m_invMass * impulse * m_J.linear2;
	b2->m_sweep.a += b2->m_invI * impulse * m_J.angular2;

	b1->SynchronizeTransform();
	b2->SynchronizeTransform();

	return linearError < b2_linearSlop;
}

b2Vec2 b2GearJoint::GetAnchor1() const
{
	return m_body1->GetWorldPoint(m_localAnchor1);
}

b2Vec2 b2GearJoint::GetAnchor2() const
{
	return m_body2->GetWorldPoint(m_localAnchor2);
}

b2Vec2 b2GearJoint::GetReactionForce() const
{
	// TODO_ERIN not tested
	b2Vec2 F = B2FORCE_SCALE(m_force) * m_J.linear2;
	return F;
}

float32 b2GearJoint::GetReactionTorque() const
{
	// TODO_ERIN not tested
	b2Vec2 r = b2Mul(m_body2->GetXForm().R, m_localAnchor2 - m_body2->GetLocalCenter());
	b2Vec2 F = m_force * m_J.linear2;
	float32 T = B2FORCE_SCALE(m_force * m_J.angular2 - b2Cross(r, F));
	return T;
}

float32 b2GearJoint::GetRatio() const
{
	return m_ratio;
}


