/*
 *  $Id: common.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 11/02/06.
 *  Modified by Nathan Sturtevant on 02/29/20.
 *
 * This file is part of HOG2. See https://github.com/nathansttt/hog2 for licensing information.
 *
 */ 

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <cassert>

#include "GLUtil.h"
#include "Trackball.h"
#include "Common.h"



// For printing debug info
//static bool const verbose = false;

static unsigned long gNextWindowID = 0;
char gDefaultMap[1024] = "";
const recVec gOrigin(0.0, 0.0, 0.0);

using namespace std;

static std::vector<commandLineCallbackData *> commandLineCallbacks;
static std::vector<joystickCallbackData *> joystickCallbacks;
static std::vector<mouseCallbackData *> mouseCallbacks;
static std::vector<mouseCallbackData2 *> mouseCallbacks2;
static std::vector<windowCallbackData *> windowCallbacks;
static std::vector<frameCallbackData *> glDrawCallbacks;
//static std::vector<dataCallbackData> dataCallbacks;

static keyboardCallbackData *keyboardCallbacks[256] = 
{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

const char *getModifierText(tKeyboardModifier t)
{
	switch (t)
	{
		case kNoModifier: return "";
		case kAnyModifier: return "*-";
		case kShiftDown: return "Sft-";
		case kControlDown: return "Ctl-";
		case kAltDown: return "Alt-";
		default: return "?-";
	}
}

void InstallKeyboardHandler(KeyboardCallback kf, const char *title, const char *description,
														tKeyboardModifier mod, unsigned char firstKey, unsigned char lastKey)
{
	for (int x = firstKey; ; x++)
	{
		keyboardCallbacks[x] = new keyboardCallbackData(kf, title, description, mod, keyboardCallbacks[x]);
		if (x >= lastKey)
			break;
	}
}

void GetKeyAssignments(std::vector<char> &keys)
{
	for (int x = 0; x < 256; x++)
	{
		for (keyboardCallbackData *kd = keyboardCallbacks[x]; kd; kd = kd->next)
		{
			char val = x;
			keys.push_back(val);
//			printf("%s%c\t%s\t%s\n", getModifierText(kd->mod), x, kd->title, kd->desc);
		}
	}
}

void GetKeyAssignmentDescriptions(std::vector<std::string> &keys)
{
	for (int x = 0; x < 256; x++)
	{
		for (keyboardCallbackData *kd = keyboardCallbacks[x]; kd; kd = kd->next)
		{
			//char val = x;
			keys.push_back(kd->title);
			//			printf("%s%c\t%s\t%s\n", getModifierText(kd->mod), x, kd->title, kd->desc);
		}
	}
}
void PrintKeyboardAssignments()
{
	printf("Legal Keyboard Commands\n-----------------------\n");
	for (int x = 0; x < 256; x++)
	{
		for (keyboardCallbackData *kd = keyboardCallbacks[x]; kd; kd = kd->next)
		{
			printf("%s%c\t%s\t%s\n", getModifierText(kd->mod), x, kd->title, kd->desc);
		}
	}
}


bool DoKeyboardCallbacks(pRecContext pContextInfo, unsigned char keyHit, tKeyboardModifier mod)
{
	bool called = false;
	for (keyboardCallbackData *kd = keyboardCallbacks[keyHit]; kd; kd = kd->next)
	{
		if ((mod == kd->mod) || (kd->mod == kAnyModifier))
		{
//			printf("(DEBUG) calling: %s%c\t%s\t%s\n", getModifierText(kd->mod), keyHit,
//						 kd->title, kd->desc);
			kd->call(pContextInfo->windowID, mod, keyHit);
			called = true;
		}
	}
	if (!called)
	{
		printf("Unknown keyboard command %s%c/%d\n\n", getModifierText(mod), keyHit, keyHit);
		PrintKeyboardAssignments();
	}
	return false;
}

void InstallFrameHandler(FrameCallback glCall, unsigned long windowID, void *userdata)
{
	glDrawCallbacks.push_back(new frameCallbackData(glCall, windowID, userdata));	
}

void RemoveFrameHandler(FrameCallback glCall, unsigned long windowID, void *userdata)
{
	for (unsigned int x = 0; x < glDrawCallbacks.size(); x++)
	{
		if ((glDrawCallbacks[x]->glCall == glCall) &&
				(glDrawCallbacks[x]->userData == userdata) &&
				(glDrawCallbacks[x]->windowID == windowID))
		{
			delete glDrawCallbacks[x];
			glDrawCallbacks[x] = glDrawCallbacks[glDrawCallbacks.size()-1];
			glDrawCallbacks.pop_back();
		}
	}
}

void HandleFrame(pRecContext pContextInfo, int viewport)
{
	glEnable(GL_BLEND); // for text fading
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ditto
	for (unsigned int x = 0; x < glDrawCallbacks.size(); x++)
	{
		if (glDrawCallbacks[x]->windowID == pContextInfo->windowID)
			glDrawCallbacks[x]->glCall(pContextInfo->windowID, viewport, glDrawCallbacks[x]->userData);
	}
	pContextInfo->display.EndFrame(); // end last frame
	for (int x = 0; x < pContextInfo->numPorts; x++)
	{
		pContextInfo->viewports[x].bounds.lerp(pContextInfo->viewports[x].finalBound, 0.1);
	}
}

void InstallJoystickHandler(JoystickCallback jC, void *userdata)
{
	joystickCallbacks.push_back(new joystickCallbackData(jC, userdata));	
}

void RemoveJoystickHandler(JoystickCallback jC, void *userdata)
{
	for (unsigned int x = 0; x < joystickCallbacks.size(); x++)
	{
		if ((joystickCallbacks[x]->jC == jC) &&
				(joystickCallbacks[x]->userData == userdata))
		{
			delete joystickCallbacks[x];
			joystickCallbacks[x] = joystickCallbacks[joystickCallbacks.size()-1];
			joystickCallbacks.pop_back();
		}
	}
}

void HandleJoystickMovement(pRecContext pContextInfo, double panX, double panY)
{
	for (unsigned int x = 0; x < joystickCallbacks.size(); x++)
	{
		//printf("Calling joystick callback %d\n", x);
		joystickCallbacks[x]->jC(pContextInfo->windowID, panX, panY, joystickCallbacks[x]->userData);
	}
}

void InstallCommandLineHandler(CommandLineCallback CLC,
															 const char *arg, const char *param, const char *usage)
{
	commandLineCallbacks.push_back(new commandLineCallbackData(CLC, arg, param, usage));
}

void PrintCommandLineArguments()
{
	printf("Valid command-line flags:\n\n");
	for (unsigned int x = 0; x < commandLineCallbacks.size(); x++)
		commandLineCallbacks[x]->Print();
	printf("\n");
}

// Process command line arguments
void ProcessCommandLineArgs(int argc, char *argv[])
{
//	printf("Processing %d command line arguments\n", argc);
//	for (int x = 0; x < argc; x++)
//	{
//		printf("%s ", argv[x]);
//	}
//	printf("\n");
	//initializeCommandLineHandlers();
	// printCommandLineArguments();

	int lastval = 1;
	for (int y = 1; y < argc; )
	{
		lastval = y;
		for (unsigned int x = 0; x < commandLineCallbacks.size(); x++)
		{
			if (strcmp(commandLineCallbacks[x]->argument, argv[y]) == 0)
			{
				y += commandLineCallbacks[x]->CLC(&argv[y], argc-y);
				break;
			}
		}
		if (lastval == y)
		{
			printf("Error: unhandled command-line parameter (%s)\n\n", argv[y]);
			PrintCommandLineArguments();
			y++;
			//exit(10);
		}
	}
}

void InstallMouseClickHandler(MouseCallback mC, tMouseEventType which)
{
	mouseCallbacks.push_back(new mouseCallbackData(mC, which));
}

void InstallMouseClickHandler(MouseCallback2 mC, tMouseEventType which)
{
	mouseCallbacks2.push_back(new mouseCallbackData2(mC, which));
}

void RemoveMouseClickHandler(MouseCallback mC)
{
	for (unsigned int x = 0; x < mouseCallbacks.size(); x++)
	{
		if (mouseCallbacks[x]->mC == mC)
		{
			delete mouseCallbacks[x];
			mouseCallbacks[x] = mouseCallbacks[mouseCallbacks.size()-1];
			mouseCallbacks.pop_back();
		}
	}
}

void RemoveMouseClickHandler(MouseCallback2 mC)
{
	for (unsigned int x = 0; x < mouseCallbacks2.size(); x++)
	{
		if (mouseCallbacks2[x]->mC == mC)
		{
			delete mouseCallbacks2[x];
			mouseCallbacks2[x] = mouseCallbacks2[mouseCallbacks2.size()-1];
			mouseCallbacks2.pop_back();
		}
	}
}

void ReinitViewports(unsigned long windowID, const Graphics::rect &r, viewportType v)
{
	pRecContext pContextInfo = GetContext(windowID);
	pContextInfo->numPorts = 1;
	pContextInfo->viewports[0].bounds = r;
	pContextInfo->viewports[0].finalBound = r;
	pContextInfo->viewports[0].type = v;
	pContextInfo->viewports[0].active = true;
	for (int x = 1; x < MAXPORTS; x++)
		pContextInfo->viewports[x].active = false;
}

/* Adds a new viewport to the existing viewports and
 * returns the new viewport numbers
 */
int AddViewport(unsigned long windowID, const Graphics::rect &r, viewportType v)
{
	pRecContext pContextInfo = GetContext(windowID);
	if (pContextInfo->numPorts >= MAXPORTS)
	{
		printf("Cannot add viewport - reached limit of %d [constant MAXPORTS]\n", MAXPORTS);
		return -1;
	}
	pContextInfo->numPorts++;
	pContextInfo->viewports[pContextInfo->numPorts-1].bounds = r;
	pContextInfo->viewports[pContextInfo->numPorts-1].finalBound = r;
	pContextInfo->viewports[pContextInfo->numPorts-1].type = v;
	pContextInfo->viewports[pContextInfo->numPorts-1].active = true;
	return pContextInfo->numPorts-1;
}

/* Adds a new viewport to the existing viewports and
 * returns the new viewport numbers. Will animate from initial to final location
 */
int AddViewport(unsigned long windowID, const Graphics::rect &initial, const Graphics::rect &fin, viewportType v)
{
	pRecContext pContextInfo = GetContext(windowID);
	if (pContextInfo->numPorts >= MAXPORTS)
	{
		printf("Cannot add viewport - reached limit of %d [constant MAXPORTS]\n", MAXPORTS);
		return -1;
	}
	pContextInfo->numPorts++;
	pContextInfo->viewports[pContextInfo->numPorts-1].bounds = initial;
	pContextInfo->viewports[pContextInfo->numPorts-1].finalBound = fin;
	pContextInfo->viewports[pContextInfo->numPorts-1].type = v;
	pContextInfo->viewports[pContextInfo->numPorts-1].active = true;
	return pContextInfo->numPorts-1;
}

void MoveViewport(unsigned long windowID, int port, const Graphics::rect &newLocation)
{
	pRecContext pContextInfo = GetContext(windowID);
	pContextInfo->viewports[port].finalBound = newLocation;
}

float GlobalHOGToViewportX(float x, int v)
{
	pRecContext pContextInfo = getCurrentContext();
	Graphics::point input(x, 0.f, 0.f);
	Graphics::point input2(0, 0.f, 0.f);
	Graphics::point result = ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[v], input);
	Graphics::point result2 = ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[v], input2);
	return result.x-result2.x;
	//	return ((result.x+1.0)*pContextInfo->windowWidth)/2.0;
}

