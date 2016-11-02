//	TumblerEditor.cpp - Simple OpenGL editor window.
//	--------------------------------------------------------------------------
//	Copyright (c) 2005-2006 Niall Moody
//	
//	Permission is hereby granted, free of charge, to any person obtaining a
//	copy of this software and associated documentation files (the "Software"),
//	to deal in the Software without restriction, including without limitation
//	the rights to use, copy, modify, merge, publish, distribute, sublicense,
//	and/or sell copies of the Software, and to permit persons to whom the
//	Software is furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//	THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//	DEALINGS IN THE SOFTWARE.
//	--------------------------------------------------------------------------
#include "TumblerEditor.h"

//----------------------------------------------------------------------------
TumblerEditor::TumblerEditor(AudioEffect *effect):
VSTGLEditor(effect, Antialias4x),
Timer(30) //30ms == ~30fps?
{
	this->notes = NULL;
	this->tumbler = NULL;
	this->notes_count = -1;
	//Set the opengl context's size - This must be called here!
    setRect(0, 0, TUMBLEREDITOR_WIDTH, TUMBLEREDITOR_HEIGHT);
}

//----------------------------------------------------------------------------
TumblerEditor::~TumblerEditor()
{
    delete this->baseJoint;
}

//----------------------------------------------------------------------------
void TumblerEditor::guiOpen()
{
	//Setup OpenGL stuff.
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glViewport(0, 0, std::move(*this).getWidth(), std::move(*this).getHeight());

    //Position viewer.
    glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2.0f);

	//Start timer to constantly update gui.
	start();
}

//----------------------------------------------------------------------------
void TumblerEditor::guiClose()
{
	//Stop the timer.
	stop();
}

void TumblerEditor::setNotesAndTumbler(b2Body *tumbler, b2RevoluteJoint *baseJoint, TumblerNote *notes, int notes_count) {
	if(this->notes == NULL) {
		this->tumbler = tumbler;
		this->baseJoint = baseJoint;
		this->notes = notes;
		this->notes_count = notes_count;
	}
	
}

void TumblerEditor::drawTumbler() {
	glColor3f(0.2f,0.9f, 1.0f);
	if(this->tumbler != NULL) {
		
		glPushMatrix();
		glLoadIdentity();
		float angle;
		//float angle = (float)(this->baseJoint->GetJointAngle()*(1/DEG_TO_RAD(1)));
		//glRotatef(angle,0.0f,0.0f,1.0f);
		
		b2RevoluteJoint *joint = (b2RevoluteJoint *)this->tumbler->GetJointList()->joint->GetNext();
		b2RevoluteJoint *firstJoint = joint;
		while(joint != NULL) {
			glPushMatrix();

				b2Body *body = joint->GetBodyB();
				
				b2Vec2 pos = body->GetPosition();
				pos *= PHYS2GL_SCALING_FACTOR;
				glTranslatef(pos.x,pos.y,0);

				angle = body->GetAngle();
				angle *= 1/DEG_TO_RAD(1);
				glRotatef(angle, 0.0f, 0.0f, 1.0f);
				
				
				
				b2PolygonShape *shape = (b2PolygonShape*)body->GetFixtureList()->GetShape();
				
				glBegin(GL_POLYGON);
				for(int v = 0; v < shape->GetVertexCount(); v++) {
					b2Vec2 pos = shape->GetVertex(v);
					pos *= PHYS2GL_SCALING_FACTOR;
					glVertex2f(pos.x,pos.y);
				}
				glEnd();

			glPopMatrix();
			
			joint = (b2RevoluteJoint *)joint->GetNext();
			if(joint == firstJoint) {
				break;
			}
		}

		glPopMatrix();
	}
}

void TumblerEditor::drawNotes() {
	glPushMatrix();
	glLoadIdentity();
	glPointSize(2.0f);
	if(this->notes != NULL) {
			for(int i = 0; i < this->notes_count ; i ++) {
				
				if(this->notes[i].IsInitialised()) {
					rgbColour colour = this->notes[i].GetColour();
					glColor3f((float)colour.r, (float)colour.g, (float)colour.b);

					b2CircleShape *shape = (b2CircleShape *)this->notes[i].GetBody()->GetFixtureList()->GetShape();
					

					glBegin(GL_POLYGON);
					
					b2Vec2 centrePos = notes[i].GetPosition();
					centrePos *= PHYS2GL_SCALING_FACTOR;
					for(int j = 0; j < 16; j++) {
						glVertex2f((float)(centrePos.x + shape->m_radius * PHYS2GL_SCALING_FACTOR* sin(j*DEG_TO_RAD(360/16))),
								(float)(centrePos.y - shape->m_radius * PHYS2GL_SCALING_FACTOR * cos(j*DEG_TO_RAD(360/16))));
					}
					glEnd();
				}
			}
		
	}
	glPopMatrix();
}

void TumblerEditor::draw() {
	glClear(GL_COLOR_BUFFER_BIT);
	this->drawTumbler();
	this->drawNotes();
}

//----------------------------------------------------------------------------
void TumblerEditor::timerCallback()
{
	refreshGraphics();
}
