// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html
#include <stdio.h>
#include "jpeglib.h"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
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

void eraseBoundaryRows(const cv::Mat& mat, int batches, cv::ImreadModes mode) {
    const int mcuSize = mode & cv::IMREAD_REDUCED_GRAYSCALE_2 ? 8
        : mode & cv::IMREAD_REDUCED_GRAYSCALE_4 ? 4
        : mode & cv::IMREAD_REDUCED_GRAYSCALE_8 ? 2
        : 16;
    int mcus = (mat.rows + mcuSize - 1) / mcuSize;
    int rowsPerBatch = (mcus + batches - 1) / batches;
    for (int i = 1; i < batches; ++i) {
        int startRow = i * rowsPerBatch * mcuSize;
        if (startRow >= mat.rows) {
            break;
        }
        mat(cv::Range(startRow - 1, startRow + 1), cv::Range::all()) = cv::Scalar::all(0);
    }
}

void testTranscodeOne(const cv::Mat& original, cv::Size targetSize, int batches, cv::ImwriteJPEGSamplingFactorParams samplingFactor = cv::IMWRITE_JPEG_SAMPLING_FACTOR_420, cv::ImreadModes mode = cv::IMREAD_COLOR, bool throughFile = false) {
    std::vector<uchar> outputNonbatched, outputBatched;

    cv::Mat sourceImage;
    cv::resize(original, sourceImage, targetSize);

    std::chrono::high_resolution_clock::time_point startEncodeNonbatched = std::chrono::high_resolution_clock::now();
    if (throughFile) {
        cv::imwrite("_nonbatched.jpg", sourceImage, { cv::IMWRITE_JPEG_SAMPLING_FACTOR, samplingFactor });
    } else {
        cv::imencode(".jpg", sourceImage, outputNonbatched, { cv::IMWRITE_JPEG_SAMPLING_FACTOR, samplingFactor });
    }
    std::chrono::high_resolution_clock::time_point endEncodeNonbatched = std::chrono::high_resolution_clock::now();
    cv::Mat decodedNonbatched;
    if (throughFile) {
        decodedNonbatched = cv::imread("_nonbatched.jpg", mode);
    } else {
        decodedNonbatched = cv::imdecode(outputNonbatched, mode);
    }
    std::chrono::high_resolution_clock::time_point endDecodeNonbatched = std::chrono::high_resolution_clock::now();

    std::chrono::high_resolution_clock::time_point startBatched = std::chrono::high_resolution_clock::now();
    if (throughFile) {
        cv::imwrite("_batched.jpg", sourceImage, { cv::IMWRITE_JPEG_RST_INTERVAL, batches, cv::IMWRITE_JPEG_SAMPLING_FACTOR, samplingFactor });
    } else {
        cv::imencode(".jpg", sourceImage, outputBatched, { cv::IMWRITE_JPEG_RST_INTERVAL, batches, cv::IMWRITE_JPEG_SAMPLING_FACTOR, samplingFactor });
    } 
    std::chrono::high_resolution_clock::time_point endBatched = std::chrono::high_resolution_clock::now();
    cv::Mat decodedBatched;
    if (throughFile) {
        decodedBatched = cv::imread("_batched.jpg", mode);
    } else {
        decodedBatched = cv::imdecode(outputBatched, mode);
    }
    std::chrono::high_resolution_clock::time_point endDecodeBatched = std::chrono::high_resolution_clock::now();

    auto durationEncodeNonbatched = std::chrono::duration_cast<std::chrono::microseconds>(endEncodeNonbatched - startEncodeNonbatched).count();
    auto durationDecodeNonbatched = std::chrono::duration_cast<std::chrono::microseconds>(endDecodeNonbatched - endEncodeNonbatched).count();
    auto durationEncodeBatched = std::chrono::duration_cast<std::chrono::microseconds>(endBatched - startBatched).count();
    auto durationDecodeBatched = std::chrono::duration_cast<std::chrono::microseconds>(endDecodeBatched - endBatched).count();

    // std::cout << "Encoding speed-up (times): " << durationEncodeNonbatched / (double)durationEncodeBatched << std::endl;
    // std::cout << "Decoding speed-up (times): " << durationDecodeNonbatched / (double)durationDecodeBatched << std::endl;

    ASSERT_EQ(decodedNonbatched.size(), decodedNonbatched.size());
    if (batches > 1) {
        eraseBoundaryRows(decodedNonbatched, batches, mode);
        eraseBoundaryRows(decodedBatched, batches, mode);
    }
    ASSERT_EQ(0, cvtest::norm(decodedNonbatched, decodedBatched, NORM_INF));
}