float ViewportToGlobalHOGX(float x, int v)
{
	pRecContext pContextInfo = getCurrentContext();
	Graphics::point input(x, 0.f, 0.f);
	Graphics::point input2(0, 0.f, 0.f);
	Graphics::point result = ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[v], input);
	Graphics::point result2 = ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[v], input2);
	return result.x-result2.x;
	//	return ((result.x+1.0)*pContextInfo->windowWidth)/2.0;
}

float GlobalHOGToViewportY(float y, int v)
{
	pRecContext pContextInfo = getCurrentContext();
	Graphics::point input(0.f, y, 0.f);
	Graphics::point result = ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[v], input);
//	result.y -= pContextInfo->viewports[v].bounds.bottom;
//	result.y = pContextInfo->viewports[v].bounds.top - result.y;
	//	if (v == 1)
//		printf("Y:%f -> %f\n", y, ((result.y+1.0))/2.0);
	return -((result.y-1.0)*pContextInfo->windowHeight)/2.0;
}

Graphics::point GlobalHOGToViewport(Graphics::point where, int viewport)
{
	auto pContextInfo = getCurrentContext();
	return GlobalHOGToViewport(pContextInfo, pContextInfo->viewports[viewport], where);
}

Graphics::point ViewportToGlobalHOG(Graphics::point where, int viewport)
{
	auto pContextInfo = getCurrentContext();
	return ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[viewport], where);
}

