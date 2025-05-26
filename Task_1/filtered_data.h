#include <iostream>
#include <vector>
#include <cmath>

struct FilteredData {
    std::vector<double> filtered_x;
    std::vector<double> filtered_y;
};

FilteredData linearRegression(const std::vector<double>& x, const std::vector<std::vector<int> >& y, double& slope, double& intercept) {
    // Calculate the mean of x and y
    double zscore_threshold = 2.0; // can change to include more outliers
    double x_mean = 0.0, y_mean = 0.0;
    double y_mean_intensity = 0.0;
    double x_stddev = 0.0, y_stddev = 0.0;
    FilteredData filteredData;

    for (int i = 0; i<x.size(); i++){
        x_mean += x[i];
    }
    x_mean /= x.size();

    // average height
    for (int i = 0; i < y.size(); i++){
        y_mean += y[i][1];
    }
    y_mean /= y.size();
    
    // average intensity
    for (int i = 0; i < y.size(); i++){
        y_mean_intensity += y[i][0];
    }
    y_mean_intensity /= y.size();

    // STDEV
    // S = sqrt((E(x-mu)^2)/n)
    for (int i = 0; i < y.size(); i++){
        y_stddev += std::pow(y[i][0] - y_mean_intensity, 2);
    }
    y_stddev = std::sqrt(y_stddev / y.size());

    // Calculate z-scores for y (intensity)
    // z = (x - mu)/stdev
    std::vector<double> z_scores(y.size());
    for (size_t i = 0; i < y.size(); ++i) {
        z_scores[i] = (y[i][0] - y_mean_intensity) / y_stddev;
    }

    // Filter data points based on z-scores
    std::vector<double> filtered_x, filtered_y;
    for (size_t i = 0; i < y.size(); ++i) {
        if (std::abs(z_scores[i]) <= zscore_threshold) {
            filtered_x.push_back(x[i]);
            filtered_y.push_back(y[i][1]);
        }
    }
    
    // Perform linear regression on filtered data
    // Calculate slope
    // m = (nExy - ExEy)/(nEx^2-(Ex)^2)
    
    int n = filtered_x.size();
    double sum_x = 0.0, sum_y = 0.0, sum_xy =0.0, sum_x2=0.0; 

    for (int i = 0; i<n; ++i){
        sum_x += filtered_x[i];
    }

    for (int i = 0; i< n; ++i){
        sum_y += filtered_y[i];
    }

    for (int i = 0; i<n; ++i){
        sum_xy += filtered_x[i]*filtered_y[i];
    }

    for (int i = 0; i<n; ++i){
        sum_x2 += std::pow(filtered_x[i], 2);
    }

    slope = (n*(sum_xy)-(sum_x*sum_y))/(n*sum_x2-std::pow(sum_x,2));

    // Calculate intercept
    intercept = (sum_y-slope*sum_x)/n;

    filteredData.filtered_x = filtered_x;
    filteredData.filtered_y = filtered_y;

    return filteredData;
}
