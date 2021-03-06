/**
 * @brief
 */

#include "matcher.h"

Matcher::Matcher(cv::Mat& img1, cv::Mat& img2, cv::Mat& depth1)
{
    //cv::imshow("gray image", img1);
    //cv::waitKey(0);
    orb = cv::ORB::create();
    bfmatcher = cv::BFMatcher::create(cv::NORM_HAMMING);

    extractORB(img1, kpts1, descriptors1);// image1
    extractORB(img2, kpts2, descriptors2);// image2

    find_feature_matches();

    construct3d_to_2d(good_matches, depth1);
}

void Matcher::extractORB(cv::Mat& im, std::vector<cv::KeyPoint>& kpts,
                         cv::Mat& descriptors)
{   
    // extract keypoint and descriptors
    orb->detectAndCompute(im, cv::Mat(), kpts, descriptors);
}

void Matcher::find_feature_matches()
{
    bfmatcher->match(descriptors1, descriptors2, matches);

    double min_dist=10000, max_dist=0;
    
    for(int i = 0; i < descriptors1.rows; i++)
    {
        double dist = matches[i].distance;
        if ( dist < min_dist ) 
            min_dist = dist;
        if ( dist > max_dist ) 
            max_dist = dist;
    }

    std::cout << "--Min dist: " << min_dist << std::endl;
    std::cout << "--Max dist: " << max_dist << std::endl;
    
    for(int i = 0; i < descriptors1.rows; i++)
    {
        if ( matches[i].distance <= std::max(2 * min_dist, 30.0 ) )
        {
            good_matches.push_back (matches[i]);
        }
    }
}

void Matcher::construct3d_to_2d(std::vector<cv::DMatch>& matches, cv::Mat& depth)
{
    for(cv::DMatch m:matches)
    {
        unsigned short d = depth.at<unsigned short>(int(kpts1[m.queryIdx].pt.y)/*row*/, int(kpts1[m.queryIdx].pt.x)/*col*/);
        if(d <= 0.0)
            continue;

        float z = d / depthfactor;
        cv::Point2d p1 = pixel2cam(kpts1[m.queryIdx].pt);
        pts_3d.push_back(cv::Point3d(p1.x * z, p1.y * z, z));
        pts_2d.push_back(kpts2[m.trainIdx].pt);
    }
}

cv::Mat Matcher::getCameraK()
{
    cv::Mat K = cv::Mat::eye(3, 3, CV_32F);
    K.at<float>(0,0) = fx;
    K.at<float>(1,1) = fy;
    K.at<float>(0,2) = cx;
    K.at<float>(1,2) = cy;

    return K;
}

cv::Point2d Matcher::pixel2cam(const cv::Point2d& p)
{
    return cv::Point2d((p.x - cx) / fx, (p.y - cy) / fy);
}

Matcher::~Matcher()
{

}
