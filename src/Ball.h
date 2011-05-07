//
// Copyright (C) 2011 Tom Makin
// All rights reserved.
//
// The software in this package is published under the terms of the BSD
// style license a copy of which has been included with this distribution in
// the LICENSE.txt file.
//
//
//@COPYRIGHT@//

#include "common.h"
#include "ParticleData.h"

#ifndef _BALL_H_
#define _BALL_H_

class Ball : public ParticleData {
public:
	Ball();
	virtual ~Ball();

	bool is_visible() const {return is_visible_;}

	void Show() { is_visible_ = true;}
	void Hide() { is_visible_ = false;}

	void UpdateVelocity(float dt, float g);
	void UpdateVelocity(float dt, float g,float max_descent_speed);

private:
	bool is_visible_;
};

#endif