Graphics::rect GlobalHOGToViewport(const Graphics::rect &loc, int port)
{
	auto pContextInfo = getCurrentContext();
	return Graphics::rect(GlobalHOGToViewport(pContextInfo, pContextInfo->viewports[port], {loc.left, loc.top}),
						  GlobalHOGToViewport(pContextInfo, pContextInfo->viewports[port], {loc.right, loc.bottom}));
}

Graphics::rect ViewportToGlobalHOG(const Graphics::rect &loc, int port)
{
	auto pContextInfo = getCurrentContext();
	return Graphics::rect(ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[port], {loc.left, loc.top}),
						  ViewportToGlobalHOG(pContextInfo, pContextInfo->viewports[port], {loc.right, loc.bottom}));
}

Graphics::point GlobalHOGToViewport(pRecContext pContextInfo, const viewport &v, Graphics::point where)
{
	if (v.type == kScaleToFill) 		// just scale regular -1/1 axes into the rectangle
	{
		// gets offset into rect
		where.x -= v.bounds.left;
		where.x /= (v.bounds.right-v.bounds.left);
		where.x = where.x*2.0-1.0;
		where.y -= v.bounds.bottom;
		where.y /= (v.bounds.top-v.bounds.bottom);
		where.y = -where.y*2.0+1.0;
		return where;
	}
	else if (v.type == kScaleToSquare)
	{
		double localWidth = v.bounds.right-v.bounds.left;
		double localHeight = v.bounds.bottom-v.bounds.top;
		double actualWidth = pContextInfo->windowWidth*localWidth;
		double actualHeight = pContextInfo->windowHeight*localHeight;
		double xRatio = actualWidth/actualHeight;
		double yRatio = actualHeight/actualWidth;
		xRatio = std::max(xRatio, 1.);
		yRatio = std::max(yRatio, 1.);
		
		where.x *= xRatio;
		where.x -= v.bounds.left;
		where.x /= (v.bounds.right-v.bounds.left);
		where.x = where.x*2.0-1.0;

		
		where.y *= yRatio;
		where.y -= v.bounds.bottom;
		where.y /= (v.bounds.top-v.bounds.bottom);
		where.y = (-where.y)*2.0+1.0;

		
		return where;

//		// gets offset into rect
//		where.x -= v.bounds.left;
//		where.x /= (v.bounds.right-v.bounds.left);
//		where.x = where.x*2.0-1.0;
//		where.y -= v.bounds.bottom;
//		where.y /= (v.bounds.top-v.bounds.bottom);
//		where.y = -where.y*2.0+1.0;
//		return where;
//
//		float localWidth = v.bounds.right-v.bounds.left;
//		float localHeight = v.bounds.bottom-v.bounds.top;
//		// find the smallest dimension; scale into that dimension and then shift into the middle
//		// minSize is in window pixels!
//		float minSize = (pContextInfo->windowWidth*localWidth<pContextInfo->windowHeight*localHeight)?localWidth:localHeight;
//		float xScale;
//		float yScale;
//		point3d center((v.bounds.left+v.bounds.right)/2.f, (v.bounds.top+v.bounds.bottom)/2.f, 0);
//		if (pContextInfo->windowWidth*localWidth < pContextInfo->windowHeight*localHeight)
//		{
//			minSize = localWidth;
//			xScale = 1.0;
//			yScale = (pContextInfo->windowHeight*localHeight)/(pContextInfo->windowWidth*localWidth);
//			//printf("Scaling x:%1.2f, y:%1.2f\n", xScale, yScale);
//		}
//		else {
//			minSize = localHeight;
//			xScale = (pContextInfo->windowWidth*localWidth)/(pContextInfo->windowHeight*localHeight);
//			yScale = 1.0;
//		}
//
//		where.x *= xScale;
//		where.y *= yScale;
//		where *= (minSize/2.0f);
//		where += center;
//
//
//		return where;
	}
	else {
		printf("Unknown scale type\n");
		exit(0);
	}
}

