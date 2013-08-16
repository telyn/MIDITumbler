#include "TumblerController.h"
#include <cstdlib>
#include <time.h>

rgbColour makeRandomNoteColour();

//categories

//0bxyz
// x = tumbler body
// y = tumbler edge
// z = note


TumblerController::TumblerController(TumblerPlugin *plugin) : Timer(15),
	world(b2Vec2(0.0f, -30.0f)) {
	this->plugin = plugin;
	this->currentNote = 0;
	this->currentQueueSize = 0;

	b2Body* ground = NULL;
	{
		b2BodyDef bd;
		ground = world.CreateBody(&bd);
	}

	{
		b2BodyDef bd;
		bd.type = b2_dynamicBody;
		bd.allowSleep = false;
		bd.position.Set(0.0f, 0.0f);
		bd.userData = NULL;
		
		this->tumbler = world.CreateBody(&bd);
		
		{
			b2FixtureDef def;
			b2PolygonShape mainShape;
			mainShape.SetAsBox(40.0f, 40.0f, b2Vec2(0.0f, 0.0f), 0);
			def.shape = &mainShape;
			def.density = 10.0f;
			def.friction = 0.0f;
			def.restitution = 0.0f;
			def.filter.categoryBits = 0x4;
			def.filter.maskBits = 0;
			tumbler->CreateFixture(&def);
		}

		float r = 8.66025403784439f;
		for (int i = 0; i < 6; i++) {
			float angle = (float)DEG_TO_RAD(-i/6.0f * 360.0f);

			bd.position.Set(r*sinf(angle), -r*cosf(angle));
			bd.angle = angle;
			bd.fixedRotation = true;
			b2Body* part = world.CreateBody(&bd);
			
			b2PolygonShape edge;
			
			edge.SetAsBox(5.0f, 0.4f, b2Vec2(0.0f,0.0f), 0);

			b2FixtureDef def;
			def.shape = &edge;
			def.density = 1.0f;
			
			def.friction = 0.5f;
			def.restitution = 0.5f;
			def.filter.categoryBits = 0x2;
			def.filter.maskBits = 0x1;
			part->CreateFixture(&def);
			
			
			b2RevoluteJointDef jd;
			jd.localAnchorA.Set(r*sinf(angle), -r*cosf(angle));
			jd.localAnchorB.Set(0.0f, 0.0f);
			jd.bodyA = tumbler;
			jd.bodyB = part;
			//jd.enableLimit = true;
			//jd.lowerAngle = angle;
			//jd.upperAngle = angle;
			jd.maxMotorTorque = 0.0f;
			jd.enableMotor = false;
			jd.motorSpeed = 0.0f;

			this->world.CreateJoint(&jd);
		}
		b2RevoluteJointDef jd;
		jd.bodyA = ground;
		jd.bodyB = tumbler;
		jd.localAnchorA.Set(0.0f, 0.0f);
		jd.localAnchorB.Set(0.0f, 0.0f);
		
		jd.motorSpeed = 0.0f*b2_pi;
		jd.maxMotorTorque = 1e18f;
		jd.enableMotor = true;
		b2RevoluteJoint *baseJoint = (b2RevoluteJoint*)this->world.CreateJoint(&jd);
		
		((TumblerEditor*)plugin->getEditor())->setNotesAndTumbler(tumbler, baseJoint, this->notes, MAX_NOTES);	
	}
	
	this->listener.SetController(this);
	this->world.SetContactListener(&listener);
	//this->addNote(60);
	start();
}

TumblerController::~TumblerController() {
	stop();
}

void TumblerController::playNote(TumblerNote *note, int velocity) {
	this->plugin->MIDI_NoteOn(0,note->GetNote(), velocity, 0);
}

void TumblerController::queueNote(int midi_note) {
	this->noteQueue[this->currentQueueSize++] = midi_note;
	if(this->currentQueueSize == NOTE_QUEUE_LENGTH) {
		this->currentQueueSize = NOTE_QUEUE_LENGTH-1;
	}
}

