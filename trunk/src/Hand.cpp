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

#include "Hand.h"
#include "Ball.h"
#include "UserDetector.h"

Hand::Hand(XnSkeletonJoint joint_id)
 :	joint_id_(joint_id),
	ball_(NULL),
	is_visible_(false),
	new_ball_request_(false),
	visibility_timer_(0.0f),
	ball_timer_(0.0f)
	
{
}

Hand::~Hand()
{
}

void Hand::Update(float dt, bool is_visible) {
	is_visible_ = is_visible;

	if(!is_visible_) visibility_timer_ = 0.0f;
	else visibility_timer_ += dt;

	//count number of seconds since we last had a ball
	if(ball_) ball_timer_ = 0.0f;
	else ball_timer_ += dt;
}
