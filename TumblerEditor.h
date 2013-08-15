//	ExampleEditor.h - Simple OpenGL editor window.
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

#ifndef TUMBLEREDITOR_H_
#define TUMBLEREDITOR_H_
#define DEG_TO_RAD( x ) ( (x) * 0.0174532925f )

#define PHYS2GL_SCALING_FACTOR 0.08f
#include "VSTGLEditor.h"
#include "VSTGLTimer.h"
#include "Box2D/Box2D.h"
#include "TumblerNote.h"

///	Simple VSTGL example.
class TumblerEditor : public VSTGLEditor,
					  public Timer
{
  public:
	///	Constructor.
	TumblerEditor(AudioEffect *effect);
	///	Destructor.
	~TumblerEditor();

	// for converting from box2D coords to gl coords
	static const int WIDTH = 400;
	static const int HEIGHT = 300;

	///	Called when the Gui's window is opened.
	void guiOpen();
	///	Called when the Gui's window is closed.
	void guiClose();

	void setNotesAndTumbler(b2Body *tumbler, b2RevoluteJoint *baseJoint, TumblerNote *notes, int notes_count);

	void drawTumbler();
	void drawNotes();
	void draw();

	///	Called repeatedly, to update the graphics.
	void timerCallback();

private:
	b2Body *tumbler;
	b2RevoluteJoint *baseJoint;
	TumblerNote *notes; // array of notes stored elsewhere :-)
	int notes_count;
};

#endif
