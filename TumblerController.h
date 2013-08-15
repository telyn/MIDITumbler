#ifndef TUMBLERPHYSICS_H
#define TUMBLERPHYSICS_H

class TumblerController;
#define MAX_NOTES 16
#define NOTE_QUEUE_LENGTH 8

#include "TumblerPlugin.h"
#include "TumblerEditor.h"
#include "TumblerNote.h"
#include "Box2D/Box2D.h"
#include "VSTGL/VSTGLTimer.h"

class TumblerContactListener : public b2ContactListener {
private:
	TumblerController *controller;
public: 
	void SetController(TumblerController *controller);
	void BeginContact(b2Contact *contact);
	void PostSolve(b2Contact* contact, b2ContactImpulse *impulse);
};

class TumblerController : private Timer {
public:
	TumblerController(TumblerPlugin *plugin);
	~TumblerController();

	void addNote(int midi_note);
	void queueNote(int midi_note);
	void timerCallback();
	void noteOffCallback(TumblerNote *note);
	void playNote(TumblerNote *note, int velocity);
private:
	b2World world;
	b2Body *tumbler;
	TumblerPlugin *plugin;
	TumblerNote notes[MAX_NOTES];

	int currentNote;
	int currentQueueSize;
	int noteQueue[NOTE_QUEUE_LENGTH];
	TumblerContactListener listener;
};

#endif