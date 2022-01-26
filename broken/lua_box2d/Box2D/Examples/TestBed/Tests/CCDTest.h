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

#ifndef CCD_TEST_H
#define CCD_TEST_H

class CCDTest : public Test
{
public:

	CCDTest()
	{
#if 0
		m_world->m_gravity.SetZero();

		{
			b2PolygonDef sd;
			sd.SetAsBox(0.1f, 10.0f);
			sd.density = 0.0f;

			b2BodyDef bd;
			bd.position.Set(0.0f, 20.0f);
			b2Body* body = m_world->CreateBody(&bd);
			body->CreateShape(&sd);
		}

		{
			b2PolygonDef sd;
			sd.SetAsBox(0.1f, 2.0f);
			sd.density = 1.0f;
			sd.restitution = 0.0f;

			m_angularVelocity = b2Random(-50.0f, 50.0f);
			//m_angularVelocity = 8.5009336f;

			b2BodyDef bd;
			bd.type = b2BodyDef::e_dynamicBody;
			bd.position.Set(50.0f, 20.0f);
			b2Body* body = m_world->CreateBody(&bd);
			body->CreateShape(&sd);
			body->SetMassFromShapes();
			body->SetLinearVelocity(b2Vec2(-200.0f, 0.0f));
			body->SetAngularVelocity(m_angularVelocity);
		}
#elif 0
		{
			b2PolygonDef sd;
			sd.SetAsBox(10.0f, 0.1f);
			sd.density = 0.0f;

			b2BodyDef bd;
			bd.type = b2BodyDef::e_static;
			bd.position.Set(0.0f, -0.2f);
			b2Body* ground = m_world->CreateBody(&bd);
			ground->CreateShape(&sd);
		}

		{
			b2PolygonDef sd;
			sd.SetAsBox(2.0f, 0.1f);
			sd.density = 1.0f;
			sd.restitution = 0.0f;

			b2BodyDef bd1;
			bd1.type = b2BodyDef::e_dynamic;
			bd1.isBullet = true;
			bd1.allowSleep = false;
			bd1.position.Set(0.0f, 20.0f);
			b2Body* b1 = m_world->Create(&bd1);
			b1->CreateShape(&sd);
			b1->SetMassFromShapes();
			b1->SetLinearVelocity(b2Vec2(0.0f, -100.0f));

			sd.SetAsBox(1.0f, 0.1f);
			b2BodyDef bd2;
			bd2.type = b2BodyDef::e_dynamic;
			bd2.isBullet = true;
			bd2.allowSleep = false;
			bd2.position.Set(0.0f, 20.2f);
			b2Body* b2 = m_world->Create(&bd2);
			b2->CreateShape(&sd);
			b2->SetMassFromShapes();
			b2->SetLinearVelocity(b2Vec2(0.0f, -100.0f));

			sd.SetAsBox(0.25f, 0.25f);
			sd.density = 10.0f;
			b2BodyDef bd3;
			bd3.type = b2BodyDef::e_dynamic;
			bd3.isBullet = true;
			bd3.allowSleep = false;
			bd3.position.Set(0.0f, 100.0f);
			b2Body* b3 = m_world->Create(&bd3);
			b3->CreateShape(&sd);
			b3->SetMassFromShapes();
			b3->SetLinearVelocity(b2Vec2(0.0f, -150.0f));
		}
#else
		const float32 k_restitution = 1.4f;

		{
			b2BodyDef bd;
			bd.position.Set(0.0f, 20.0f);
			b2Body* body = m_world->CreateBody(&bd);

			b2PolygonDef sd;
			sd.density = 0.0f;
			sd.restitution = k_restitution;

			sd.SetAsBox(0.1f, 10.0f, b2Vec2(-10.0f, 0.0f), 0.0f);
			body->CreateShape(&sd);

			sd.SetAsBox(0.1f, 10.0f, b2Vec2(10.0f, 0.0f), 0.0f);
			body->CreateShape(&sd);

			sd.SetAsBox(0.1f, 10.0f, b2Vec2(0.0f, -10.0f), 0.5f * b2_pi);
			body->CreateShape(&sd);

			sd.SetAsBox(0.1f, 10.0f, b2Vec2(0.0f, 10.0f), -0.5f * b2_pi);
			body->CreateShape(&sd);
		}

#if 0
		{
			b2PolygonDef sd_bottom;
			sd_bottom.SetAsBox(1.0f, 0.1f, b2Vec2(0.0f, -1.0f), 0.0f);
			sd_bottom.density = 4.0f;

			b2PolygonDef sd_top;
			sd_top.SetAsBox(1.0f, 0.1f, b2Vec2(0.0f,  1.0f), 0.0f);
			sd_top.density = 4.0f;

			b2PolygonDef sd_left;
			sd_left.SetAsBox(0.1f, 1.0f, b2Vec2(-1.0f, 0.0f), 0.0f);
			sd_left.density = 4.0f;

			b2PolygonDef sd_right;
			sd_right.SetAsBox(0.1f, 1.0f, b2Vec2(1.0f, 0.0f), 0.0f);
			sd_right.density = 4.0f;

			b2BodyDef bd;
			bd.type = b2BodyDef::e_dynamicBody;
			bd.position.Set(0.0f, 15.0f);
			b2Body* body = m_world->CreateBody(&bd);
			body->CreateShape(&sd_bottom);
			body->CreateShape(&sd_top);
			body->CreateShape(&sd_left);
			body->CreateShape(&sd_right);
			body->SetMassFromShapes();
		}
#elif 1
		{
			b2PolygonDef sd_bottom;
			sd_bottom.SetAsBox( 1.5f, 0.15f );
			sd_bottom.density = 4.0f;

			b2PolygonDef sd_left;
			sd_left.SetAsBox(0.15f, 2.7f, b2Vec2(-1.45f, 2.35f), 0.2f);
			sd_left.density = 4.0f;

			b2PolygonDef sd_right;
			sd_right.SetAsBox(0.15f, 2.7f, b2Vec2(1.45f, 2.35f), -0.2f);
			sd_right.density = 4.0f;

			b2BodyDef bd;
			bd.position.Set( 0.0f, 15.0f );
			b2Body* body = m_world->CreateBody(&bd);
			body->CreateShape(&sd_bottom);
			body->CreateShape(&sd_left);
			body->CreateShape(&sd_right);
			body->SetMassFromShapes();
		}
#else
		{
			b2BodyDef bd;
			bd.type = b2BodyDef::e_dynamic;
			bd.position.Set(-5.0f, 20.0f);
			bd.isBullet = true;
			b2Body* body = m_world->CreateBody(&bd);
			body->SetAngularVelocity(b2Random(-50.0f, 50.0f));

			b2PolygonDef sd;
			sd.SetAsBox(0.1f, 4.0f);
			sd.density = 1.0f;
			sd.restitution = 0.0f;
			body->CreateShape(&sd);
			body->SetMassFromShapes();
		}
#endif

		for (int32 i = 0; i < 0; ++i)
		{
			b2BodyDef bd;
			bd.position.Set(0.0f, 15.0f + i);
			bd.isBullet = true;
			b2Body* body = m_world->CreateBody(&bd);
			body->SetAngularVelocity(b2Random(-50.0f, 50.0f));

			b2CircleDef sd;
			sd.radius = 0.25f;
			sd.density = 1.0f;
			sd.restitution = 0.0f;
			body->CreateShape(&sd);
			body->SetMassFromShapes();
		}
#endif
	}

	static Test* Create()
	{
		return new CCDTest;
	}

	float32 m_angularVelocity;
};

#endif
