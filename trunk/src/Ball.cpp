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

#include "Ball.h"

Ball::Ball()
	: is_visible_(false)
{
}

Ball::~Ball()
{
}

const float MAX_BALL_SPEED = 5000.0f; //mm/s

void Ball::UpdateVelocity(float dt, float g) {
	UpdateVelocity(dt,g,0);
}

void Ball::UpdateVelocity(float dt, float g, float max_descent_speed) {
	XV3 v_new = v_[0];

	v_new.Y -= g*dt;

	float speed = v_new.magnitude();

	float max_speed = MAX_BALL_SPEED;

	//Make the balls float when descending by limiting their max_descent_speed
	if( v_new.Y < 0 && 
		max_descent_speed > 0 &&
		max_descent_speed < MAX_BALL_SPEED) {
		max_speed = max_descent_speed;
	}

	if(speed > max_speed) {
		v_new *= (max_speed/speed);
	}

	AddDataPoint(dt, x_[0] + v_new*dt);
}
