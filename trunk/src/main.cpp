//@COPYRIGHT@//
//
// Copyright (c) 2011, Tomoto S. Washio
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//   * Neither the name of the Tomoto S. Washio nor the names of his
//     contributors may be used to endorse or promote products derived from
//     this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//@COPYRIGHT@//

//Code modified by Tom Makin, 05/05/2011

#include "common.h"
#include "config.h"
#include "util.h"

#include "RenderingContext.h"

#include "WorldRenderer.h"
#include "SkeletonRenderer.h"

#include "UserDetector.h"

#include "FrameRateCounter.h"

#include <GLShaderManager.h>
#include <GLFrustum.h>

#define APP_VERSION "0.1"

// OpenNI objects
Context g_context;
DepthGenerator g_depthGen;
ImageGenerator g_imageGen;
UserGenerator g_userGen;

// GL objects
GLShaderManager g_shaderMan;

// App objects
RenderingContext g_renderingCtx;
BallManager* g_ballManager;

WorldRenderer* g_worldRenderer;
SkeletonRenderer* g_skeletonRenderer;

UserDetector* g_userDetector;
FrameRateCounter g_frameRateCounter;

static void onGlutKeyboard(unsigned char key, int x, int y)
{
	switch (key) {
		case 27:
			exit(1);
		case '=':
		case '+':
			g_ballManager->IncreaseFloatFactor();
			break;
		case '_':
		case '-':
			g_ballManager->DecreaseFloatFactor();
			break;
		case 's':
			g_skeletonRenderer->toggleEnabled();
			break;
		case 'f':
			g_frameRateCounter.toggleEnabled();
			break;
		case 'm':
			g_renderingCtx.mirror();
			break;
		case 'j':
			g_ballManager->ToggleJugglingMode();
			break;
	}
}

static void onGlutDisplay()
{
	g_frameRateCounter.update();

	g_context.WaitAndUpdateAll();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//This is where all the ball and hand updating is done
	g_ballManager->Update();

	g_worldRenderer->draw();

	g_skeletonRenderer->draw();

	glutSwapBuffers();
}
static void onGlutIdle()
{
	glutPostRedisplay();
}

static void initXN()
{
	CALL_XN( g_context.InitFromXmlFile(getResourceFile("config", "OpenNIConfig.xml").c_str()) );
	CALL_XN( g_context.SetGlobalMirror(TRUE) );
	CALL_XN( g_context.FindExistingNode(XN_NODE_TYPE_IMAGE, g_imageGen) );
	CALL_XN( g_context.FindExistingNode(XN_NODE_TYPE_DEPTH, g_depthGen) );
	CALL_XN( g_depthGen.GetAlternativeViewPointCap().SetViewPoint(g_imageGen) );
	CALL_XN( g_context.FindExistingNode(XN_NODE_TYPE_USER, g_userGen) );
	CALL_XN( g_userGen.Create(g_context) );

	//onnly need the upper body for the hand and arm data
	g_userGen.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);
	//g_userGen.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

	ImageMetaData imageMD;
	g_imageGen.GetMetaData(imageMD);
	CHECK_ERROR(imageMD.PixelFormat() == XN_PIXEL_FORMAT_RGB24, "This camera is not supported.");

	g_userDetector = new UserDetector(&g_userGen);

	g_ballManager = new BallManager(g_userDetector);

	CALL_XN( g_context.StartGeneratingAll() );
}

static void sorryThisProgramCannotRunBecause(const char* reason)
{
	printf("Failed: Sorry this program cannot run because %s.\n", reason);
	errorExit();
}

static void checkOpenGLCapabilities()
{
	const char* openGLVersion = (const char*)(glGetString(GL_VERSION));
	printf("OpenGL version = %s\n", openGLVersion);
	const char* shaderLanguageVersion = (const char*)(glGetString(GL_SHADING_LANGUAGE_VERSION));
	if (shaderLanguageVersion) {
		printf("Shader language version = %s\n", shaderLanguageVersion);
	} else {
		sorryThisProgramCannotRunBecause("this GPU does not support programmable shaders");
		errorExit();
	}

	if (glGenVertexArrays) {
		// VAOs are supported. Good.
	} else {
		sorryThisProgramCannotRunBecause("VAOs are not supported");
	}
}

static void initGL(int* pArgc, char* argv[])
{
	glutInit(pArgc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(X_RES, Y_RES);
	glutCreateWindow(WIN_TITLE);
	glutHideWindow();
	CALL_GLEW( glewInit() );

	checkOpenGLCapabilities();

	glutKeyboardFunc(onGlutKeyboard);
	glutDisplayFunc(onGlutDisplay);
	glutIdleFunc(onGlutIdle);

	g_shaderMan.InitializeStockShaders();

	glEnable(GL_DEPTH_TEST);

	// enable smoothing
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_SMOOTH);
	// glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	g_renderingCtx.mirror();
}

static void initProjection()
{
	XnFieldOfView fov;
	g_depthGen.GetFieldOfView(fov);

	GLFrustum frustum;
	float verticalFOVInDegree = float(m3dRadToDeg(fov.fVFOV));
	float aspect = float(tan(fov.fHFOV/2)/tan(fov.fVFOV/2));
	frustum.SetPerspective(verticalFOVInDegree, aspect, PERSPECTIVE_Z_MIN, PERSPECTIVE_Z_MAX);

	g_renderingCtx.projectionMatrix.LoadMatrix(frustum.GetProjectionMatrix());
	g_renderingCtx.projectionMatrix.Scale(-1, 1, -1);

	g_renderingCtx.orthoMatrix.Scale(1/XY_ASPECT, 1, 1);
}

static void initRenderers()
{
	g_renderingCtx.shaderMan = &g_shaderMan;

	LOG( initProjection() );

	LOG( g_worldRenderer = new WorldRenderer(&g_renderingCtx, &g_depthGen, &g_imageGen, g_ballManager) );
	LOG( g_skeletonRenderer = new SkeletonRenderer(&g_renderingCtx, &g_depthGen, g_userDetector) );
}

static void displayWelcomeMessage()
{
	puts("kinect-juggle " APP_VERSION);
	char file[64];
	char line[256];
	sprintf(file, "welcome-%d.txt", GetConsoleCP());
	FILE* fp = fopen(getResourceFile("message", file).c_str(), "r");
	if (fp) {
		while (fgets(line, sizeof(line) - 1, fp)) {
			fputs(line, stdout);
		}
		fclose(fp);
	} else {
		puts("Welcome to Kinect Juggle!");
		puts("");
		puts("Available keys during the play:");
		puts("[ESC]  -- Exit");
		puts("[j]    -- Enable juggling mode (stops balls flying towards the camera)");
		puts("[+][-] -- Adjust ball descent speed while juggling");
		puts("[m]    -- Mirror the screen.");
		puts("[s]    -- Toggle skeleton (for troubleshooting).");
		puts("");
		puts("It may take a minute until initialization completes... Enjoy!");
		puts("");
	}
}

void main(int argc, char* argv[])
{
	// enable memory leak report for Win32 debug
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	displayWelcomeMessage();

	initGL(&argc, argv);
	initXN();
	initRenderers();
	glutShowWindow();
	glutMainLoop();
}
