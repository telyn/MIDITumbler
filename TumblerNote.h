#ifndef TUMBLERNOTE_H
#define TUMBLERNOTE_H
#include "Box2D/Box2D.h"
#include "rgbhsv.h"

class TumblerNote {
public:
	TumblerNote();
	~TumblerNote();
	b2Vec2 GetPosition();
	b2Body *GetBody();
	rgbColour GetColour();
	int GetNote();
	void Set(int note, b2Body *body);
	void UnSet();
	bool IsInitialised();
	static void Seed();
private:
	int note;
	rgbColour colour;
	b2Body * body;
};

#endif