void TumblerController::addNote(int midi_note) {
	b2BodyDef bd;
	bd.type = b2_dynamicBody;
	b2Body *body = world.CreateBody(&bd);
	b2CircleShape shape;
	// this is kinda bad. magic one liner to extract velocity from note.
	shape.m_radius = 0.3f + ((float)(midi_note >> 8) / 127.0f);
	shape.m_p = b2Vec2(0.0f, 0.0f);
	b2FixtureDef def;
	def.shape = &shape;
	def.density = 0.1f;
	def.friction = 0.0f;
	def.filter.categoryBits = 0x1;
	def.filter.maskBits = 0x3;
	def.restitution = 0.95f * this->plugin->getParameter(TumblerPlugin::RESTITUTION);
	body->CreateFixture(&def);

	
	this->notes[this->currentNote].Set(midi_note, body);
	this->currentNote = (this->currentNote+1) % MAX_NOTES;
	
}

void TumblerController::timerCallback() {
	static int stepCount = 0;
	// set the rotation of the edges.
	world.SetGravity(b2Vec2(0, -60 * this->plugin->getParameter(TumblerPlugin::GRAVITY)));
	b2RevoluteJoint *joint = (b2RevoluteJoint *)this->tumbler->GetJointList()->joint;
	joint->SetMotorSpeed(2 * b2_pi * this->plugin->getParameter(TumblerPlugin::ROTATIONSPEED));
	joint = (b2RevoluteJoint *)joint->GetNext();
	b2RevoluteJoint *firstJoint = joint;
	int i = 1;
	while(joint != NULL) {
		b2Body *body = joint->GetBodyB();
		float angle = this->tumbler->GetAngle() + (b2_pi * 0.5f * -this->plugin->getParameter(TumblerPlugin::OPENNESS)) + ((b2_pi * i) / 3);
		b2Vec2 pos = body->GetPosition();
		body->SetTransform(pos, angle);

		i++;
		joint = (b2RevoluteJoint *)joint->GetNext();
		if(joint == firstJoint) {
			break;

		}
	}

	for(int i = 0; i < MAX_NOTES; i++) {
		if(this->notes[i].IsInitialised()) {
			b2Fixture *fixture = this->notes[i].GetBody()->GetFixtureList();
			fixture->SetRestitution(0.95f*this->plugin->getParameter(TumblerPlugin::RESTITUTION));
			//fixture->SetFriction(0.0f);
		}
	}

	this->world.Step(0.015f, 6, 2);

	//send MIDI noteoffs for all notes... this is kinda inefficient but WHATEVER i@M OVER IT
	// I'll fix it later..
	stepCount++;
	if(stepCount > 10) {
		for(int i = 0; i < MAX_NOTES; i++) { // casually rebinding i
			if(this->notes[i].IsInitialised()) {
				this->plugin->MIDI_NoteOff(0, this->notes[i].GetNote(), 127, 0);

				// as we're here, check to see if it's miles away from origin.
				if(this->notes[i].GetBody()->GetPosition().LengthSquared() > 225) {
					this->notes[i].UnSet();
				}
			}
		}

		// now dequeue notes and add them
		if(this->currentQueueSize > 0) {
			do {
				this->addNote(this->noteQueue[--this->currentQueueSize]);
			} while(this->currentQueueSize > 0);
		}
		stepCount = 0;
	}
}

void TumblerContactListener::SetController(TumblerController *controller) {
		this->controller = controller;
	}
void TumblerContactListener::BeginContact(b2Contact * contact) {
	TumblerNote *note = NULL;
	b2Body *body = NULL;
	b2Body *bodyA = contact->GetFixtureA()->GetBody();
	b2Body *bodyB = contact->GetFixtureB()->GetBody();
	if(bodyA->GetType() == b2_dynamicBody && bodyA->GetUserData() != NULL) {
		if(bodyB->GetType() == b2_dynamicBody  && bodyB->GetUserData() != NULL) {
			return; // both
		} else {
			// just A
			body = bodyA;
			note = (TumblerNote *)bodyA->GetUserData();
		}
	} else {
		if(bodyB->GetType() == b2_dynamicBody  && bodyB->GetUserData() != NULL) {
			// just B
			body = bodyB;
			note = (TumblerNote *)bodyB->GetUserData();
		} else {
			return;
			// neither
		}
	}
	 
	if(note->IsInitialised()) {
		
		int vel = (int)body->GetLinearVelocity().LengthSquared() / 5;
		vel = vel > 127 ? 127 : vel;
		vel = vel < 1 ? 1 : vel;
		controller->playNote(note, vel);
	}
}

AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	TumblerPlugin *plugin = new TumblerPlugin(audioMaster);
	return plugin;

}