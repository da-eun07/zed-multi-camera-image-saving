///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2020, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


/******************************************************************************************************************
 ** This sample demonstrates how to use two ZEDs with the ZED SDK, each grab are in a separate thread             **
 ** This sample has been tested with 3 ZEDs in HD720@30fps resolution. Linux only.                                **
 *******************************************************************************************************************/

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

 // Using std and sl namespaces
using namespace std;
using namespace sl;

void zed_acquisition(Camera& zed, cv::Mat& image_cv_left, cv::Mat& image_cv_right, cv::Mat& image_cv_depth, bool& run, Timestamp& ts);

int main(int argc, char** argv) {

    string file_dir = "../data/2024.05.22"; // ../data/NAME/CAM_NUM/TYPE/

	InitParameters init_parameters;
    init_parameters.depth_mode = DEPTH_MODE::PERFORMANCE;
    init_parameters.camera_resolution = RESOLUTION::HD720;
    init_parameters.camera_fps = 30;
    
	vector< DeviceProperties> devList = Camera::getDeviceList();
    int nb_detected_zed = devList.size();

	for (int z = 0; z < nb_detected_zed; z++) {
		std::cout << "ID : " << devList[z].id << " ,model : " << devList[z].camera_model << " , S/N : " << devList[z].serial_number << " , state : "<<devList[z].camera_state<<std::endl;
	}
	
    if (nb_detected_zed == 0) {
        cout << "No ZED Detected, exit program" << endl;
        return EXIT_FAILURE;
    }
    
    cout << nb_detected_zed << " ZED Detected" << endl;

    vector<Camera> zeds(nb_detected_zed);
    // try to open every detected cameras
    for (int z = 0; z < nb_detected_zed; z++) {
        init_parameters.input.setFromCameraID(z);
        ERROR_CODE err = zeds[z].open(init_parameters);
        if (err == ERROR_CODE::SUCCESS) {
            auto cam_info = zeds[z].getCameraInformation();
            cout << cam_info.camera_model << ", ID: " << z << ", SN: " << cam_info.serial_number << " Opened" << endl;
        } else {
            cout << "ZED ID:" << z << " Error: " << err << endl;
            zeds[z].close();
        }
    }
    
    bool run = true;
    // Create a grab thread for each opened camera
    vector<thread> thread_pool(nb_detected_zed); // compute threads
    vector<cv::Mat> images_left(nb_detected_zed); // display images
    vector<string> wnd_names_left(nb_detected_zed); // display windows names
    vector<Timestamp> images_ts(nb_detected_zed); // images timestamps

    vector<cv::Mat> images_right(nb_detected_zed); // display images
    vector<string> wnd_names_right(nb_detected_zed); // display windows names

    vector<cv::Mat> images_depth(nb_detected_zed); // display images
    vector<string> wnd_names_depth(nb_detected_zed); // display windows name

    for (int z = 0; z < nb_detected_zed; z++)
        if (zeds[z].isOpened()) {
            // create an image to store Left+Depth image
            images_left[z] = cv::Mat(720, 1280, CV_8UC4);
            images_right[z] = cv::Mat(720, 1280, CV_8UC4);
            images_depth[z] = cv::Mat(720, 1280, CV_8UC4);
            // camera acquisition thread
            thread_pool[z] = std::thread(zed_acquisition, ref(zeds[z]), ref(images_left[z]), ref(images_right[z]), ref(images_depth[z]), ref(run), ref(images_ts[z]));
            // create windows for display
            wnd_names_left[z] = "ZED ID: " + to_string(z) + " LEFT";
            wnd_names_right[z] = "ZED ID: " + to_string(z) + " RIGHT";
            wnd_names_depth[z] = "ZED ID: " + to_string(z) + " DEPTH";
            // cv::namedWindow(wnd_names_left[z]);
            // cv::namedWindow(wnd_names_right[z]);
            // cv::namedWindow(wnd_names_depth[z]);
        }

    vector<Timestamp> last_ts_left(nb_detected_zed, 0); // use to detect new images
    vector<Timestamp> last_ts_right(nb_detected_zed, 0); // use to detect new images
    vector<Timestamp> last_ts_depth(nb_detected_zed, 0); // use to detect new images

    char key = ' ';
    // Loop until 'Esc' is pressed
    while (key != 27) {
        // Resize and show images
        for (int z = 0; z < nb_detected_zed; z++) {
            if (images_ts[z] > last_ts_left[z]) { // if the current timestamp is newer it is a new image
                auto now = chrono::system_clock::now();
                time_t now_time_t = chrono::system_clock::to_time_t(now);
                tm now_tm = *localtime(&now_time_t);
                auto now_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
                ostringstream ossl;
                ostringstream ossr; 
                ostringstream ossd;
                ossl << file_dir << "/" << z << "/rgb_left/" << put_time(&now_tm, "%H:%M:%S") << "." << setfill('0') <<setw(3) << now_ms.count() << ".png";
                cv::imwrite(ossl.str(), images_left[z]);
                ossr << file_dir << "/" << z << "/rgb_right/" << put_time(&now_tm, "%H:%M:%S") << "." << setfill('0') <<setw(3) << now_ms.count() << ".png";
                cv::imwrite(ossr.str(), images_right[z]);
                ossd << file_dir << "/" << z << "/depth/" << put_time(&now_tm, "%H:%M:%S") << "." << setfill('0') <<setw(3) << now_ms.count() << ".tiff";
                cv::imwrite(ossd.str(), images_depth[z]);

                // cv::imshow(wnd_names_left[z], images_left[z]);
                last_ts_left[z] = images_ts[z];
                // cout << "ZED ID:" << z << " Image timestampe: " << images_ts[z] << endl;
            }
        }

        key = cv::waitKey(10);
    }

    // stop all running threads
    run = false;

    // Wait for every thread to be stopped
    for (int z = 0; z < nb_detected_zed; z++)
        if (zeds[z].isOpened()) {
            thread_pool[z].join();
        }
    return EXIT_SUCCESS;
}

void zed_acquisition(Camera& zed, cv::Mat& image_cv_left, cv::Mat& image_cv_right, cv::Mat& image_cv_depth, bool& run, Timestamp& ts) {
    Mat zed_image;
    const int w_low_res = image_cv_left.cols;
    const int h_low_res = image_cv_left.rows;
    Resolution low_res(w_low_res, h_low_res);
    while (run) {
        // grab current images and compute depth
        if (zed.grab() == ERROR_CODE::SUCCESS) {
            zed.retrieveImage(zed_image, VIEW::LEFT, MEM::CPU, low_res);
            cv::Mat(h_low_res, w_low_res, CV_8UC4, zed_image.getPtr<sl::uchar1>(MEM::CPU)).copyTo(image_cv_left(cv::Rect(0, 0, w_low_res, h_low_res)));
            zed.retrieveImage(zed_image, VIEW::RIGHT, MEM::CPU, low_res);
            cv::Mat(h_low_res, w_low_res, CV_8UC4, zed_image.getPtr<sl::uchar1>(MEM::CPU)).copyTo(image_cv_right(cv::Rect(0, 0, w_low_res, h_low_res)));
            zed.retrieveImage(zed_image, VIEW::DEPTH, MEM::CPU, low_res);
            cv::Mat(h_low_res, w_low_res, CV_8UC4, zed_image.getPtr<sl::uchar1>(MEM::CPU)).copyTo(image_cv_depth(cv::Rect(0, 0, w_low_res, h_low_res)));
            
            ts = zed.getTimestamp(TIME_REFERENCE::IMAGE);
        }
        sleep_ms(2);
    }
    zed.close();
}
