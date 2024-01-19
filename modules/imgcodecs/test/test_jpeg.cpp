// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html
#include <stdio.h>
#include "jpeglib.h"
#include "test_precomp.hpp"

#include <chrono>
namespace opencv_test { namespace {

#ifdef HAVE_JPEG

/**
 * Test for check whether reading exif orientation tag was processed successfully or not
 * The test info is the set of 8 images named testExifRotate_{1 to 8}.jpg
 * The test image is the square 10x10 points divided by four sub-squares:
 * (R corresponds to Red, G to Green, B to Blue, W to white)
 * ---------             ---------
 * | R | G |             | G | R |
 * |-------| - (tag 1)   |-------| - (tag 2)
 * | B | W |             | W | B |
 * ---------             ---------
 *
 * ---------             ---------
 * | W | B |             | B | W |
 * |-------| - (tag 3)   |-------| - (tag 4)
 * | G | R |             | R | G |
 * ---------             ---------
 *
 * ---------             ---------
 * | R | B |             | G | W |
 * |-------| - (tag 5)   |-------| - (tag 6)
 * | G | W |             | R | B |
 * ---------             ---------
 *
 * ---------             ---------
 * | W | G |             | B | R |
 * |-------| - (tag 7)   |-------| - (tag 8)
 * | B | R |             | W | G |
 * ---------             ---------
 *
 *
 * Every image contains exif field with orientation tag (0x112)
 * After reading each image the corresponding matrix must be read as
 * ---------
 * | R | G |
 * |-------|
 * | B | W |
 * ---------
 *
 */

typedef testing::TestWithParam<string> Imgcodecs_Jpeg_Exif;

TEST_P(Imgcodecs_Jpeg_Exif, exif_orientation)
{
    const string root = cvtest::TS::ptr()->get_data_path();
    const string filename = root + GetParam();
    const int colorThresholdHigh = 250;
    const int colorThresholdLow = 5;

    Mat m_img = imread(filename);
    ASSERT_FALSE(m_img.empty());
    Vec3b vec;

    //Checking the first quadrant (with supposed red)
    vec = m_img.at<Vec3b>(2, 2); //some point inside the square
    EXPECT_LE(vec.val[0], colorThresholdLow);
    EXPECT_LE(vec.val[1], colorThresholdLow);
    EXPECT_GE(vec.val[2], colorThresholdHigh);

    //Checking the second quadrant (with supposed green)
    vec = m_img.at<Vec3b>(2, 7);  //some point inside the square
    EXPECT_LE(vec.val[0], colorThresholdLow);
    EXPECT_GE(vec.val[1], colorThresholdHigh);
    EXPECT_LE(vec.val[2], colorThresholdLow);

    //Checking the third quadrant (with supposed blue)
    vec = m_img.at<Vec3b>(7, 2);  //some point inside the square
    EXPECT_GE(vec.val[0], colorThresholdHigh);
    EXPECT_LE(vec.val[1], colorThresholdLow);
    EXPECT_LE(vec.val[2], colorThresholdLow);
}

const string exif_files[] =
{
    "readwrite/testExifOrientation_1.jpg",
    "readwrite/testExifOrientation_2.jpg",
    "readwrite/testExifOrientation_3.jpg",
    "readwrite/testExifOrientation_4.jpg",
    "readwrite/testExifOrientation_5.jpg",
    "readwrite/testExifOrientation_6.jpg",
    "readwrite/testExifOrientation_7.jpg",
    "readwrite/testExifOrientation_8.jpg"
};

INSTANTIATE_TEST_CASE_P(ExifFiles, Imgcodecs_Jpeg_Exif,
                        testing::ValuesIn(exif_files));

//==================================================================================================

TEST(Imgcodecs_Jpeg, encode_empty)
{
    cv::Mat img;
    std::vector<uchar> jpegImg;
    ASSERT_THROW(cv::imencode(".jpg", img, jpegImg), cv::Exception);
}

TEST(Imgcodecs_Jpeg, encode_decode_progressive_jpeg)
{
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    std::vector<int> params;
    params.push_back(IMWRITE_JPEG_PROGRESSIVE);
    params.push_back(1);

    string output_progressive = cv::tempfile(".jpg");
    EXPECT_NO_THROW(cv::imwrite(output_progressive, img, params));
    cv::Mat img_jpg_progressive = cv::imread(output_progressive);

    string output_normal = cv::tempfile(".jpg");
    EXPECT_NO_THROW(cv::imwrite(output_normal, img));
    cv::Mat img_jpg_normal = cv::imread(output_normal);

    EXPECT_EQ(0, cvtest::norm(img_jpg_progressive, img_jpg_normal, NORM_INF));

    EXPECT_EQ(0, remove(output_progressive.c_str()));
    EXPECT_EQ(0, remove(output_normal.c_str()));
}

TEST(Imgcodecs_Jpeg, encode_decode_optimize_jpeg)
{
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    std::vector<int> params;
    params.push_back(IMWRITE_JPEG_OPTIMIZE);
    params.push_back(1);

    string output_optimized = cv::tempfile(".jpg");
    EXPECT_NO_THROW(cv::imwrite(output_optimized, img, params));
    cv::Mat img_jpg_optimized = cv::imread(output_optimized);

    string output_normal = cv::tempfile(".jpg");
    EXPECT_NO_THROW(cv::imwrite(output_normal, img));
    cv::Mat img_jpg_normal = cv::imread(output_normal);

    EXPECT_EQ(0, cvtest::norm(img_jpg_optimized, img_jpg_normal, NORM_INF));

    EXPECT_EQ(0, remove(output_optimized.c_str()));
    EXPECT_EQ(0, remove(output_normal.c_str()));
}

TEST(Imgcodecs_Jpeg, encode_decode_rst_jpeg)
{
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    std::vector<int> params;
    params.push_back(IMWRITE_JPEG_RST_INTERVAL);
    params.push_back(1);

    string output_rst = cv::tempfile(".jpg");
    EXPECT_NO_THROW(cv::imwrite(output_rst, img, params));
    cv::Mat img_jpg_rst = cv::imread(output_rst);

    string output_normal = cv::tempfile(".jpg");
    EXPECT_NO_THROW(cv::imwrite(output_normal, img));
    cv::Mat img_jpg_normal = cv::imread(output_normal);

    EXPECT_EQ(0, cvtest::norm(img_jpg_rst, img_jpg_normal, NORM_INF));

    EXPECT_EQ(0, remove(output_rst.c_str()));
    EXPECT_EQ(0, remove(output_normal.c_str()));
}

void testTranscodeOne(const cv::Mat& original, cv::Size targetSize, int batches, cv::ImwriteJPEGSamplingFactorParams samplingFactor = cv::IMWRITE_JPEG_SAMPLING_FACTOR_420) {
    std::vector<uchar> outputNonbatched, outputBatched;

    cv::Mat sourceImage;
    cv::resize(original, sourceImage, targetSize);

    std::chrono::high_resolution_clock::time_point startNonbatched = std::chrono::high_resolution_clock::now();
    cv::imencode(".jpg", sourceImage, outputNonbatched, { cv::IMWRITE_JPEG_SAMPLING_FACTOR, samplingFactor });
    std::chrono::high_resolution_clock::time_point endNonbatched = std::chrono::high_resolution_clock::now();
    cv::Mat decodedNonbatched = cv::imdecode(outputNonbatched, cv::IMREAD_COLOR);

    std::chrono::high_resolution_clock::time_point startBatched = std::chrono::high_resolution_clock::now();
    cv::imencode(".jpg", sourceImage, outputBatched, { cv::IMWRITE_JPEG_RST_INTERVAL, batches, cv::IMWRITE_JPEG_SAMPLING_FACTOR, samplingFactor });
    std::chrono::high_resolution_clock::time_point endBatched = std::chrono::high_resolution_clock::now();
    cv::Mat decodedBatched = cv::imdecode(outputBatched, cv::IMREAD_COLOR);

    auto durationNonbatched = std::chrono::duration_cast<std::chrono::microseconds>(endNonbatched - startNonbatched).count();
    auto durationBatched = std::chrono::duration_cast<std::chrono::microseconds>(endBatched - startBatched).count();
    std::cout << "Speed-up (times): " << durationNonbatched / (double)durationBatched << std::endl;

    ASSERT_EQ(sourceImage.size(), decodedNonbatched.size());
    ASSERT_EQ(0, cvtest::norm(decodedNonbatched, decodedBatched, NORM_INF));

    decodedNonbatched.convertTo(decodedNonbatched, CV_16S);
    decodedBatched.convertTo(decodedBatched, CV_16S);
    auto diff = cv::abs(decodedNonbatched - decodedBatched);
    double maxDiff;
    cv::minMaxLoc(diff, nullptr, &maxDiff);
    ASSERT_EQ(0, maxDiff);

    // TODO: test grayscale (cinfo->comps_in_scan == 1)
    // it may require us to set iMCU_row_num on the batches
}

void testTranscodeAllColorspaces(const cv::Mat& bgr, cv::Size targetSize, int batches) {
    static const std::array<cv::ImwriteJPEGSamplingFactorParams, 5> samplingModes = {
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_411,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_420,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_422,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_440,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_444
    };

    for (auto samplingMode : samplingModes) {
        testTranscodeOne(bgr, targetSize, batches, samplingMode);
        cv::Mat gray;
        cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
        testTranscodeOne(gray, targetSize, batches, samplingMode);
        cv::Mat bgra;
        cv::cvtColor(bgr, bgra, cv::COLOR_BGR2BGRA);
        testTranscodeOne(bgra, targetSize, batches, samplingMode);
    }
}

void writeOne(const cv::Mat& original, cv::Size targetSize, const std::string& fileName, int batches) {
    std::vector<uchar> outputBatched;

    cv::Mat sourceImage;
    cv::resize(original, sourceImage, targetSize);

    cv::imwrite(fileName, sourceImage, { cv::IMWRITE_JPEG_RST_INTERVAL, batches });
}

void initJpegParallelization() {
    static bool initialized [[maybe_unused]] = ([]() {
        static jpeg_parallel_impl impl;

        impl.apply = [](jpeg_batch_entry_point entry_point, int num_batches, void* context) {
            cv::parallel_for_(cv::Range(0, num_batches), [=](const cv::Range& range) {
                entry_point(context, range.start);
            }, num_batches);

            // for (int i = num_batches - 1; i >= 0; --i) {
            //     entry_point(context, i);
            // }
        };

        jpeg_set_parallel_impl(&impl);

        return true;
    })();
}

typedef testing::TestWithParam<std::pair<int, int>> Imgcodecs_Jpeg_Batching;

// TEST_P(Imgcodecs_Jpeg_Batching, encode_benchmark)
// {
//     cvtest::TS& ts = *cvtest::TS::ptr();
//     string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
//     cv::Mat img = cv::imread(input);
//     ASSERT_FALSE(img.empty());

//     initJpegParallelization();

//     // testTranscodeOne(img, {512, 512});
//     auto sizeRange = GetParam();
//     for (int i = sizeRange.first; i <= sizeRange.second; ++i) {
//         const int batches = 0;
//         // for (int batches = 2; batches <= 8; ++batches) {
//             testTranscodeAllColorspaces(img, {i, i}, batches);
//             // testTranscodeAllColorspaces(img, {i, 511}, batches);
//             // testTranscodeAllColorspaces(img, {511, i}, batches);
//         // }
//     }
//     // testTranscodeAllColorspaces(img, {512, 512}, 4);
//     // writeOne(img, {65, 65}, "511.jpg", 4);
//     // writeOne(img, {512, 512}, "512.jpg", 7);
// }

TEST(Imgcodecs_Jpeg, encode_test)
{
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    initJpegParallelization();

    writeOne(img, {512, 512}, "parallel.jpg", 4);

    // int* p = new int[20];
    // delete[] p;
    // p[0] = 15;
    // std::cout << "use after free: " << p[0] << std::endl;
}

std::vector<std::pair<int, int>> ranges = ([]() {
    std::vector<std::pair<int, int>> result;
    for (int i = 2048; i < 2048 + 64; i += 16) {
        result.emplace_back(i, i + 16 - 1);
    }
    return result;
})();


// INSTANTIATE_TEST_CASE_P(Imgcodecs_Jpeg_Batching, Imgcodecs_Jpeg_Batching,
//                         testing::ValuesIn(ranges));

//==================================================================================================

static const uint32_t default_sampling_factor = static_cast<uint32_t>(0x221111);

static uint32_t test_jpeg_subsampling( const Mat src, const vector<int> param )
{
    vector<uint8_t> jpeg;

    if ( cv::imencode(".jpg", src, jpeg, param ) == false )
    {
        return 0;
    }

    if ( src.channels() != 3 )
    {
        return 0;
    }

    // Find SOF Marker(FFC0)
    int sof_offset = 0; // not found.
    int jpeg_size = static_cast<int>( jpeg.size() );
    for ( int i = 0 ; i < jpeg_size - 1; i++ )
    {
        if ( (jpeg[i] == 0xff ) && ( jpeg[i+1] == 0xC0 ) )
        {
            sof_offset = i;
            break;
        }
    }
    if ( sof_offset == 0 )
    {
        return 0;
    }

    // Extract Subsampling Factor from SOF.
    return ( jpeg[sof_offset + 0x0A + 3 * 0 + 1] << 16 ) +
           ( jpeg[sof_offset + 0x0A + 3 * 1 + 1] << 8  ) +
           ( jpeg[sof_offset + 0x0A + 3 * 2 + 1]       ) ;
}

TEST(Imgcodecs_Jpeg, encode_subsamplingfactor_default)
{
    vector<int> param;
    Mat src( 48, 64, CV_8UC3, cv::Scalar::all(0) );
    EXPECT_EQ( default_sampling_factor, test_jpeg_subsampling(src, param) );
}

TEST(Imgcodecs_Jpeg, encode_subsamplingfactor_usersetting_valid)
{
    Mat src( 48, 64, CV_8UC3, cv::Scalar::all(0) );
    const uint32_t sampling_factor_list[] = {
        IMWRITE_JPEG_SAMPLING_FACTOR_411,
        IMWRITE_JPEG_SAMPLING_FACTOR_420,
        IMWRITE_JPEG_SAMPLING_FACTOR_422,
        IMWRITE_JPEG_SAMPLING_FACTOR_440,
        IMWRITE_JPEG_SAMPLING_FACTOR_444,
    };
    const int sampling_factor_list_num = 5;

    for ( int i = 0 ; i < sampling_factor_list_num; i ++ )
    {
        vector<int> param;
        param.push_back( IMWRITE_JPEG_SAMPLING_FACTOR );
        param.push_back( sampling_factor_list[i] );
        EXPECT_EQ( sampling_factor_list[i], test_jpeg_subsampling(src, param) );
    }
}

TEST(Imgcodecs_Jpeg, encode_subsamplingfactor_usersetting_invalid)
{
    Mat src( 48, 64, CV_8UC3, cv::Scalar::all(0) );
    const uint32_t sampling_factor_list[] = { // Invalid list
        0x111112,
        0x000000,
        0x001111,
        0xFF1111,
        0x141111, // 1x4,1x1,1x1 - unknown
        0x241111, // 2x4,1x1,1x1 - unknown
        0x421111, // 4x2,1x1,1x1 - unknown
        0x441111, // 4x4,1x1,1x1 - 410(libjpeg cannot handle it)
    };
    const int sampling_factor_list_num = 8;

    for ( int i = 0 ; i < sampling_factor_list_num; i ++ )
    {
        vector<int> param;
        param.push_back( IMWRITE_JPEG_SAMPLING_FACTOR );
        param.push_back( sampling_factor_list[i] );
        EXPECT_EQ( default_sampling_factor, test_jpeg_subsampling(src, param) );
    }
}

#endif // HAVE_JPEG

}} // namespace