Graphics::point ViewportToGlobalHOG(pRecContext pContextInfo, const viewport &v, Graphics::point where)
{
	if (v.type == kScaleToFill)
	{
		where.x = (where.x+1.0)/2.0;
		where.x *= (v.bounds.right-v.bounds.left);
		where.x += v.bounds.left;

		where.y = -(where.y-1.0)/2.0;
		where.y *= (v.bounds.top-v.bounds.bottom);
		where.y += v.bounds.bottom;
		return where;
	}
	else if (v.type == kScaleToSquare)
	{
		double localWidth = v.bounds.right-v.bounds.left;
		double localHeight = v.bounds.bottom-v.bounds.top;
		double actualWidth = pContextInfo->windowWidth*localWidth;
		double actualHeight = pContextInfo->windowHeight*localHeight;
		double xRatio = actualWidth/actualHeight;
		double yRatio = actualHeight/actualWidth;
		xRatio = std::max(xRatio, 1.);
		yRatio = std::max(yRatio, 1.);
		
		where.x = (where.x+1.0)/2.0;
		where.x *= (v.bounds.right-v.bounds.left);
		where.x += v.bounds.left;
		where.x /= xRatio;
		
		where.y = -(where.y-1.0)/2.0;
		where.y *= (v.bounds.top-v.bounds.bottom);
		where.y += v.bounds.bottom;
		where.y /= yRatio;
		return where;

		
//		//		printf("From (%f, %f) to ", where.x, where.y);
////		float localWidth = v.bounds.right-v.bounds.left;
////		float localHeight = v.bounds.bottom-v.bounds.top;
//		// find the smallest dimension; scale into that dimension and then shift into the middle
//		// minSize is in window pixels!
//		float minSize;// = (pContextInfo->windowWidth*localWidth<pContextInfo->windowHeight*localHeight)?localWidth:localHeight;
//		float xScale;
//		float yScale;
////		printf("-->Window: w%d h%d\n", pContextInfo->windowWidth, pContextInfo->windowHeight);
//		if (pContextInfo->windowWidth*localWidth < pContextInfo->windowHeight*localHeight)
//		{
//			minSize = localWidth;
//			xScale = 1.0;
//			yScale = (pContextInfo->windowHeight*localHeight)/(pContextInfo->windowWidth*localWidth);
//			//printf("Scaling x:%1.2f, y:%1.2f\n", xScale, yScale);
//		}
//		else {
//			minSize = localHeight;
//			xScale = (pContextInfo->windowWidth*localWidth)/(pContextInfo->windowHeight*localHeight);
//			yScale = 1.0;
//		}
//		point3d center((v.bounds.left+v.bounds.right)/2.f, (v.bounds.top+v.bounds.bottom)/2.f, 0);
//
//		where -= center;
//		where /= (minSize/2.0f);
//		where.x /= xScale;
//		where.y /= yScale;
////		printf("(%f, %f)\n", where.x, where.y);
//
//		return where;
	}
	else {
		printf("Unknown scale type\n");
		exit(0);
	}
}


