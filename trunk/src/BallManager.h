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

#ifndef _BALL_MANAGER_H_
#define _BALL_MANAGER_H_

#include "Ball.h"
#include "Hand.h"
#include "vec.h"
#include "TimeTicker.h"

class UserDetector;

const int NUM_BALLS = 3;
const float ACCELERATION_THRESHOLD = -7000.0F;
const float GRAB_THRESHOLD = 100.0f;
const float MIN_VISIBILITY_TIME = 0.25f; //1 second - minimum time that hand must be continously visible before release is allowed
const float MIN_TIME_WITHOUT_BALL = 0.5f; //0.5 seconds - time to wait before allowing the hand to catch again
const float MIN_VERTICAL_RELEASE_VELOCITY = 1000.0f; //1 m/s upwards
const float TERMINAL_VELOCITY_DEFAULT = 500.0f;
const float TERMINAL_VELOCITY_INCREMENT = 100.0f;

class BallManager {
 public:
	 BallManager(UserDetector* user_detector);

	 virtual ~BallManager();

	 UserDetector* user_detector() const { return user_detector_;}

	 Ball* RequestNewBall();

	 void Update();
	 void GetBallInfo(const int i, bool* enabled, XV3* position) const;

	 void IncreaseFloatFactor() {
		 terminal_velocity_ -= TERMINAL_VELOCITY_INCREMENT;
		 printf("Max descent speed decreased to %g. Balls will now float more\n",terminal_velocity_);
	 }
	 void DecreaseFloatFactor() {
		 terminal_velocity_ += TERMINAL_VELOCITY_INCREMENT;
		 printf("Max descent speed increased to %g. Balls will now float less\n",terminal_velocity_);
	 }

	 void ToggleJugglingMode() {
		if(juggling_mode_) {
			juggling_mode_ = false;
			printf("Juggling mode disabled\n");
		} else {
			juggling_mode_ = true;
			printf("Juggling mode enabled\n");
		}
		left_hand_.SetBall(NULL);
		right_hand_.SetBall(NULL);
	 }

 private:
	TimeTicker time_ticker_;

	Ball balls_[NUM_BALLS];
	Hand left_hand_;
	Hand right_hand_;
	UserDetector* user_detector_;
	bool juggling_mode_;
	float terminal_velocity_;

	Ball* GrabBall(const XV3 ref_position);
	void UpdateHand(Hand* hand,float dt);
	void UpdateBalls(float dt);
};

#endif

