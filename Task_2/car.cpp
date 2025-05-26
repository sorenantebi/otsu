#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include "../stb_image/stb_image_write.h"
#include "../stb_image/stb_image.h"
#include "kmeans.h"
#include <vector>
#include <cmath>

int MAX_INTENSITY = 255;

// Maximize variance between classes, by iterating over each part of the intensity histogram
// find intensity value that acts as a threshold to maximize variance between the two classes
int otsu_threshold(const std::vector<unsigned char>& sorted_pixels, int width, int height) {
    long int N = width * height;
    int threshold = 0;
    float sum = 0;
    float sumB = 0;
    int q1 = 0;
    int q2 = 0;
    float varMax = 0;
    std::vector<float> hist(256, 0);

    // Compute histogram
    for (int i = 0; i < N; i++) {
        int value = (int) sorted_pixels[i];
        hist[value]++;
    }

    // Auxiliary value for computing m2
    for (int i = 0; i <= MAX_INTENSITY; i++){
      sum += i * ((int)hist[i]);
    }

    for (int i = 0 ; i <= MAX_INTENSITY ; i++) {
      // Update q1
      q1 += hist[i]; // pixels up till i hist distribution
      if (q1 == 0)
        continue;
      // Update q2
      q2 = N - q1; // rest of the pixels

      if (q2 == 0) // go until you cover whole histogram
        break;
      // Update m1 and m2 --> calculate the mean intensity, weight contribution of each intensity to total sum
      // --> to calculate the mean 
      sumB += (float) (i * ((int)hist[i]));
      float m1 = sumB / q1;
      float m2 = (sum - sumB) / q2;

      // Update the between class variance
      float varBetween = (float) q1 * (float) q2 * (m1 - m2) * (m1 - m2);

      // Update the threshold if necessary
      if (varBetween > varMax) {
        varMax = varBetween;
        threshold = i;
      }
    }
    return threshold;
}
    
void sort_row_pixels(std::vector<unsigned char>& pixels, int width, int height) {
    for (int y = 0; y < height; ++y) {
        std::vector<unsigned char>::iterator row_start = pixels.begin() + y * width;
        std::vector<unsigned char>::iterator row_end = row_start + width;
        std::sort(row_start, row_end, std::greater<unsigned char>());
    }
}

void sort_image(std::vector<unsigned char>& pixels, int width, int height){
    std::sort(pixels.begin(), pixels.end(), std::greater<unsigned char>());
}

void manual_threshold(unsigned char* image, int width, int height, int channels){
    std::vector<unsigned char> dark_areas_green(image, image + width * height * channels);
    
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_green[3*i+1] < 55){
            dark_areas_green[3*i] = 0;
            dark_areas_green[3*i+1] = 0;
            dark_areas_green[3*i+2] = 0;
        }
    }

    stbi_write_png("dark_areas_green_manual.png", width, height, 3, dark_areas_green.data(), width*channels);

    std::vector<unsigned char> dark_areas_red(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_red[3*i] < 40){
            dark_areas_red[3*i] = 0;
            dark_areas_red[3*i+1] = 0;
            dark_areas_red[3*i+2] = 0;
        }
    }

    stbi_write_png("dark_areas_red_manual.png", width, height, 3, dark_areas_red.data(), width*channels);


    std::vector<unsigned char> dark_red_light_green(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (!(dark_red_light_green[3*i] < 40 && dark_red_light_green[3*i+1]>55)){
            dark_red_light_green[3*i] = 0;
            dark_red_light_green[3*i+1] = 0;
            dark_red_light_green[3*i+2] = 0;
        }
    }

    stbi_write_png("dark_red_light_green_manual.png", width, height, 3, dark_red_light_green.data(), width*channels);

}


// Notes
    // maybe add morphological post processing --> dilation --> add pixel to edges of objects, erosion
    // use kmeans to find lowest intensity in the cluster + use that as the threshold
    // maybe only include certain intensities a certain deviation from the mean?
    // gaussian smoothing?
    // maybe use a gaussian mixture model
    // canny edge detection? --> gaussian smoothing --> grad mag w/ sobel/prewitt filters --> NMS --> hysteresis thresholding
    // how to test? measure IoU
    // maybe increase contrast in image --> adaptive histogram equalization or CLAHE
    // median filtering to filter high frequency noise?
  
   

