//	TumblerPlugin.cpp - Definition of the plugin class.
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

#include "TumblerPlugin.h"

using namespace std;



//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
TumblerPlugin::TumblerPlugin(audioMasterCallback audioMaster):
AudioEffectX(audioMaster, numPrograms, numParameters),
programs(0),
samplerate(44100.0f),
tempo(120.0f),
tempEvents(0),
numEvents(0),
numPendingEvents(0),
frames(0),
effectName("Tumbler"),
vendorName("Hannah Roat (aetheria.co.uk, github.com/nornalbion)")
{
	int i;

	//Initialise parameters.
	parameters[0] = 0.0f;
	parameters[1] = 0.0f;
	parameters[2] = 1.0f;
	parameters[3] = 1.0f;


	//Setup event queue.
	for(i=0;i<maxNumEvents;++i)
	{
		midiEvent[i] = new VstMidiEvent;

		midiEvent[i]->type = 1;
		midiEvent[i]->midiData[3] = 0;
		midiEvent[i]->reserved1 = 99;
		midiEvent[i]->deltaFrames = -99;
		midiEvent[i]->noteLength = 0;
		midiEvent[i]->noteOffset = 0;

		eventNumArray[i] = -1;
	}
	tempEvents = new VstEvents;
	tempEvents->numEvents = 1;
	tempEvents->events[0] = (VstEvent *)midiEvent[0];

	//Setup programs here.
	programs = new TumblerProgram[numPrograms];
	setProgram(0);

	//Set various constants.
    setNumInputs(0);
    setNumOutputs(2);
    canProcessReplacing(true);
    isSynth(true);
    setUniqueID('AETm');

	//Construct editor here.
	editor = new TumblerEditor(this);

	this->controller = new TumblerController(this);
}

//----------------------------------------------------------------------------
TumblerPlugin::~TumblerPlugin()
{
	int i;

	//Delete event queue.
	for(i=0;i<maxNumEvents;++i)
	{
		if(midiEvent[i])
			delete midiEvent[i];
	}
	if(tempEvents)
		delete tempEvents;

	//Delete programs.
	if(programs)
		delete [] programs;

	//Cleanup everything else.
}

//----------------------------------------------------------------------------
void TumblerPlugin::process(float **inputs, float **outputs, VstInt32 sampleFrames)
{
	VstInt32 i;

	frames = sampleFrames;

	//Calculate audio.
	for(i=0;i<frames;++i)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

	}
	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//----------------------------------------------------------------------------
void TumblerPlugin::processReplacing(float **inputs,
								 float **outputs,
								 VstInt32 sampleFrames)
{
	VstInt32 i;

	frames = sampleFrames;

	//Calculate audio.
	for(i=0;i<frames;++i)
	{
		//Process MIDI events,if there are any.
		if(numEvents>0)
			processMIDI(i);

	}
	//If there are events remaining in the queue, update their delta values.
	if(numPendingEvents > 0)
	{
		for(i=0;i<numPendingEvents;++i)
			midiEvent[eventNumArray[i]]->deltaFrames -= frames;
	}
}

//----------------------------------------------------------------------------
VstInt32 TumblerPlugin::processEvents(VstEvents *events)
{
	AudioEffectX::processEvents(events);
	int i, j, k;
	VstMidiEvent *event;

	//Go through each event in events.
	for(i=0;i<events->numEvents;++i)
	{
		//Only interested in MIDI events.
		if(events->events[i]->type == kVstMidiType)
		{
			event = (VstMidiEvent *)events->events[i];
			j = -1;

			//Find a space for it in the midiEvent queue.
			for(k=1;k<maxNumEvents;++k)
			{
				if(midiEvent[k]->deltaFrames == -99)
				{
					eventNumArray[numPendingEvents] = k;
					++numPendingEvents;

					j = k;
					break;
				}
			}
			//Add it to the queue if there's still room.
			if((j > 0)&&(numEvents < maxNumEvents))
			{
				numEvents++;
				midiEvent[j]->midiData[0] =		event->midiData[0];
				midiEvent[j]->midiData[1] =		event->midiData[1];
				midiEvent[j]->midiData[2] =		event->midiData[2];
				midiEvent[j]->midiData[3] =		event->midiData[3];
				midiEvent[j]->type =			event->type;
				midiEvent[j]->byteSize =		event->byteSize;
				midiEvent[j]->deltaFrames =		event->deltaFrames;
				midiEvent[j]->flags =			event->flags;
				midiEvent[j]->noteLength =		event->noteLength;
				midiEvent[j]->noteOffset =		event->noteOffset;
				midiEvent[j]->detune =			event->detune;
				midiEvent[j]->noteOffVelocity = event->noteOffVelocity;
				midiEvent[j]->reserved1 =		99;
				midiEvent[j]->reserved2 =		event->reserved2;
			}
		}
	}

	return 1;
}