/* New low-end mouse handler. Does the viewport computation - just requires incoming global HOG coordinates. */
bool HandleMouse(pRecContext pContextInfo, point3d where, tButtonType button, tMouseEventType mouse)
{
	for (int x = MAXPORTS-1; x >= 0; x--)
	{
//		if (!pContextInfo->viewports[x].active)
//			continue;
		if (!PointInRect(where, pContextInfo->viewports[x].bounds))
			continue;
		// got hit in rect
		Graphics::point res = GlobalHOGToViewport(pContextInfo, pContextInfo->viewports[x], where);
		// click handled
		if (HandleMouseClick(pContextInfo, x, -1, -1, res, button, mouse))
			return true;
	}
	return false;
}

// this is called by the OS when it gets a click
bool HandleMouseClick(pRecContext pContextInfo, int viewport, int x, int y, point3d where,
					  tButtonType button, tMouseEventType mouse)
{
	for (unsigned int j = 0; j < mouseCallbacks2.size(); j++)
	{
		if (mouseCallbacks2[j]->which&mouse) // need to ask for event to call handler
		{
			if (mouseCallbacks2[j]->mC(pContextInfo->windowID, viewport, x, y, where,
									   button, mouse))
				return true;
		}
	}
	return HandleMouseClick(pContextInfo, x, y, where, button, mouse);
}

bool HandleMouseClick(pRecContext pContextInfo, int x, int y, point3d where,
											tButtonType button, tMouseEventType mouse)
{
	for (unsigned int j = 0; j < mouseCallbacks.size(); j++)
	{
		if (mouseCallbacks[j]->which&mouse) // need to ask for event to call handler
		{
			if (mouseCallbacks[j]->mC(pContextInfo->windowID, x, y, where,
									  button, mouse))
				return true;
		}
	}
	return false;
}

void InstallWindowHandler(WindowCallback wC)
{
	windowCallbacks.push_back(new windowCallbackData(wC));
}

void RemoveWindowHandler(WindowCallback wC)
{
	for (unsigned int x = 0; x < windowCallbacks.size(); x++)
	{
		if (windowCallbacks[x]->wC == wC)
		{
			delete windowCallbacks[x];
			windowCallbacks[x] = windowCallbacks[windowCallbacks.size()-1];
			windowCallbacks.pop_back();
		}
	}
}

void HandleWindowEvent(pRecContext pContextInfo, tWindowEventType e)
{
	for (unsigned int j = 0; j < windowCallbacks.size(); j++)
	{
		//printf("Calling window callback %d\n", x);
		windowCallbacks[j]->wC(pContextInfo->windowID, e);
	}
}

//void InstallDataHandler(DataCallback dC)
//{
//	dataCallbacks.push_back(dataCallbackData(dC));
//}
//
//void RemoveDataHandler(DataCallback dC)
//{
//	for (unsigned int x = 0; x < dataCallbacks.size(); x++)
//	{
//		if (dataCallbacks[x].dC == dC)
//		{
//			dataCallbacks[x] = dataCallbacks[dataCallbacks.size()-1];
//			dataCallbacks.pop_back();
//		}
//	}
//}


// intializes context conditions
void initialConditions(pRecContext pContextInfo)
{
	pContextInfo->moveAllPortsTogether = true;
	pContextInfo->numPorts = 3;
	pContextInfo->currPort = 0;
	for (int x = 0; x < MAXPORTS; x++)
	{
		resetCamera(&pContextInfo->camera[x]);
		for (int y = 0; y < 4; y++)
		{
			pContextInfo->camera[x].rotations.worldRotation[y] = 0;
			pContextInfo->camera[x].rotations.cameraRotation[y] = 0.0001;
		}
//		pContextInfo->camera[x].rotations.cameraRotation[0] = 180;
//		pContextInfo->camera[x].rotations.cameraRotation[2] = 1;
		pContextInfo->camera[x].thirdPerson = true;
	}
	gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;

	pContextInfo->windowID = gNextWindowID++;
	ReinitViewports(pContextInfo->windowID, {-1, -1, 1, 1}, kScaleToSquare);
}

bool DoKeyboardCommand(pRecContext pContextInfo, unsigned char keyHit, bool shift, bool cntrl, bool alt)
{
	DoKeyboardCallbacks(pContextInfo, tolower(keyHit), 
											shift?kShiftDown:(cntrl?kControlDown:(alt?kAltDown:kNoModifier)));
	return false;
}


//void rotateObject()
//{
//	pRecContext pContextInfo = getCurrentContext();
//	if (!pContextInfo)
//		return;
//	pContextInfo->rotations[pContextInfo->currPort].objectRotation[0] += 1;
//	pContextInfo->rotations[pContextInfo->currPort].objectRotation[1] = 0;
//	pContextInfo->rotations[pContextInfo->currPort].objectRotation[2] = 1;
//	pContextInfo->rotations[pContextInfo->currPort].objectRotation[3] = 0;
//}

