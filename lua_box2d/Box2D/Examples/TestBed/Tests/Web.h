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

#ifndef WEB_H
#define WEB_H

// This tests distance joints, body destruction, and joint destruction.
class Web : public Test
{
public:
	Web()
	{
		b2Body* ground = NULL;
		{
			b2PolygonDef sd;
			sd.SetAsBox(50.0f, 10.0f);

			b2BodyDef bd;
			bd.position.Set(0.0f, -10.0f);
			ground = m_world->CreateBody(&bd);
			ground->CreateShape(&sd);
		}

		{
			b2PolygonDef sd;
			sd.SetAsBox(0.5f, 0.5f);
			sd.density = 5.0f;
			sd.friction = 0.2f;

			b2BodyDef bd;

			bd.position.Set(-5.0f, 5.0f);
			m_bodies[0] = m_world->CreateBody(&bd);
			m_bodies[0]->CreateShape(&sd);
			m_bodies[0]->SetMassFromShapes();

			bd.position.Set(5.0f, 5.0f);
			m_bodies[1] = m_world->CreateBody(&bd);
			m_bodies[1]->CreateShape(&sd);
			m_bodies[1]->SetMassFromShapes();

			bd.position.Set(5.0f, 15.0f);
			m_bodies[2] = m_world->CreateBody(&bd);
			m_bodies[2]->CreateShape(&sd);
			m_bodies[2]->SetMassFromShapes();

			bd.position.Set(-5.0f, 15.0f);
			m_bodies[3] = m_world->CreateBody(&bd);
			m_bodies[3]->CreateShape(&sd);
			m_bodies[3]->SetMassFromShapes();

			b2DistanceJointDef jd;
			b2Vec2 p1, p2, d;

			jd.frequencyHz = 4.0f;
			jd.dampingRatio = 0.5f;

			jd.body1 = ground;
			jd.body2 = m_bodies[0];
			jd.localAnchor1.Set(-10.0f, 10.0f);
			jd.localAnchor2.Set(-0.5f, -0.5f);
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[0] = m_world->CreateJoint(&jd);

			jd.body1 = ground;
			jd.body2 = m_bodies[1];
			jd.localAnchor1.Set(10.0f, 10.0f);
			jd.localAnchor2.Set(0.5f, -0.5f);
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[1] = m_world->CreateJoint(&jd);

			jd.body1 = ground;
			jd.body2 = m_bodies[2];
			jd.localAnchor1.Set(10.0f, 30.0f);
			jd.localAnchor2.Set(0.5f, 0.5f);
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[2] = m_world->CreateJoint(&jd);

			jd.body1 = ground;
			jd.body2 = m_bodies[3];
			jd.localAnchor1.Set(-10.0f, 30.0f);
			jd.localAnchor2.Set(-0.5f, 0.5f);
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[3] = m_world->CreateJoint(&jd);

			jd.body1 = m_bodies[0];
			jd.body2 = m_bodies[1];
			jd.localAnchor1.Set(0.5f, 0.0f);
			jd.localAnchor2.Set(-0.5f, 0.0f);;
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[4] = m_world->CreateJoint(&jd);

			jd.body1 = m_bodies[1];
			jd.body2 = m_bodies[2];
			jd.localAnchor1.Set(0.0f, 0.5f);
			jd.localAnchor2.Set(0.0f, -0.5f);
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[5] = m_world->CreateJoint(&jd);

			jd.body1 = m_bodies[2];
			jd.body2 = m_bodies[3];
			jd.localAnchor1.Set(-0.5f, 0.0f);
			jd.localAnchor2.Set(0.5f, 0.0f);
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[6] = m_world->CreateJoint(&jd);

			jd.body1 = m_bodies[3];
			jd.body2 = m_bodies[0];
			jd.localAnchor1.Set(0.0f, -0.5f);
			jd.localAnchor2.Set(0.0f, 0.5f);
			p1 = jd.body1->GetWorldPoint(jd.localAnchor1);
			p2 = jd.body2->GetWorldPoint(jd.localAnchor2);
			d = p2 - p1;
			jd.length = d.Length();
			m_joints[7] = m_world->CreateJoint(&jd);
		}
	}

	void Keyboard(unsigned char key)
	{
		switch (key)
		{
		case 'b':
			for (int32 i = 0; i < 4; ++i)
			{
				if (m_bodies[i])
				{
					m_world->DestroyBody(m_bodies[i]);
					m_bodies[i] = NULL;
					break;
				}
			}
			break;

		case 'j':
			for (int32 i = 0; i < 8; ++i)
			{
				if (m_joints[i])
				{
					m_world->DestroyJoint(m_joints[i]);
					m_joints[i] = NULL;
					break;
				}
			}
			break;
		}
	}

	void Step(Settings* settings)
	{
		Test::Step(settings);
		DrawString(5, m_textLine, "This demonstrates a soft distance joint.");
		m_textLine += 15;
		DrawString(5, m_textLine, "Press: (b) to delete a body, (j) to delete a joint");
		m_textLine += 15;
	}

	void JointDestroyed(b2Joint* joint)
	{
		for (int32 i = 0; i < 8; ++i)
		{
			if (m_joints[i] == joint)
			{
				m_joints[i] = NULL;
				break;
			}
		}
	}

	static Test* Create()
	{
		return new Web;
	}

	b2Body* m_bodies[4];
	b2Joint* m_joints[8];
};

#endif
