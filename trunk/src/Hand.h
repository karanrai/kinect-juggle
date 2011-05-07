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

#ifndef _HAND_H_
#define _HAND_H_

#include "common.h"
#include "ParticleData.h"

class Ball;

class Hand : public ParticleData {
 private:
	 XnSkeletonJoint joint_id_;

	 Ball* ball_; //ball currently being held by the hand
	 bool is_visible_;
	 bool new_ball_request_;
	 float visibility_timer_;
	 float ball_timer_;

	 Hand&  operator = (const Hand& other) { /*..*/}
	 Hand(const Hand& other) {/*..*/}

 public:	 
	 Hand(XnSkeletonJoint joint_id);
	 virtual ~Hand();

	 void Update(float dt,bool is_visible);

	 Ball* GetBall() const {return ball_;}
	 void SetBall(Ball* ball) {
		new_ball_request_ = false;
		ball_ = ball;
	 }

	 float GetTimeWithoutBall() const { return ball_timer_;}
	 float GetVisibilityTime() const { return visibility_timer_;}

	 XnSkeletonJoint joint_id() const { return joint_id_;}

	 //void SetVisibility(const bool is_visible) {
	//	 is_visible_ = is_visible;
	//	 if(!is_visible) visibility_timer_ = 0.0f;
	 //}
	 //bool GetIsVisible() const { return is_visible_;}

	 void SetBallRequestFlag() {if(!ball_) new_ball_request_ = true;}
	 bool GetBallRequestFlag() { return new_ball_request_;}
};

#endif