void resetCamera()
{
	pRecContext pContextInfo = getCurrentContext();
	if (!pContextInfo)
		return;
	pContextInfo->numPorts = 1;
	for (int x = 0; x < MAXPORTS; x++)
	{
		resetCamera(&pContextInfo->camera[x]);
		for (int y = 0; y < 4; y++)
		{
			pContextInfo->camera[x].rotations.worldRotation[y] = 0;
			pContextInfo->camera[x].rotations.cameraRotation[y] = 0.0001;
		}
//		pContextInfo->camera[x].rotations.cameraRotation[0] = 180;
//		pContextInfo->camera[x].rotations.cameraRotation[2] = 1;
	}
	
	gTrackBallRotation [0] = gTrackBallRotation [1] = gTrackBallRotation [2] = gTrackBallRotation [3] = 0.0f;
	gTrackingContextInfo = 0;

	updateProjection(pContextInfo);  // update projection matrix
//	updateModelView(pContextInfo);
}

// sets the camera data to initial conditions
void resetCamera(recCamera * pCamera)
{
	pCamera->aperture = 10.0;
	
	pCamera->viewPos.x = 0.0;
	pCamera->viewPos.y = 0.0;
	pCamera->viewPos.z = -12.5;
	pCamera->viewDir.x = -pCamera->viewPos.x; 
	pCamera->viewDir.y = -pCamera->viewPos.y; 
	pCamera->viewDir.z = -pCamera->viewPos.z;
	
	pCamera->viewUp.x = 0;  
	pCamera->viewUp.y = -1;//-.1;
	pCamera->viewUp.z = 0;//-1;

	//pCamera->viewRot.worldRotation = {0,0,0,0};
}

recVec cameraLookingAt(int port)
{
	pRecContext pContextInfo = getCurrentContext();
	if (port == -1)
		port = pContextInfo->currPort;
	return /*pContextInfo->camera[port].viewPos-*/pContextInfo->camera[port].viewDir;
}


void cameraLookAt(GLfloat x, GLfloat y, GLfloat z, float cameraSpeed, int port)
{
	pRecContext pContextInfo = getCurrentContext();
	if (!pContextInfo)
		return; 
//	const float cameraSpeed = .1;
	if (port == -1)
		port = pContextInfo->currPort;
	
	pContextInfo->camera[port].viewDir.x = (1-cameraSpeed)*pContextInfo->camera[port].viewDir.x + cameraSpeed*(x - pContextInfo->camera[port].viewPos.x);
	pContextInfo->camera[port].viewDir.y = (1-cameraSpeed)*pContextInfo->camera[port].viewDir.y + cameraSpeed*(y - pContextInfo->camera[port].viewPos.y);
	pContextInfo->camera[port].viewDir.z = (1-cameraSpeed)*pContextInfo->camera[port].viewDir.z + cameraSpeed*(z - pContextInfo->camera[port].viewPos.z);
//	pContextInfo->rotations[port].objectRotation[0] *= (1-cameraSpeed);
//	pContextInfo->rotations[port].worldRotation[0] *= (1-cameraSpeed);
	updateProjection(pContextInfo);
}

void cameraMoveTo(GLfloat x, GLfloat y, GLfloat z, float cameraSpeed, int port)
{
	pRecContext pContextInfo = getCurrentContext();
	if (!pContextInfo)
		return;
//	const float cameraSpeed = .1;
	if (port == -1)
	{
		port = pContextInfo->currPort;
	}
	pContextInfo->camera[port].viewPos.x = (1-cameraSpeed)*pContextInfo->camera[port].viewPos.x + cameraSpeed*x;
	pContextInfo->camera[port].viewPos.y = (1-cameraSpeed)*pContextInfo->camera[port].viewPos.y + cameraSpeed*y;
	pContextInfo->camera[port].viewPos.z = (1-cameraSpeed)*pContextInfo->camera[port].viewPos.z + cameraSpeed*z;
	updateProjection(pContextInfo);
}

void cameraOffset(GLfloat x, GLfloat y, GLfloat z, float cameraSpeed, int port)
{
	pRecContext pContextInfo = getCurrentContext();
	if (!pContextInfo)
		return;
	if (port == -1)
	{
		port = pContextInfo->currPort;
	}
	pContextInfo->camera[port].viewPos.x += cameraSpeed*x;
	pContextInfo->camera[port].viewPos.y += cameraSpeed*y;
	pContextInfo->camera[port].viewPos.z += cameraSpeed*z;
	updateProjection(pContextInfo);
}