void testTranscodeAllColorspaces(const cv::Mat& bgr, cv::Size targetSize, int batches) {
    static const std::array<cv::ImwriteJPEGSamplingFactorParams, 5> samplingModes = {
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_420,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_422,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_444,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_411,
        cv::IMWRITE_JPEG_SAMPLING_FACTOR_440,
    };

    static const std::array<cv::ImreadModes, 4> imreadOptions = {
        cv::IMREAD_COLOR,
        cv::IMREAD_REDUCED_COLOR_2,
        cv::IMREAD_REDUCED_COLOR_4,
        cv::IMREAD_REDUCED_COLOR_8,
    };

    for (auto throughFile : {true, false}) {
        for (auto imreadOption : imreadOptions) {
            for (auto samplingMode : samplingModes) {
                testTranscodeOne(bgr, targetSize, batches, samplingMode, imreadOption, throughFile);
                cv::Mat gray;
                cv::cvtColor(bgr, gray, cv::COLOR_BGR2GRAY);
                testTranscodeOne(gray, targetSize, batches, samplingMode, imreadOption, throughFile);
                cv::Mat bgra;
                cv::cvtColor(bgr, bgra, cv::COLOR_BGR2BGRA);
                testTranscodeOne(bgra, targetSize, batches, samplingMode, imreadOption, throughFile);
            }
        }
    }
}

void writeOne(const cv::Mat& original, cv::Size targetSize, const std::string& fileName, int batches) {
    std::vector<uchar> outputBatched;

    cv::Mat sourceImage;
    cv::resize(original, sourceImage, targetSize);

    cv::imwrite(fileName, sourceImage, { cv::IMWRITE_JPEG_RST_INTERVAL, batches, });
}

void initJpegParallelization() {
    static bool initialized [[maybe_unused]] = ([]() {
        static jpeg_parallel_impl impl;

        cv::setNumThreads(8);

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
static constexpr bool AUTOMATIC_BATCHING = false;

TEST_P(Imgcodecs_Jpeg_Batching, encode_benchmark)
{
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    initJpegParallelization();

    auto sizeRange = GetParam();
    for (int i = sizeRange.first; i <= sizeRange.second; ++i) {
        // const int batches = 0;
        for (int batches = 8; batches <= 8; ++batches) {
            if (i >= 2048 && batches == 2) {
                continue;
            }

            if (AUTOMATIC_BATCHING) {
                batches = 1;
            }
            testTranscodeAllColorspaces(img, {i, i}, batches);
            testTranscodeAllColorspaces(img, {i, 128}, batches);
            testTranscodeAllColorspaces(img, {128, i}, batches);
            if (AUTOMATIC_BATCHING) {
                break;
            }
        }
    }
}

TEST(Imgcodecs_Jpeg, encode_test)
{
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    initJpegParallelization();

    writeOne(img, {511, 511}, "serial.jpg", 0);
    writeOne(img, {511, 511}, "parallel.jpg", 4);

    testTranscodeOne(img, {2048, 2048}, 4);
}

TEST(Imgcodecs_Jpeg, decode_test) {
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    initJpegParallelization();

    // cv::Size theSize = {512, 512-9};
    cv::Size theSize = {512, 512};
    // int batches = 2;

    testTranscodeAllColorspaces(img, theSize, 4);

    // writeOne(img, theSize, "serial.jpg", 0);
    // writeOne(img, theSize, "parallel.jpg", batches);

    // cv::Mat serial = cv::imread("serial.jpg");
    // cv::Mat parallel = cv::imread("parallel.jpg");

    // cv::imwrite("roundtrip.jpg", parallel);

    // eraseBoundaryRows(serial, batches);
    // eraseBoundaryRows(parallel, batches);
    // EXPECT_EQ(0, cvtest::norm(serial, parallel, NORM_INF));
}

TEST(Imgcodecs_Jpeg, transcode_minitest) {
    cvtest::TS& ts = *cvtest::TS::ptr();
    string input = string(ts.get_data_path()) + "../cv/shared/lena.png";
    cv::Mat img = cv::imread(input);
    ASSERT_FALSE(img.empty());

    initJpegParallelization();

    for (int i = 2048; i <= 2048; ++i) {
        testTranscodeAllColorspaces(img, {i, i}, 4);
    }
}

std::vector<std::pair<int, int>> ranges = ([]() {
    std::vector<std::pair<int, int>> result;
    // for (int i = 1; i <= 512; i ++) {
    //     result.emplace_back(i, i);
    // }
    for (int i = 256; i <= 1024; i ++) {
        result.emplace_back(i, i);
    }
    // for (int i = 1024; i <= 4096-512; i ++) {
    //     result.emplace_back(i, i);
    // }
    for (int i = 4096-512; i <= 4096; i ++) {
        result.emplace_back(i, i);
    }
    return result;
})();


INSTANTIATE_TEST_CASE_P(Imgcodecs_Jpeg_Batching, Imgcodecs_Jpeg_Batching,
                        testing::ValuesIn(ranges));

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
