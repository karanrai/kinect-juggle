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

#include "WorldRenderer.h"
#include "UserDetector.h"
#include "config.h"
#include <GLFrustum.h>
#include "util.h"
#include "math.h"


const float DEFAULT_DEPTH_ADJUSTMENT = 105; // empirical valuel. adjustable by 'q' and 'a' key.

WorldRenderer::WorldRenderer(RenderingContext* rctx, DepthGenerator* depthGen, ImageGenerator* imageGen,
							 BallManager* ball_manager)
: AbstractOpenGLRenderer(rctx)
{
	m_depthGen = depthGen;
	m_imageGen = imageGen;
	//m_henshinDetector = henshinDetector;
	m_ball_manager = ball_manager;

	DepthMetaData dmd;
	m_depthGen->GetMetaData(dmd);
	m_width = dmd.XRes();
	m_height = dmd.YRes();

	// allocate working buffers
	XnUInt32 numPoints = getNumPoints();
	m_vertexBuf = new M3DVector3f[numPoints];
	m_colorBuf = new M3DVector4f[numPoints];

	// pre-set values on working buffers
	M3DVector3f* vp = m_vertexBuf;
	M3DVector4f* cp = m_colorBuf;
	for (XnUInt32 iy = 0; iy < m_height; iy++) {
		for (XnUInt32 ix = 0; ix < m_width; ix++) {
			(*vp)[0] = normalizeX(float(ix));
			(*vp)[1] = normalizeY(float(iy));
			(*vp)[2] = 0;
			vp++;
			(*cp)[0] = (*cp)[1] = (*cp)[2] = 0;
			(*cp)[3] = 1; // alpha is always 1.0
			cp++;
		}
	}

	m_batch.init(numPoints);

	m_depthAdjustment = DEFAULT_DEPTH_ADJUSTMENT;
}

WorldRenderer::~WorldRenderer()
{
	delete[] m_vertexBuf;
	delete[] m_colorBuf;
}

void WorldRenderer::getHenshinData(XnUserID* pUserID, const XnLabel** ppLabel, XV3* pHeadCenter, XV3* pHeadDirection)
{
	UserDetector* userDetector = m_ball_manager->user_detector();

	XnUserID userID = *pUserID = userDetector->getTrackedUserID();

	if (userID) {
		SceneMetaData smd;
		userDetector->getUserGenerator()->GetUserPixels(userID, smd);
		*ppLabel = smd.Data();

		XV3 ps[2];
		//ps[0].assign(m_kkhStatus->center);
		ps[0].assign(userDetector->getSkeletonJointPosition(XN_SKEL_HEAD));
		ps[1].assign(userDetector->getSkeletonJointPosition(XN_SKEL_NECK));
		m_depthGen->ConvertRealWorldToProjective(3, ps, ps);
		normalizeProjective(&ps[0]);
		normalizeProjective(&ps[1]);
		//normalizeProjective(&ps[2]);
		//*pLightCenter = ps[0];
		*pHeadCenter = ps[0];
		*pHeadDirection = ps[0] - ps[1];
	}
}

#ifndef USE_MACRO
inline void setGray(M3DVector4f* cp, float g)
{
	(*cp)[0] = (*cp)[1] = (*cp)[2] = g * 0.7f + 0.2f;
}
#else
#define setGray(cp, g) ((*(cp))[0] = (*(cp))[1] = (*(cp))[2] = (g) * 0.7f + 0.2f)
#endif

#ifndef USE_MACRO
inline void setRed(M3DVector4f* cp, float g)
{
	(*cp)[0] = g * 0.8f + 0.2f;
	(*cp)[1] = (*cp)[2] = 0;
}
#else
#define setRed(cp, g) ((*(cp))[0] = (g) * 0.8f + 0.2f, (*(cp))[1] = (*(cp))[2] = 0)
#endif

#ifndef USE_MACRO
inline void setRGB(M3DVector4f* cp, const XnRGB24Pixel& p)
{
	(*cp)[0] = b2fNormalized(p.nRed);
	(*cp)[1] = b2fNormalized(p.nGreen);
	(*cp)[2] = b2fNormalized(p.nBlue);
}
#else
#define setRGB(cp, p) ((*(cp))[0] = b2fNormalized((p).nRed), (*(cp))[1] = b2fNormalized((p).nGreen), (*(cp))[2] = b2fNormalized((p).nBlue))
#endif

void WorldRenderer::draw()
{
	drawBackground();
}