//----------------------------------------------------------------------------
void TumblerPlugin::resume()
{
	//Let old hosts know we want to receive MIDI events.
    AudioEffectX::resume();
}

//----------------------------------------------------------------------------
void TumblerPlugin::suspend()
{
    
}

//----------------------------------------------------------------------------
void TumblerPlugin::setProgram(VstInt32 program)
{
	int i;

	curProgram = program;

	setParameter(0, programs[curProgram].rotationspeed);
	setParameter(1, programs[curProgram].openness);
	setParameter(2, programs[curProgram].gravity);
	setParameter(3, programs[curProgram].restitution);
}

//----------------------------------------------------------------------------
void TumblerPlugin::setProgramName(char *name)
{
	programs[curProgram].name = name;
}


//----------------------------------------------------------------------------
void TumblerPlugin::getProgramName(char *name)
{
	strcpy(name, programs[curProgram].name.c_str());
}

//----------------------------------------------------------------------------
bool TumblerPlugin::getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text)
{
	bool retval = false;

    if(index < numPrograms)
    {
		strcpy(text, programs[index].name.c_str());
		retval = true;
    }

	return retval;
}

//----------------------------------------------------------------------------
bool TumblerPlugin::copyProgram(VstInt32 destination)
{
	bool retval = false;

    if(destination < numPrograms)
    {
		programs[destination] = programs[curProgram];
        retval = true;
    }

    return retval;
}

//----------------------------------------------------------------------------
void TumblerPlugin::setParameter(VstInt32 index, float value)
{
	parameters[index] = value;
	switch(index) {
	case ROTATIONSPEED:
		programs[curProgram].rotationspeed = parameters[index];
		break;
	case OPENNESS:
		programs[curProgram].openness = parameters[index];
		break;
	case GRAVITY:
		programs[curProgram].gravity = parameters[index];
		break;
	case RESTITUTION:
		programs[curProgram].restitution = parameters[index];
		break;
	}

	if(editor)
	{
		((VSTGLEditor *)editor)->setParameter(index, value);
	}
}

//----------------------------------------------------------------------------
float TumblerPlugin::getParameter(VstInt32 index)
{
	return parameters[index];
}

//----------------------------------------------------------------------------
void TumblerPlugin::getParameterLabel(VstInt32 index, char *label)
{
	if(index == ROTATIONSPEED) {
		strcpy(label, "Rotation speed");
	} else if(index == OPENNESS) {
		strcpy(label, "Openness");
	} else if(index == GRAVITY) {
		strcpy(label, "Gravity");
	} else if(index == RESTITUTION) {
		strcpy(label, "Restitution");
	}

	strcpy(label, " ");
}

//----------------------------------------------------------------------------
void TumblerPlugin::getParameterDisplay(VstInt32 index, char *text)
{
	float p = parameters[index];
	char temp[16] = "";
	switch(index) {
		case ROTATIONSPEED:
			sprintf(temp, "%d deg/s",(int)(360*p));
			break;
		case OPENNESS:
			sprintf(temp, "%d", (int)(p*127));
			break;
		case GRAVITY:
			sprintf(temp, "%.1f m/s", p*60);
			break;
		case RESTITUTION:
			sprintf(temp, "%.2f", p*0.95);
			break;
	}
	strcpy(text, temp);
}

//----------------------------------------------------------------------------
void TumblerPlugin::getParameterName(VstInt32 index, char *label)
{
	if(index == ROTATIONSPEED) {
		strcpy(label, "Rotation speed");
	} else if(index == OPENNESS) {
		strcpy(label, "Openness");
	} else if(index == GRAVITY) {
		strcpy(label, "Gravity");
	} else if(index == RESTITUTION) {
		strcpy(label, "Restitution");
	}
}

