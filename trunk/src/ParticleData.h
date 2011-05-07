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

#ifndef _PARTICLE_DATA_H_
#define _PARTICLE_DATA_H_

#include "common.h"
#include "vec.h"

const int MAX_DATA_POINTS=2;

class ParticleData {
 public:
		int num_data_points_;
		float dt_[MAX_DATA_POINTS];
		XV3 x_[MAX_DATA_POINTS];
		XV3 v_[MAX_DATA_POINTS];

		ParticleData() {Reset();}

		XV3 GetAccelerationVector() const {
			if(num_data_points_ < 2) return XV3();
			return (v_[0]-v_[1])/dt_[0];
		}

		float GetAcceleration() const {
			if(num_data_points_ < 2) return 0.0F;
			return (v_[0].magnitude()-v_[1].magnitude())/dt_[0];
		}

		//float GetVerticalVelocity() const {
		//	return v_[0].Z;
		//}

		
		
		XV3 GetReleaseVelocity() {
			return v_[1];

			//override the most recent velocity value with the largest value
			//ie the maximum that would have been achieved prior to rapid deceleration
			float v_max = v_[0].magnitude2();
			int i_max = 0;

			for (int i=1; i < num_data_points_; ++i) {
				float v = v_[i].magnitude2();
				if(v > v_max) {
					v_max = v;
					i_max = i;
				}
			}
			return v_[i_max];
		}

		void SetVelocity(XV3 v) {
			v_[0] = v;
		}

		XV3 GetPosition() const { 
			return x_[0];
		}

		void SyncData(const ParticleData& source) {
			num_data_points_ = source.num_data_points_;

			for (int i=0; i < num_data_points_; ++i) {
				x_[i] = source.x_[i];
				v_[i] = source.v_[i];
				dt_[i] = source.dt_[i];
			}
		}

		void AddDataPoint(float dt, const XV3& x) {
			if(dt == 0) {
				x_[0] = x;
				v_[0].zero();
				dt_[0] = 0;
				num_data_points_ = 1;
				return;
			}

			//shuffle the list along
			for (int i=0; i < num_data_points_-1; ++i) {
				x_[i+1] = x_[i];
				v_[i+1] = v_[i];
				dt_[i+1] = dt_[i];
			}
			
			x_[0] = x;
			if(dt > 0) {
				v_[0] = (x_[0]-x_[1])/dt;
			} else {
				v_[0].zero();
			}
			dt_[0] = dt;

			if(num_data_points_ < MAX_DATA_POINTS) num_data_points_++;
		}

		void Reset() {
			num_data_points_ = 0;
			x_[0].zero(); x_[1].zero();
			v_[0].zero(); v_[1].zero();
		}
};

#endif