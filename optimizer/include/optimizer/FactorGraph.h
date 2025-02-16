#pragma once

struct NoiseModels {
    gtsam::noiseModel::Diagonal::shared_ptr covariance_X;
    gtsam::noiseModel::Diagonal::shared_ptr covariance_U;
    gtsam::noiseModel::Diagonal::shared_ptr covariance_ternary;
    gtsam::noiseModel::Diagonal::shared_ptr covariance_obs;
    gtsam::noiseModel::Diagonal::shared_ptr covariance_priors;
};

std::pair<gtsam::NonlinearFactorGraph, gtsam::Values> CentroidFG(int max_states_,
    const CentroidData& centroid, // pass by const reference
    const NoiseModels& covariance_information, const SDF_s& sdf_) { 

    gtsam::Values init_values;
    gtsam::NonlinearFactorGraph graph;

    bool use_sdf = true;
    auto& covariance_X = covariance_information.covariance_X;
    auto& covariance_U = covariance_information.covariance_U;
    auto& covariance_ternary = covariance_information.covariance_ternary;
    auto& covariance_obs = covariance_information.covariance_obs;
    auto& covariance_anchors = covariance_information.covariance_priors;

    int max_states = max_states_;
    std::vector<obstacle> modifiable_obstacles = sdf_.obstacles;
    
    for (int j = 0; j < max_states; ++j) {
        for(auto&& obstacle : modifiable_obstacles) {
            // check if the current trajectory will collide with the obstacle
            // make the distance calculation faster by using x*x not std::pow
            double distance_to_obstacle = (centroid.X_k_ref[j].x() - obstacle.x) * (centroid.X_k_ref[j].x() - obstacle.x) + (centroid.X_k_ref[j].y() - obstacle.y) * (centroid.X_k_ref[j].y() - obstacle.y);
            // if so, set the weight to 1.0
            obstacle.w = (distance_to_obstacle < (sdf_.sys_radius_safety_radius_squared)) ? 1.0 : 0.0;

        //     if ((cx + safety_radius >= obstacle.minX) &&
        //        (cx - safety_radius <= obstacle.maxX) &&
        //        (cy + safety_radius >= obstacle.minY) &&
        //        (cy - safety_radius <= obstacle.maxY)) {
        //        // If within expanded bounding box, set the weight to 1.0
        //        obstacle.w = 1.0;
        //    } else {
        //        // If outside expanded bounding box, set the weight to 0.0
        //        obstacle.w = 0.0;
        //    }
        }

        gtsam::Symbol key_pos_c('C', j);
        gtsam::Symbol key_control_c('U', j);
        gtsam::Symbol key_pos_next_c('C', j + 1);

        if (j == 0) {
            graph.add(NonlinearEquality<Pose2>(key_pos_c, centroid.X_k_real));
            init_values.insert(key_pos_c, centroid.X_k_real);
        } else {
            for (auto &&obstacle : modifiable_obstacles) {
                if (obstacle.w > 0.0) {
                    graph.add(boost::make_shared<UnaryFactorObstacleAvoidance>(key_pos_next_c, obstacle, sdf_.sys_radius_safety_radius, sdf_.inv_sys_radius_safety_radius, covariance_obs));
                }
            }
            graph.add(PriorFactor<Pose2>(key_pos_c, centroid.X_k_ref[j], covariance_X));
            init_values.insert(key_pos_c, centroid.X_k_ref[j]);
        }

        graph.add(PriorFactor<Pose2>(key_control_c, centroid.U_k_ref[j], covariance_U));
        init_values.insert(key_control_c, centroid.U_k_ref[j]); 
        graph.add(boost::make_shared<TernaryFactorStateEstimation>(key_pos_c, key_control_c, key_pos_next_c, covariance_ternary));

        if (j == (max_states - 1)) {
            graph.add(PriorFactor<Pose2>(key_pos_next_c, centroid.X_k_ref[j + 1], covariance_anchors));
            init_values.insert(key_pos_next_c, centroid.X_k_ref[j + 1]);
        }
    }
    // graph.print("Factor Graph: \n");
    return std::make_pair(graph, init_values);
}