void setPortCamera(pRecContext pContextInfo, int currPort)
{
	const double ratios[4][4][4] =
{{{0, 1, 0, 1}},
{{0, 0.5, 0, 1}, {0.5, 0.5, 0, 1}},
{{0, 0.5, 0.5, 0.5}, {0.5, 0.5, 0.5, 0.5}, {0, 1, 0, 0.5}},
{{0, 0.5, 0.5, 0.5}, {0.5, 0.5, 0.5, 0.5}, {0, 0.5, 0, 0.5}, {0.5, 0.5, 0, 0.5}}};
	
	currPort = 0; // NOTE: everyone gets the "full" screen
	//const double *val = ratios[pContextInfo->numPorts-1][currPort]; 
	const double *val = ratios[0][currPort];
	
	pContextInfo->camera[currPort].viewOriginX = pContextInfo->globalCamera.viewOriginX;
	pContextInfo->camera[currPort].viewOriginY = pContextInfo->globalCamera.viewOriginY;
	
	pContextInfo->camera[currPort].viewWidth = (GLint)(val[1]*pContextInfo->globalCamera.viewWidth);
	pContextInfo->camera[currPort].viewHeight = (GLint)(val[3]*pContextInfo->globalCamera.viewHeight);
	//	printf("Window %d port %d width: %d, height %d\n",
	//				 pContextInfo->windowID, currPort,
	//				 pContextInfo->camera[currPort].viewWidth,
	//				 pContextInfo->camera[currPort].viewHeight);
}

void setViewport(pRecContext pContextInfo, int currPort)
{
	pContextInfo->display.SetViewport(currPort);
	currPort = 0;
	
	const double ratios[4][4][4] =
	{{{0, 1, 0, 1}}, // x, width%, y, height%
		{{0, 0.5, 0, 1}, {0.5, 0.5, 0, 1}},
		{{0, 0.5, 0.5, 0.5}, {0.5, 0.5, 0.5, 0.5}, {0, 1, 0, 0.5}},
		{{0, 0.5, 0.5, 0.5}, {0.5, 0.5, 0.5, 0.5}, {0, 0.5, 0, 0.5}, {0.5, 0.5, 0, 0.5}}};
	
	const double *val = ratios[0][currPort];
	//	const double *val = ratios[pContextInfo->numPorts-1][currPort];
	
	glViewport(val[0]*pContextInfo->globalCamera.viewWidth,
			   val[2]*pContextInfo->globalCamera.viewHeight,
			   val[1]*pContextInfo->globalCamera.viewWidth,
			   val[3]*pContextInfo->globalCamera.viewHeight);
	
}

point3d GetOGLPos(pRecContext pContextInfo, int x, int y)
{
	setViewport(pContextInfo, pContextInfo->currPort);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(pContextInfo->camera[pContextInfo->currPort].frust.left, pContextInfo->camera[pContextInfo->currPort].frust.right,
			  pContextInfo->camera[pContextInfo->currPort].frust.bottom, pContextInfo->camera[pContextInfo->currPort].frust.top,
			  pContextInfo->camera[pContextInfo->currPort].frust.near, pContextInfo->camera[pContextInfo->currPort].frust.far);
	// projection matrix already set	
	updateModelView(pContextInfo, pContextInfo->currPort);
	
	if (pContextInfo->numPorts > 1)
		HandleFrame(pContextInfo, pContextInfo->currPort);
	
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;
	
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );
	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
	// if (gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ) == GL_FALSE)
		// printf("WARNING: gluUnProject failed\n");
	//assert(!"gluUnProject missing");
	
//	printf("Clicked (%f, %f, %f) [far plane %f near %f]\n", posX, posY, posZ,
//		   pContextInfo->frust[pContextInfo->currPort].far+pContextInfo->camera[pContextInfo->currPort].viewPos.z,
//		   pContextInfo->frust[pContextInfo->currPort].near+pContextInfo->camera[pContextInfo->currPort].viewPos.z);
	return point3d(posX, posY, posZ);
}

void SetNumPorts(unsigned long windowID, int count)
{
	pRecContext pContextInfo = GetContext(windowID);
	if ((count <= MAXPORTS) && (count > 0))
	{
		pContextInfo->display.SetNumViewports(count);
		pContextInfo->display.SetViewport(0);
		
		if (pContextInfo->numPorts > count)
			pContextInfo->currPort = 0;
		
		pContextInfo->numPorts = count;
		for (int x = 0; x < pContextInfo->numPorts; x++)
		{
			setPortCamera(pContextInfo, x);
		}
		updateProjection(pContextInfo);
	}
}

int GetNumPorts(unsigned long windowID)
{
	pRecContext pContextInfo = GetContext(windowID);
	return pContextInfo->numPorts;
}

int GetActivePort(unsigned long windowID)
{
	pRecContext pContextInfo = GetContext(windowID);
	return pContextInfo->currPort;
}

void SetActivePort(unsigned long windowID, int which)
{
	pRecContext pContextInfo = GetContext(windowID);
	if ((which >= 0) && (which < pContextInfo->numPorts))
		pContextInfo->currPort = which;
}


