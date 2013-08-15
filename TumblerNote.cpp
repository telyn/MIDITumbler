#include "TumblerNote.h"
#include <time.h>
rgbColour makeRandomNoteColour();

TumblerNote::TumblerNote() {
	this->note = -1;
	//randomly assign colour
	this->colour = makeRandomNoteColour();
	this->body = NULL;
}

TumblerNote::~TumblerNote() {
	this->body->GetWorld()->DestroyBody(body);
	body = NULL;
}

b2Vec2 TumblerNote::GetPosition() {
	return this->body->GetPosition();
}

b2Body *TumblerNote::GetBody() {
	return this->body;
}

int TumblerNote::GetNote() {
	return this->note;
}

rgbColour TumblerNote::GetColour() {
	return this->colour;
}

void TumblerNote::Set(int note, b2Body *body) {
	if(this->body != NULL) {
		this->body->GetWorld()->DestroyBody(this->body);
	}
	this->body = body;
	this->body->SetUserData(this);
	this->note = note;
	//if you get a new body, better be a new colour!
	this->colour = makeRandomNoteColour();
}

void TumblerNote::UnSet() {
	this->note = 0;
	this->body->GetWorld()->DestroyBody(this->body);
	this->body = NULL;
}

void TumblerNote::Seed() {
	srand(time(NULL));
}

bool TumblerNote::IsInitialised() {
	return this->body != NULL;
}

rgbColour makeRandomNoteColour() {
	hsvColour colour;
	colour.h = rand() % 360;
	colour.s = 1.0f;
	colour.v = 1.0f;
	return hsv2rgb(colour);
	
}