int main() {
    // Read the image
    const char* filename = "car.png";
  
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // GRAYSCALE                                                                                                              //
    // grayscale threshold --> can take average of each pixel val = (r+g+b)/3 and set image[i], image[i+1], image[i+2] = val  //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int gr_width, gr_height, gr_original_channels;
    int gr_channels = 1;
    unsigned char* grayscale = stbi_load(filename, &gr_width, &gr_height, &gr_original_channels, gr_channels);
   
    // Check if the image was loaded successfully
    if (!grayscale) {
        std::cerr << "Failed to load image" << std::endl;
        return -1;
    }
    // Print image information
    std::cout << "Image width: " << gr_width << ", height: " << gr_height << ", channels: " << gr_channels << std::endl;

    std::vector<unsigned char> gr_pixels(grayscale, grayscale + gr_width * gr_height * gr_channels);

    // Sort grayscale by row descending
    sort_row_pixels(gr_pixels, gr_width, gr_height);
    stbi_write_jpg("sorted_row_grayscale.jpg", gr_width, gr_height, gr_channels, gr_pixels.data(), gr_width * gr_channels);

    // Sort grayscale descending
    sort_image(gr_pixels, gr_width, gr_height);
    stbi_write_jpg("sorted_grayscale.jpg", gr_width, gr_height, gr_channels, gr_pixels.data(), gr_width * gr_channels);

    int threshold = otsu_threshold(gr_pixels,gr_width,gr_height);
    std::cout << "Otsu grayscale: " << threshold << std::endl;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // RGB                                                                                                   //
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
  
    int width, height, original_channels;
    int channels = 3;
    unsigned char* image = stbi_load(filename, &width, &height, &original_channels, channels);

    // Check if the image was loaded successfully
    if (!image) {
        std::cerr << "Failed to load image" << std::endl;
        return -1;
    }

    std::cout << "Image width: " << width << ", height: " << height << ", channels: " << channels << std::endl;

    std::vector<unsigned char> pixels(image, image + width * height * channels);

    std::vector<unsigned char> red_channel, green_channel, blue_channel;
    for (size_t i = 0; i < pixels.size(); i += 3) {
        red_channel.push_back(pixels[i]);
        green_channel.push_back(pixels[i + 1]);
        blue_channel.push_back(pixels[i + 2]);
    }

    sort_row_pixels(green_channel, width, height);
    sort_row_pixels(red_channel,width,height);
   
    stbi_write_png("row_sorted_green.png", width, height, 1, green_channel.data(), width);
    stbi_write_png("row_sorted_red.png", width, height, 1, red_channel.data(), width);

    sort_image(green_channel, width, height);
    sort_image(red_channel, width, height);

    stbi_write_png("image_sorted_green.png", width, height, 1, green_channel.data(), width);
    stbi_write_png("image_sorted_red.png", width, height, 1, red_channel.data(), width);

    int green_threshold = otsu_threshold(green_channel, width, height);
    std::cout << "Otsu green: " << green_threshold << std::endl;
    int red_threshold = otsu_threshold(red_channel, width, height);
    std::cout << "Otsu red: " << red_threshold << std::endl;

    std::vector<unsigned char> dark_areas_green(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_green[3*i+1] < green_threshold){
            dark_areas_green[3*i] = 0;
            dark_areas_green[3*i+1] = 0;
            dark_areas_green[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_areas_green.png", width, height, 3, dark_areas_green.data(), width*channels);


    std::vector<unsigned char> dark_areas_red(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_red[3*i] < red_threshold){
            dark_areas_red[3*i] = 0;
            dark_areas_red[3*i+1] = 0;
            dark_areas_red[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_areas_red.png", width, height, 3, dark_areas_red.data(), width*channels);


    std::vector<unsigned char> dark_red_light_green(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (!(dark_red_light_green[3*i] < red_threshold && dark_red_light_green[3*i+1]>green_threshold)){
            dark_red_light_green[3*i] = 0;
            dark_red_light_green[3*i+1] = 0;
            dark_red_light_green[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_red_light_green.png", width, height, 3, dark_red_light_green.data(), width*channels);

    /////////////////////////////////////////////////////////////////
    // Using Otsu threshold from grayscale image on rgb image      //
    /////////////////////////////////////////////////////////////////

    std::vector<unsigned char> dark_areas_green_gr(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_green_gr[3*i+1] < threshold){
            dark_areas_green_gr[3*i] = 0;
            dark_areas_green_gr[3*i+1] = 0;
            dark_areas_green_gr[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_areas_green_gr.png", width, height, 3, dark_areas_green_gr.data(), width*channels);


    std::vector<unsigned char> dark_areas_red_gr(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_red_gr[3*i] < threshold){
            dark_areas_red_gr[3*i] = 0;
            dark_areas_red_gr[3*i+1] = 0;
            dark_areas_red_gr[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_areas_red_gr.png", width, height, 3, dark_areas_red_gr.data(), width*channels);


    std::vector<unsigned char> dark_red_light_green_gr(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (!(dark_red_light_green_gr[3*i] < threshold && dark_red_light_green_gr[3*i+1]>threshold)){
            dark_red_light_green_gr[3*i] = 0;
            dark_red_light_green_gr[3*i+1] = 0;
            dark_red_light_green_gr[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_red_light_green_gr.png", width, height, 3, dark_red_light_green_gr.data(), width*channels);


    
    /////////////////////////////////////////////////////////////////
    // KMEANS                                                      //
    /////////////////////////////////////////////////////////////////
    
    std::vector<Point> red_points;
    for (int i = 0; i < width * height; ++i) {
        Point p;
        p.intensity = red_channel[i]; // Assuming RGB order
        red_points.push_back(p);
    }

    std::vector<Point> green_points;
    for (int i = 0; i < width * height; ++i) {
        Point p;
        p.intensity = green_channel[i]; // Assuming RGB order
        green_points.push_back(p);
    }

    // Number of clusters
    int k = 2;

    // red_centroids and red_assignments
    std::vector<Point> red_centroids;
    std::vector<int> red_assignments;
    std::vector<Point> green_centroids;
    std::vector<int> green_assignments;

    // Maximum iterations
    int max_iterations = 100;

    // Perform k-means clustering
    kMeans(red_points, k, red_centroids, red_assignments, max_iterations);
    kMeans(green_points, k, green_centroids, green_assignments, max_iterations);
    int red_intensity_threshold = -1;
    int green_intensity_threshold = -1;

    // Generate a new image using the cluster red_centroids
    for (int i = 0; i < k; ++i) {
        std::vector<int> result = getClusterStats(red_points, red_assignments, i);
        int intensity = result[0];
        int count = result[1];
        int min_intensity = result[2];
        int max_intensity = result[3];
        red_intensity_threshold = std::max(red_intensity_threshold, min_intensity);
        std::cout << "Cluster " << i << " intensity: " << intensity << " (count: " << count << ") " << "Min intensity: " << min_intensity << " Max intensity: " << max_intensity << std::endl;
    }

    for (int i = 0; i < k; ++i) {
        std::vector<int> result = getClusterStats(green_points, green_assignments, i);
        int intensity = result[0];
        int count = result[1];
        int min_intensity = result[2];
        int max_intensity = result[3];
        green_intensity_threshold = std::max(green_intensity_threshold, min_intensity);
        std::cout << "Cluster " << i << " intensity: " << intensity << " (count: " << count << ") " << "Min intensity: " << min_intensity << " Max intensity: " << max_intensity << std::endl;
    }

    // Save the resulting image
    unsigned char* outputData = new unsigned char[width * height];
    for (int i = 0; i < width * height; ++i) {
        outputData[i] = red_centroids[red_assignments[i]].intensity;
    }
    stbi_write_png("output_image_red.png", width, height, 1, outputData, width);
    for (int i = 0; i < width * height; ++i) {
        outputData[i] = green_centroids[green_assignments[i]].intensity;
    }
    stbi_write_png("output_image_green.png", width, height, 1, outputData, width);


    std::vector<unsigned char> dark_areas_green_km(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_green_km[3*i+1] < green_intensity_threshold){
            dark_areas_green_km[3*i] = 0;
            dark_areas_green_km[3*i+1] = 0;
            dark_areas_green_km[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_areas_green_km.png", width, height, 3, dark_areas_green_km.data(), width*channels);


    std::vector<unsigned char> dark_areas_red_km(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (dark_areas_red_km[3*i] < red_intensity_threshold){
            dark_areas_red_km[3*i] = 0;
            dark_areas_red_km[3*i+1] = 0;
            dark_areas_red_km[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_areas_red_km.png", width, height, 3, dark_areas_red_km.data(), width*channels);


    std::vector<unsigned char> dark_red_light_green_km(image, image + width * height * channels);
    for (int i = 0; i < width * height; ++i) {
        if (!(dark_red_light_green_km[3*i] < red_intensity_threshold && dark_red_light_green_km[3*i+1]>green_intensity_threshold)){
            dark_red_light_green_km[3*i] = 0;
            dark_red_light_green_km[3*i+1] = 0;
            dark_red_light_green_km[3*i+2] = 0;
        }
    }
    stbi_write_png("dark_red_light_green_km.png", width, height, 3, dark_red_light_green_km.data(), width*channels);

    manual_threshold(image, width, height, channels); 

    stbi_image_free(image);
    stbi_image_free(grayscale);
}