class bmp_header {
public:
	bmp_header()
	://bfType(19778),
	zero(0), bfOffBits(sizeof(bmp_header)+2), biSize(40), biPlanes(1),
	biBitCount(32), biCompression(0), biSizeImage(0), biXPelsPerMeter(2835), biYPelsPerMeter(2835),
	biClrUsed(0), biClrImportant(0) {}
	
//	uint16_t bfType;//	19778
	uint32_t bfSize; //	??	specifies the size of the file in bytes.
	uint32_t zero; // 0
	uint32_t bfOffBits;
	//	11	4	bfOffBits	1078	specifies the offset from the beginning of the file to the bitmap data.
	
	uint32_t biSize; // 40
	uint32_t biWidth;
	uint32_t biHeight;
	uint16_t biPlanes; // 0 (1??)
	uint16_t biBitCount; // 24
	uint32_t biCompression; // 0
	uint32_t biSizeImage; // 0
	uint32_t biXPelsPerMeter; // 0
	uint32_t biYPelsPerMeter; // 0
	uint32_t biClrUsed; // 0
	uint32_t biClrImportant; // 0
};

void SaveScreenshot(unsigned long windowID, const char *filename)
{
	pRecContext pContextInfo = GetContext(windowID);

	char file[strlen(filename)+5];
	sprintf(file, "%s.bmp", filename);
	FILE *f = fopen(file, "w+");

	if (f == 0) return;
	
//	3	4	bfSize	??	specifies the size of the file in bytes.
//	19	4	biWidth	100	specifies the width of the image, in pixels.
//	23	4	biHeight	100	specifies the height of the image, in pixels.

	
	uint32_t width  = pContextInfo->globalCamera.viewWidth;
	uint32_t height  =pContextInfo->globalCamera.viewHeight;
	long rowBytes = width * 4;
	long imageSize = rowBytes * height;
	std::vector<char> image(imageSize);
//	char image[imageSize];
	char zero[4] = {0, 0, 0, 0};
	glReadPixels(0, 0, GLsizei(width), GLsizei(height), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, &image[0]);//GL_BGRA
	
	bmp_header h;
	h.biWidth = width;
	h.biHeight = height;
	int buffer = (4-width%4)%4;
	h.bfSize = sizeof(bmp_header)+2+(width+buffer)*height*4;
	h.biSizeImage = (width+buffer)*height*4;
	uint16_t bfType = 19778;
	fwrite(&bfType, sizeof(bfType), 1, f);
	fwrite(&h, sizeof(bmp_header), 1, f);
	for (int x = 0; x < height; x++)
	{
		fwrite(&image[x*width*4], sizeof(char), width*4, f);
		if (0 != width%4)
			fwrite(&zero, sizeof(char), buffer, f);
	}
	fclose(f);
}

void SetZoom(int windowID, float amount)
{
	pRecContext pContextInfo = GetContext(windowID);

	if (pContextInfo->moveAllPortsTogether)
	{
		for (int x = 0; x < pContextInfo->numPorts; x++)
		{
			pContextInfo->camera[x].aperture = amount;
//			pContextInfo->camera[x].viewPos.z = -12.5+amount;
//			if (pContextInfo->camera[x].viewPos.z == 0.0) // do not let z = 0.0
//				pContextInfo->camera[x].viewPos.z = 0.0001;
			updateProjection(pContextInfo, x);  // update projection matrix
		}
	}
	else {
		pContextInfo->camera[pContextInfo->currPort].aperture = amount;
//		pContextInfo->camera[pContextInfo->currPort].viewPos.z = -12.5+amount;
//		if (pContextInfo->camera[pContextInfo->currPort].viewPos.z == 0.0) // do not let z = 0.0
//			pContextInfo->camera[pContextInfo->currPort].viewPos.z = 0.0001;
		updateProjection(pContextInfo, pContextInfo->currPort);  // update projection matrix
	}
}

recVec GetHeading(unsigned long windowID, int which)
{
	recVec v;
	GetHeading(windowID, which, v.x, v.y, v.z);
	return v;
}

void GetHeading(unsigned long windowID, int which, GLdouble &hx, GLdouble &hy, GLdouble &hz)
{
	pRecContext pContextInfo = GetContext(windowID);

	double fRot[4];
	for (int x = 0; x < 4; x++)
		fRot[x] = pContextInfo->camera[which].rotations.cameraRotation[x];
	// these formulas are derived from the opengl redbook 1.4 pg 700
	double xp, yp, zp, len, sa, ca;//hx, hy, hz,
	len = 1/sqrt(fRot[1]*fRot[1] +
				 fRot[2]*fRot[2] +
				 fRot[3]*fRot[3]);
	xp = fRot[1]*len;
	yp = fRot[2]*len;
	zp = fRot[3]*len;
	ca = cos(-fRot[0]*PI/180.0);
	sa = sin(-fRot[0]*PI/180.0);
	hx = (1-ca)*xp*zp+sa*yp;
	hy = (1-ca)*yp*zp-sa*xp;
	hz = ca+(1-ca)*zp*zp;
	len = 1/sqrt(hx*hx+hy*hy+hz*hz);
	hx *= len;
	hy *= len;
	hz *= len;
//	printf("Heading vector: (%1.3f, %1.3f, %1.3f)\n", hx, hy, hz);
}

