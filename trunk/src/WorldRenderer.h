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

#ifndef _DEPTH_IMAGE_RENDERER_H_
#define _DEPTH_IMAGE_RENDERER_H_

#include "common.h"
#include "config.h"
#include "util.h"
#include "AbstractOpenGLRenderer.h"
#include "BallManager.h"
#include "TimeTicker.h"

class WorldRenderer : public AbstractOpenGLRenderer
{
private:
	// This should be GLBatch's job, but GLBatch has memory leak!
	class Batch {
	private:
		GLuint m_numPoints;
		GLuint m_vertexArrayID;
		GLuint m_vertexBufID;
		GLuint m_colorBufID;

	public:
		Batch();
		~Batch();
		void init(GLuint numPoints);
		void draw(M3DVector3f* vertexBuf, M3DVector4f* colorBuf);
	};

protected:
	DepthGenerator* m_depthGen;
	ImageGenerator* m_imageGen;
	
	//HenshinDetector* m_henshinDetector;
	BallManager* m_ball_manager;

	Batch m_batch; 

	XnUInt32 m_width;
	XnUInt32 m_height;
	M3DVector3f* m_vertexBuf;
	M3DVector4f* m_colorBuf;

	float m_depthAdjustment;

	cv::RNG m_rng;

public:
	WorldRenderer(RenderingContext* rctx, DepthGenerator* depthGen, ImageGenerator* imageGen, BallManager* ball_manager);
	virtual ~WorldRenderer();
	
	void draw();

	void addDepthAdjustment(float value) { m_depthAdjustment += value; }
private:
	XnUInt32 getNumPoints() { return m_width * m_height; }

	void getHenshinData(XnUserID* pUserID, const XnLabel** ppLabel, XV3* pHeadCenter, XV3* pHeadDirection);

	void drawBackground();
	// void drawHenshiningGlow();

	float normalizeX(float x) { return (m_width - x * 2.0f) / m_height; }
	float normalizeY(float y) { return 1.0f - (y * 2.0f / m_height); }

	void normalizeProjective(XV3* pp)
	{
		float nearZ = PERSPECTIVE_Z_MIN + m_depthAdjustment;
		pp->X = normalizeX(pp->X);
		pp->Y = normalizeY(pp->Y);
		pp->Z = getNormalizedDepth(pp->Z, nearZ, PERSPECTIVE_Z_MAX);
	}
};

#endif