void WorldRenderer::drawBackground()
{
	m_rctx->orthoMatrix.PushMatrix();
	{
		//TODO: find out what this does
		//m_rctx->orthoMatrix.Translate(
		//	float(m_rng.gaussian(0.6)) * currentIntensity * 0.01f,
		//	float(m_rng.gaussian(0.6)) * currentIntensity * 0.01f,
		//	0);

		// setup shader
		m_rctx->shaderMan->UseStockShader(GLT_SHADER_SHADED, m_rctx->orthoMatrix.GetMatrix());

		// get depth buffer
		DepthMetaData dmd;
		m_depthGen->GetMetaData(dmd);
		const XnDepthPixel* dp = dmd.Data();

		// get image buffer
		ImageMetaData imd;
		m_imageGen->GetMetaData(imd);
		const XnRGB24Pixel* ip = imd.RGB24Data();

		// get working buffers
		M3DVector3f* vp = m_vertexBuf;
		M3DVector4f* cp = m_colorBuf;
		XnUInt32 numPoints = getNumPoints();

		// setup henshin-related information
		const float Z_SCALE = 10.0f;
		XnUserID userID = 0;
		const XnLabel* lp = NULL;
		XV3 headCenter, headDirection;
		getHenshinData(&userID, &lp, &headCenter, &headDirection);

		float lightRadius = 900.0f;

		bool isTracked = userID && lp;

		const int NUM_BALLS = 3;
		XV3 ball_centers[NUM_BALLS];
		bool ball_enabled_flags[NUM_BALLS];

		float ball_radius[3];
		float ball_core_radius[3];
		float ball_core_radius2[3];

		//get the ball centres and transform into projective coords
		//Also calculate an appropriate radius to make the ball scale as it moves away from the camera
		for (int j=0; j< NUM_BALLS; j++) {
			 m_ball_manager->GetBallInfo(j, &ball_enabled_flags[j],&ball_centers[j]);

			 if(!ball_enabled_flags[j]) continue;

			 XV3 ball_top(ball_centers[j]); //copy the ball center before transformation
			 
			 m_depthGen->ConvertRealWorldToProjective(1, &ball_centers[j], &ball_centers[j]);
			 normalizeProjective(&ball_centers[j]);
			 
			 //this is probably a clunky way to transform the radius into projectiev coods but it seems to work ok
			 ball_top.Y +=lightRadius;
			 m_depthGen->ConvertRealWorldToProjective(1, &ball_top, &ball_top);
			 normalizeProjective(&ball_top);
			 ball_radius[j] = fabs(ball_top.Y-ball_centers[j].Y);
			 ball_core_radius[j]  = ball_radius[j]*0.1f;
			 ball_core_radius2[j] = square(ball_core_radius[j]);
		}

		XnUInt32 ix = 0, iy = 0;
		float nearZ = PERSPECTIVE_Z_MIN + m_depthAdjustment;
		for (XnUInt32 i = 0; i < numPoints; i++, dp++, ip++, vp++, cp++, lp++, ix++) {

			if (ix == m_width) {
				ix = 0;
				iy++;
			}

			// (*vp)[0] (x) is already set
			// (*vp)[1] (y) is already set
			(*vp)[2] = (*dp) ? getNormalizedDepth(*dp, nearZ, PERSPECTIVE_Z_MAX) : Z_INFINITE;

			setRGB(cp, *ip);

			//highlight the tracked user
			if(isTracked) {
				if(*lp == userID) {
					(*cp)[0] *= 1.2f;
					(*cp)[1] *= 1.2f;
					(*cp)[2] *= 1.2f;
				}
			}

			// draw balls
			for(int j=0; j < NUM_BALLS; j++) {
				if(!ball_enabled_flags[j]) continue;

				XV3& lightCenter = ball_centers[j];
				//float ball_depth = (*dp) ? getNormalizedDepth(ball_radius[j], nearZ, PERSPECTIVE_Z_MAX) : 0;

				if((*vp)[2] < (lightCenter.Z - 0.001*ball_radius[j])) continue; //don't draw obscured pixels
				{
					// TODO: Should we use 3D object?
					XV3 flatCoords(*vp);
					flatCoords.Z = lightCenter.Z;
					float flatDistance2 = lightCenter.distance2(flatCoords);

					if (flatDistance2 < ball_core_radius2[j]) {
						float r = (1.0f - sqrt(flatDistance2) / ball_core_radius[j]) * (1.0f + 0.8f * ball_radius[j]);
						float r2 = r * r;
						float a = (r <= 1.0f) ? (2 * r2 - r2 * r2) : 1.0f;

						(*cp)[0] *= 1.2;
						(*cp)[1] *= 1.2;
						(*cp)[2] *= 1.2;

						//assuming we only have three balls cycle through red,green and blue for each one
						(*cp)[j] = interpolate((*cp)[0], 1.0f, a);
						//(*cp)[1] = interpolate((*cp)[1], 1.0f, a);
						//(*cp)[2] = interpolate((*cp)[2], 1.0f, a);
					}
				}
			}
		}

		glEnable(GL_POINT_SIZE);
		glPointSize(getPointSize());
		m_batch.draw(m_vertexBuf, m_colorBuf);
	}
	m_rctx->orthoMatrix.PopMatrix();
}

WorldRenderer::Batch::Batch()
{
	m_numPoints = 0;
	m_vertexArrayID = 0;
	m_vertexBufID = 0;
	m_colorBufID = 0;
}

WorldRenderer::Batch::~Batch()
{
	if (m_vertexArrayID) glDeleteVertexArrays(1, &m_vertexArrayID);
	if (m_vertexBufID) glDeleteBuffers(1, &m_vertexBufID);
	if (m_colorBufID) glDeleteBuffers(1, &m_colorBufID);
}

void WorldRenderer::Batch::init(GLuint numPoints)
{
	m_numPoints = numPoints;

	glGenBuffers(1, &m_vertexBufID);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(M3DVector3f) * numPoints, NULL, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &m_colorBufID);
	glBindBuffer(GL_ARRAY_BUFFER, m_colorBufID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(M3DVector4f) * numPoints, NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &m_vertexArrayID);
	glBindVertexArray(m_vertexArrayID);
	{
		glEnableVertexAttribArray(GLT_ATTRIBUTE_VERTEX);
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufID);
		glVertexAttribPointer(GLT_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(GLT_ATTRIBUTE_COLOR);
		glBindBuffer(GL_ARRAY_BUFFER, m_colorBufID);
		glVertexAttribPointer(GLT_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, 0);
	}
	glBindVertexArray(0);
}

void WorldRenderer::Batch::draw(M3DVector3f* vertexBuf, M3DVector4f* colorBuf)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(M3DVector3f) * m_numPoints, vertexBuf);

	glBindBuffer(GL_ARRAY_BUFFER, m_colorBufID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(M3DVector4f) * m_numPoints, colorBuf);

	glBindVertexArray(m_vertexArrayID);
	glDrawArrays(GL_POINTS, 0, m_numPoints);
	glBindVertexArray(0);
}
