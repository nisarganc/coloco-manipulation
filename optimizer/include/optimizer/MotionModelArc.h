#pragma once

#include "Utils.h"

namespace MotionModelArc
{
    double Ts = 0.1;
    
    gtsam::Pose2 centroid_solveForNextPose(gtsam::Pose2 current_centroid_pose, gtsam::Pose2 current_centroid_speed) {

        const double x_now = current_centroid_pose.x();
        const double y_now = current_centroid_pose.y();
        double theta_now = current_centroid_pose.theta();

        const double x_dot = current_centroid_speed.x();
        const double y_dot = current_centroid_speed.y();
        const double theta_dot = current_centroid_speed.theta();

        // Initialize next pose values to current values
        double x_next = x_now;
        double y_next = y_now;
        double theta_next = theta_now;

        x_next += x_dot * Ts; // ToDo: time_for_translation;
        y_next += y_dot * Ts; // ToDo: time_for_translation;
        theta_next = Utils::ensure_orientation_range2(theta_dot * Ts); // ToDo: time_for_centroid_rotation;

        return gtsam::Pose2(x_next, y_next, theta_next);
    }

    gtsam::Pose2 centroid_solveForControl(gtsam::Pose2 current_centroid_pose, gtsam::Pose2 next_centroid_pose)
    {
        
        double x_now = current_centroid_pose.x();
        double y_now = current_centroid_pose.y();
        double theta_now = current_centroid_pose.theta();
        
        double x_next = next_centroid_pose.x();
        double y_next = next_centroid_pose.y();
        double theta_next = next_centroid_pose.theta();

        double x_dot, y_dot, theta_dot;

        x_dot = (x_next - x_now) / Ts;
        y_dot = (y_next - y_now) / Ts;
        theta_dot = Utils::ensure_orientation_range2((theta_next - theta_now) / Ts);

        return gtsam::Pose2(x_dot, y_dot, theta_dot);
    }
    
}