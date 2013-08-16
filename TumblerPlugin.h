//	TumblerPlugin.h - Declaration of the plugin class.
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

// parameters
// [0] = rotation speed
// [1] = openness
// [2] = gravity
// [3] = restitution

#ifndef TUMBLERPLUGIN_H_
#define TUMBLERPLUGIN_H_
class TumblerPlugin;

#include "audioeffectx.h"
#include <string>
#include "TumblerController.h"
#include "TumblerEditor.h"

struct TumblerProgram;

//Trick to ensure inline functions get inlined properly.
#ifdef WIN32
#define strictinline __forceinline
#elif defined (__GNUC__)
#define strictinline inline __attribute__((always_inline))
#else
#define strictinline inline
#endif

//----------------------------------------------------------------------------
///	VST plugin class.
class TumblerPlugin : public AudioEffectX
{
public:
	///	Constructor.
	/*!
		\param audioMaster This is a callback function used by AudioEffect to
		communicate with the host.  Subclasses of AudioEffect/AudioEffectX
		should not ever need to make use of it directly.
	 */
	TumblerPlugin(audioMasterCallback audioMaster);
	///	Destructor.
	~TumblerPlugin();

	///	Processes a block of audio, accumulating.
	/*!
		\param inputs Pointer to an array of an array of audio samples
		containing  the audio input to the plugin (well, it won't necessarily
		be an actual, contiguous array, but it's simpler to view it that way).
		\param outputs Pointer to an array of an array of audio samples which
		will contain the audio output of the plugin (again, won't necessarily
		be a contiguous array...).
		\param sampleFrames The number of samples to process for this block.
	 */
	void process(float **inputs, float **outputs, VstInt32 sampleFrames);
	///	Processes a block of audio, replacing.
	/*!
		\sa process()
	 */
	void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
	///	Called by the host to inform the plugin of certain events.
	/*!
		\param events Pointer to the events.

		The main events will be MIDI messages.
	 */
	VstInt32 processEvents(VstEvents* events);
	///	Called when audio processing begins.
	/*!
		Use this to set up buffers etc.
	 */
	void resume();
	///	Called when audio processing stops.
	void suspend();

	///	Called to set the plugin's current program.
	/*!
		\param program Index of the program to make current.
	 */
	void setProgram(VstInt32 program);
	///	Called to set the name of the current program.
	void setProgramName(char *name);
	///	Returns the name of the current program.
	void getProgramName(char *name);
	///	Returns the name of the current program.
	/*!
		\param category If your programs are arranged into categories?
		\param index Index of the program whose name the host wants to query.
		\param text String to put the return name into.

		\return True if successful, false otherwise?

		The aim of this method is to allow hosts to get the names for each
		program in a plugin, without having to call setProgram() first.
	 */
	bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char* text);
	///	Copies the current program to the destination program.
	bool copyProgram(VstInt32 destination);

	///	Called to set the plugin's parameters.
	void setParameter(VstInt32 index, float value);
	///	Returns the value of the indexed parameter.
	float getParameter(VstInt32 index);
	///	Returns the label (units) of the indexed parameter.
	void getParameterLabel(VstInt32 index, char *label);
	///	Returns the value of the indexed parameter as text.
	void getParameterDisplay(VstInt32 index, char *text);
	///	Returns the name of the indexed parameter.
	void getParameterName(VstInt32 index, char *text);

	///	Called by the host to determine the plugin's capabilities.
	/*!
		\return 1=can do, -1=cannot do, 0=don't know.
	 */
	VstInt32 canDo(char* text);
	///	Returns the value of the plugin's VU meter?
	float getVu();
	///	Returns the name of the plugin.
	bool getEffectName(char* name);
	///	Returns the name of the plugin author.
	bool getVendorString(char* text);
	///	Returns the name of the plugin.
	bool getProductString(char* text);
	///	Returns the plugin's version.
	VstInt32 getVendorVersion();
	///	Returns the plugin's category.
	VstPlugCategory getPlugCategory();
	///	Returns certain information about the plugin's inputs.
	/*!
		This is used to assign names to the plugin's inputs.
	 */
	bool getInputProperties(VstInt32 index, VstPinProperties* properties);
	///	Returns certain information about the plugin's outputs.
	/*!
		\sa getInputProperties()
	 */
	bool getOutputProperties(VstInt32 index, VstPinProperties* properties);
	///	Stupid name method...
	VstInt32 getGetTailSize();

	//------------------------------------------
	///	Enum for enumerating the plugin's parameters.
	enum
	{
		ROTATIONSPEED,
		OPENNESS,
		GRAVITY,
		RESTITUTION,
		numParameters = 4
	};
	///	Called when a MIDI Note On message is received.
	void MIDI_NoteOn(int ch, int note, int val, int delta);
	///	Called when a MIDI Note Off message is received.
	void MIDI_NoteOff(int ch, int note, int val, int delta);
  private:
	///	Called for every sample, to dispatch MIDI events appropriately.
	strictinline void processMIDI(VstInt32 pos);

	//-----------------------------------------
	///	Various constants.
	enum
	{
		numPrograms = 1,	///<Number of programs this plugin has.
		versionNumber = 110 ,///<The current version of the plugin.
		maxNumEvents = 250	///<The maximum number of events in our MIDI queue.
	};

	//------------------------------------------
	///	Array of our plugin's programs.
    TumblerProgram *programs;

	///	The current samplerate.
	float samplerate;
	///	The current tempo.
	float tempo;

	///	Array of our plugin's parameters.
	float parameters[numParameters];

	//------------------------------------------
	//MIDI stuff
	///	VstEvents pointer for outputting events to the host.
	VstEvents *tempEvents;
	///	Our MIDI event queue.
	VstMidiEvent *midiEvent[maxNumEvents];
	///	The number of events in our event queue.
	int numEvents;

	///	Sorted array holding the indices of pending events (in midiEvent), for speed.
	int eventNumArray[maxNumEvents];
	///	Number of events in eventNumArray (same as numEvents?).
	int numPendingEvents;

	///	Number of samples in the current processing block.
	VstInt32 frames;
	//-----------------------------------------

	///	Name of the plugin.
	std::string effectName;
	///	Name of the plugin's author.
    std::string vendorName;

	TumblerController *controller;
};

//---------------------------------------------------------------------------
///	Simple struct representing a program of the plugin.
struct TumblerProgram
{
  public:
	
	TumblerProgram() : name("(empty)")
	{
		rotationspeed = 0.0f;
		openness = 0.0f;
		gravity = 0.5f;
		restitution = 1.0f;
	};

	///	The parameters of the plugin.
	float rotationspeed;
	float openness;
	float gravity;
	float restitution;

	///	The name of this program.
	std::string name;
};

#endif
