#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iostream>
#include "../stb_image/stb_image_write.h"
#include "../stb_image/stb_image.h"
#include <vector>
#include "filtered_data.h"



int main() {
    // Read the image
    const char* filename = "diffuse.png";
    int width, height, original_channels;
    int channels = 3;
    unsigned char* image = stbi_load(filename, &width, &height, &original_channels, channels);
    
    // Check if the image was loaded successfully
    if (!image) {
        std::cerr << "Failed to load image" << std::endl;
        return -1;
    }

    // Print image information
    std::cout << "Image width: " << width << ", height: " << height << ", channels: " << channels << std::endl;
    
    // Store information in 2d vector --> max_values[column][pixel intensity, y value of the pixel]
    std::vector<std::vector<int> > max_values(width, std::vector<int>(2));

    // Access pixel values
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            // Calculate index in the 1D array
            int index = (y * width + x) * channels;
            if (max_values[x][0] < (int)image[index]){
                max_values[x][0] = (int)image[index];
                max_values[x][1] = y;
            }    
        }
    }
    
    // Find average y value for max intensity in each column
    for (int x = 0; x < width; ++x) {
        int sum = 0;
        int divisor = 0;
        for (int y = 0; y < height; ++y) {
            if (image[(y * width + x)*channels] == max_values[x][0]){
                sum += y;
                divisor++;
            }
        }
        max_values[x][1] = sum/divisor;
    }

    std::vector<double> x_data(width);
    std::vector<std::vector<int> > y_data;

    for (int i = 0; i < width; ++i) {
        x_data[i] = i;
        std::vector<int> temp_row;
        temp_row.push_back(max_values[i][0]);
        temp_row.push_back(max_values[i][1]);
        y_data.push_back(temp_row);
    }

    // Perform linear regression
    double slope, intercept;
    FilteredData filteredData = linearRegression(x_data, y_data, slope, intercept);
    std::cout << "Slope: " << slope << ", intercept: " << intercept << std::endl;

    // Plot max_values of each column
    for (int x = 0; x < filteredData.filtered_x.size(); ++x) {
        image[(int)(filteredData.filtered_y[x] * width +  filteredData.filtered_x[x]) * channels] = 255; // Set pixel to red
        image[(int)(filteredData.filtered_y[x] * width +  filteredData.filtered_x[x]) * channels + 1] = 0;
        image[(int)(filteredData.filtered_y[x] * width +  filteredData.filtered_x[x]) * channels + 2] = 0;
    }

    // Plot the regression line
    
    for (int x = 0; x < width; ++x) {
        int y = slope * x + intercept;
        if (y >= 0 && y < height) {
            int index = (y * width + x) * channels;
            image[index] = 255; // Set pixel to white
            image[index + 1] = 255;
            image[index + 2] = 255;
        }
    }
    
    // Save the modified image with maximum points marked
    stbi_write_jpg("image_with_max_points.jpg", width, height, channels, image, width * channels);

   
    // Free the image memory
    stbi_image_free(image);

    return 0;
}