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

#include "BallManager.h"
#include "UserDetector.h"
#include "util.h"
#include "common.h"

BallManager::BallManager(UserDetector* user_detector)
	: 	left_hand_(XN_SKEL_LEFT_HAND),
		right_hand_(XN_SKEL_RIGHT_HAND),
		user_detector_(user_detector),
		juggling_mode_(false),
		terminal_velocity_(TERMINAL_VELOCITY_DEFAULT)
{
	//XV3 temp(100,100,100);
	//balls_[0].AddDataPoint(0,temp);
	//balls_[0].Show();
}

BallManager::~BallManager()
{
}

Ball* BallManager::RequestNewBall() {
	printf("new ball requested\n");
	for (int i=0; i < NUM_BALLS; ++i) {
		Ball& ball = balls_[i];
		if(!ball.is_visible()) {
			ball.Show();
			return &ball;
		}
	}
	return NULL;
}

Ball* BallManager::GrabBall(const XV3 ref_position) {
	int min_index = -1;
	
	for (int i =0 ; i< NUM_BALLS; ++i) {
		Ball& ball = balls_[i];
		if(!ball.is_visible()) {
			//if(juggling_mode_) return &ball; //return a ball straight away
			continue;
		}


		XV3 ball_pos = ball.GetPosition();
		//float dist = ref_position.distance(ball_pos );

		float dx = ref_position.X - ball_pos.X;
		float dy = ref_position.Y - ball_pos.Y;

		float dist = sqrt(dx*dx + dy*dy);

		//float dist = ref_position.distance(ball.GetPosition());
		//printf("dist = %g\n",dist);

		if(dist < GRAB_THRESHOLD) {
			return &ball;
		}
	}
	return NULL;
}

void BallManager::GetBallInfo(const int i, bool* enabled, XV3* position) const {
	if(i<0 || i > NUM_BALLS) return;

	*enabled = balls_[i].is_visible();
	(*position).assign(balls_[i].GetPosition());
}

void BallManager::Update() {
	float dt = time_ticker_.tick();

	XnUserID userID =user_detector_->getTrackedUserID();

	UpdateBalls(dt);

	if (!userID) {
		left_hand_.SetBall(NULL);
		right_hand_.SetBall(NULL);
		return;
	}

	UpdateHand(&left_hand_,dt);
	UpdateHand(&right_hand_,dt);
}

void BallManager::UpdateBalls(float dt) {
	const float g = 3000.0f; // mm/s

	for (int i=0 ; i < NUM_BALLS; ++i) {
		Ball& ball = balls_[i];
		if(!ball.is_visible()) continue;

		if(juggling_mode_) {
			ball.UpdateVelocity(dt,g, terminal_velocity_);
		}else{ 
			ball.UpdateVelocity(dt,g,0);
		}
		float y = ball.GetPosition().Y;

		if(y < -1000 || y > 3000 || fabs(ball.GetPosition().X) > 1000) {
			ball.Hide();
			printf("ball hidden\n");
			XV3 temp;
			temp.assign(balls_[0].GetPosition());
			printf("{%g, %g, %g}\n",temp.X,temp.Y,temp.Z);
		}
	}

		//XV3 temp;
		//temp.assign(balls_[0].GetPosition());

		//printf("{%g, %g, %g}\n",temp.X,temp.Y,temp.Z);

}

void BallManager::UpdateHand(Hand* hand, float dt) {
	if(hand == NULL) return;

	XnSkeletonJointPosition xn_joint_position;

	user_detector_->getSkeletonJointPosition(hand->joint_id(), &xn_joint_position);

	bool is_visible = isConfident(xn_joint_position);

	hand->Update(dt,is_visible);

	if(is_visible) {
		hand->AddDataPoint(dt, xn_joint_position.position);

		if(hand->GetBallRequestFlag()) {
			printf("get new ball for hand\n");
			hand->SetBall(RequestNewBall());
		}
	} else {
		//printf("hand is hidden\n");
		hand->SetBallRequestFlag(); //if no ball is being held then set the new_ball_request flag
		return;
	}

	//return;

	Ball* current_ball = hand->GetBall();

	if(current_ball) {
		float a = hand->GetAcceleration();
		
		XV3 release_velocity = current_ball->GetReleaseVelocity();

		bool acceleration_test = a < ACCELERATION_THRESHOLD;
		bool visibility_test = hand->GetVisibilityTime() > MIN_VISIBILITY_TIME;
		bool velocity_test = release_velocity.Y > MIN_VERTICAL_RELEASE_VELOCITY;

		//if(!acceleration_test) printf("acceleration too high\n");
		//if(!visibility_test) printf("not visible for long enough\n");
		//if(!velocity_test) printf("velocity too low\n");

		if(	acceleration_test && visibility_test && velocity_test)
		{
			//printf("acceleration = %g\n",a);
			hand->SetBall(NULL); //release ball from the hand

			if(juggling_mode_) release_velocity.Z = 0;

			current_ball->SetVelocity(release_velocity);

		} else {
			current_ball->Show();
			current_ball->SyncData(*hand);
		}
	} else if(hand->GetTimeWithoutBall() > MIN_TIME_WITHOUT_BALL) {
		//try and grab a new ball
		Ball* closest_ball = GrabBall(xn_joint_position.position);
		if(closest_ball) hand->SetBall(closest_ball);
	}
}