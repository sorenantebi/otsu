#ifndef KMEANS_H
#define KMEANS_H

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <tuple>

// Algorithm K-Means
// --> Add 2 clusters to start, {initialized to the beginning of the sorted image for better convergence?}
// --> Assign each point in the image to a cluster, based on minimum distance between point and cluster
// --> the new cluster_index for each pixel is stored in the new_assignments array
// --> update the centroids: loop through all the points in the image and count the number of points assigned to 
// each cluster (points_in_cluster[cluster_index]++), cluster_index (0 or 1) from new_assignments[i] --> calculate total intensity in each cluster
// using the cluster_total_intensity[cluster_index] += points[i].intensity
// --> loop through each centroid and calculate average intensity --> set this as the new intensity
// next iteration
// kmeans++
struct Point {
    int intensity;
};

double distance(Point p1, Point p2) {
    return std::abs(p1.intensity - p2.intensity);
}

int assignCluster(Point point, const std::vector<Point>& centroids) {
    double min_dist = distance(point, centroids[0]);
    int cluster_index = 0;
    for (size_t i = 1; i < centroids.size(); ++i) {
        double dist = distance(point, centroids[i]);
        if (dist < min_dist) {
            min_dist = dist;
            cluster_index = i;
        }
    }
    return cluster_index;
}

void updateCentroids(const std::vector<Point>& points, std::vector<Point>& centroids, const std::vector<int>& new_assignments) {
    std::vector<int> points_in_cluster(centroids.size(), 0);
    std::vector<int> cluster_total_intensity(centroids.size(), 0);

    for (size_t i = 0; i < points.size(); ++i) {
        int cluster_index = new_assignments[i];
        points_in_cluster[cluster_index]++;
        cluster_total_intensity[cluster_index] += points[i].intensity;
    }
    
    for (size_t i = 0; i < centroids.size(); ++i) {
        if (points_in_cluster[i] > 0) {
            centroids[i].intensity = cluster_total_intensity[i] / points_in_cluster[i];
        }
    }
}

std::vector<int> getClusterStats(const std::vector<Point>& points, const std::vector<int>& new_assignments, int cluster_index) {
    int sum_intensity = 0;
    int count = 0;
    int min_intensity = std::numeric_limits<int>::max(); // Initialize min_intensity to the maximum possible value
    int max_intensity = std::numeric_limits<int>::min();
    for (size_t i = 0; i < points.size(); ++i) {
        if (new_assignments[i] == cluster_index) {
            sum_intensity += points[i].intensity;
            count++;
            if (points[i].intensity < min_intensity) {
                min_intensity = points[i].intensity;
            } else if (points[i].intensity >= max_intensity){
                max_intensity = points[i].intensity;
            }

        }
    }
    if (count > 0) {
        std::vector<int> vect;
        vect.push_back(sum_intensity / count);
        vect.push_back(count);
        vect.push_back(min_intensity);
        vect.push_back(max_intensity);
        return vect;
    } else {
        std::vector<int> vect;
        vect.push_back(0);
        vect.push_back(0);
        vect.push_back(0);
        return vect;
    }
}

void kMeans(const std::vector<Point>& points, int k, std::vector<Point>& centroids, std::vector<int>& new_assignments, int max_iterations) {
    // Initialize centroids randomly
    
    for (int i = 0; i < k; ++i) {
        centroids.push_back(points[i]);
    }
   
    // Iterate until convergence or max iterations
    for (int iter = 0; iter < max_iterations; ++iter) {
        // Assign points to nearest centroid
        new_assignments.clear();
        for (int i = 0; i < points.size(); i++) {
            new_assignments.push_back(assignCluster(points[i], centroids));
        }
        
        // Update centroids
        updateCentroids(points, centroids, new_assignments);
    }
    
}

#endif 