//----------------------------------------------------------------------------
VstInt32 TumblerPlugin::canDo(char *text)
{
	if(!strcmp(text, "sendVstEvents")) 
		return 1;
	if(!strcmp(text, "sendVstMidiEvent")) 
		return 1; 
	if(!strcmp(text, "receiveVstEvents")) 
		return 1;
	if(!strcmp(text, "receiveVstMidiEvent")) 
		return 1;
	if(!strcmp(text, "receiveVstTimeInfo")) 
		return 1;

	return -1;
}

//----------------------------------------------------------------------------
float TumblerPlugin::getVu()
{
	return 0.0f;
}

//----------------------------------------------------------------------------
bool TumblerPlugin::getEffectName(char* name)
{
    strcpy(name, effectName.c_str());

    return true;
}

//----------------------------------------------------------------------------
bool TumblerPlugin::getVendorString(char* text)
{
    strcpy(text, vendorName.c_str());

    return true;
}

//----------------------------------------------------------------------------
bool TumblerPlugin::getProductString(char* text)
{
    strcpy(text, effectName.c_str());

    return true;
}

//----------------------------------------------------------------------------
VstInt32 TumblerPlugin::getVendorVersion()
{
    return versionNumber;
}

//----------------------------------------------------------------------------
VstPlugCategory TumblerPlugin::getPlugCategory()
{
    return(kPlugCategEffect);
}

//----------------------------------------------------------------------------
bool TumblerPlugin::getInputProperties(VstInt32 index, VstPinProperties* properties)
{
	if(index == 0)
	{
		sprintf(properties->label, "%s Left Input 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		return true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Input 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------
bool TumblerPlugin::getOutputProperties(VstInt32 index, VstPinProperties* properties)
{

	if(index == 0)
	{
		sprintf(properties->label, "%s Left Output 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		return true;
	}
	else if(index == 1)
	{
		sprintf(properties->label, "%s Right Output 1", effectName.c_str());
		properties->flags = kVstPinIsStereo|kVstPinIsActive;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------------
VstInt32 TumblerPlugin::getGetTailSize()
{
	return 1; //1=no tail, 0=don't know, everything else=tail size
}

//----------------------------------------------------------------------------
void TumblerPlugin::processMIDI(VstInt32 pos)
{
	int data1, data2;
	int status, ch, delta;

	int i, j;

	for(i=0;i<numPendingEvents;++i)
	{
		if((midiEvent[eventNumArray[i]]->deltaFrames%frames) == pos)
		{
			//--pass on/act on event--
			delta = 0; //because we're at pos frames into the buffer...
			ch = (midiEvent[eventNumArray[i]]->midiData[0] & 0x0F);
			status = (midiEvent[eventNumArray[i]]->midiData[0] & 0xF0);
			data1 = (midiEvent[eventNumArray[i]]->midiData[1] & 0x7F);
			data2 = (midiEvent[eventNumArray[i]]->midiData[2] & 0x7F);

			switch(status)
			{
				case 0x90: // note on
					if(data2 > 0) // velocity > 0
						//shouldn't do this here..shush.
						this->controller->queueNote(data1 | (data2 << 8));
					else
						// ignore dat shit
					break;
				case 0x80: // note off
						// and dat shit
					break;
			}
			midiEvent[eventNumArray[i]]->deltaFrames = -99;
			--numEvents;

			//--reset EventNumArray--
			for(j=(i+1);j<numPendingEvents;++j)
				eventNumArray[(j-1)] = eventNumArray[j];
			--numPendingEvents;
			//break;
		}
	}
}

//----------------------------------------------------------------------------
void TumblerPlugin::MIDI_NoteOn(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x90 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}

//----------------------------------------------------------------------------
void TumblerPlugin::MIDI_NoteOff(int ch, int note, int val, int delta)
{
	midiEvent[0]->midiData[0] = 0x80 + ch;
	midiEvent[0]->midiData[1] = note;
	midiEvent[0]->midiData[2] = val;
	midiEvent[0]->deltaFrames = delta;
	sendVstEventsToHost(tempEvents);
}