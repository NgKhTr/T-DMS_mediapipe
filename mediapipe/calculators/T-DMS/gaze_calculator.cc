#ifndef MEDIAPIPE_CALCULATORS_T_DMS_GAZE_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_GAZE_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/calculators/T-DMS/gaze_calculator_options.pb.h"
#include "mediapipe/util/render_data.pb.h"
#include "mediapipe/util/color.pb.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/gpu/gpu_buffer.h"
#include "mediapipe/framework/port/opencv_core_inc.h"  // For cv::Point2d, Mat, etc.
#include "mediapipe/framework/port/opencv_imgproc_inc.h"  // For use cv::line, 
#include "mediapipe/framework/port/opencv_calib3d_inc.h" // For cv::projectPoints, cv::solvePnP


namespace mediapipe {
typedef std::pair<cv::Point2d, cv::Point2d> Arrow;

class GazeCalculator : public CalculatorBase {
private:
    bool verbose_;
    Color color_;
    double thickness_;
    cv::Point2d convertRelativeToAbsolute2d(const NormalizedLandmark& landmark, int width, int height) {
        return {landmark.x() * width, landmark.y() * height};
    }
    cv::Point3d convertRelativeToAbsolute3d(const NormalizedLandmark& landmark, int width, int height) {
        return {landmark.x() * width, landmark.y() * height, 0.0};
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("LEFT_IRIS_LANDMARKS").Set<NormalizedLandmarkList>();
        cc->Inputs().Tag("RIGHT_IRIS_LANDMARKS").Set<NormalizedLandmarkList>();
        cc->Inputs().Tag("IMAGE_GPU").Set<GpuBuffer>();
        cc->Inputs().Tag("FACE_LANDMARKS").Set<NormalizedLandmarkList>();
        cc->Outputs().Tag("GAZE_RENDER_DATA").Set<RenderData>();
        cc->Outputs().Tag("LEFT_GAZE_DIRECTION").Set<Arrow>();
        cc->Outputs().Tag("RIGHT_GAZE_DIRECTION").Set<Arrow>();
        // cc->Outputs().Tag("FACE_DIRECTION").Set<Arrow>();
        return absl::OkStatus();
    }

    absl::Status Open(CalculatorContext* cc) override {
        const auto& opts = cc->Options<GazeCalculatorOptions>();
        verbose_ = opts.verbose();
        color_ = opts.color();
        thickness_ = opts.thickness();
        cc->SetOffset(TimestampDiff(0));
        return absl::OkStatus();
    }

    absl::Status Process(CalculatorContext* cc) override {
        if (cc->Inputs().Tag("FACE_LANDMARKS").IsEmpty()
            || cc->Inputs().Tag("LEFT_IRIS_LANDMARKS").IsEmpty()
            || cc->Inputs().Tag("RIGHT_IRIS_LANDMARKS").IsEmpty()
            || cc->Inputs().Tag("IMAGE_GPU").IsEmpty()
        ){
            return absl::OkStatus();
        }
        const NormalizedLandmarkList& face_landmarks = cc->Inputs().Tag("FACE_LANDMARKS").Get<NormalizedLandmarkList>(),
            left_iris_landmarks = cc->Inputs().Tag("LEFT_IRIS_LANDMARKS").Get<NormalizedLandmarkList>(),
            right_iris_landmarks = cc->Inputs().Tag("RIGHT_IRIS_LANDMARKS").Get<NormalizedLandmarkList>();
        const GpuBuffer& gpu_buffer = cc->Inputs().Tag("IMAGE_GPU").Get<GpuBuffer>();
        const int width = gpu_buffer.width(),
            height = gpu_buffer.height();
        LOG(INFO) << "FACE_LANDMARKS: " << face_landmarks.landmark_size();
        // === CONVERT LANDMARKS ===
        const int nose_index = 4, chin_index = 152,
            left_eye_index = 263, right_eye_index = 33,
            left_mouth_index = 287, right_mouth_index = 57;

        const cv::Point2d nose_2d = convertRelativeToAbsolute2d(face_landmarks.landmark(nose_index), width, height),
            chin_2d = convertRelativeToAbsolute2d(face_landmarks.landmark(chin_index), width, height),
            left_eye_2d = convertRelativeToAbsolute2d(face_landmarks.landmark(left_eye_index), width, height),
            right_eye_2d = convertRelativeToAbsolute2d(face_landmarks.landmark(right_eye_index), width, height),
            left_mouth_2d = convertRelativeToAbsolute2d(face_landmarks.landmark(left_mouth_index), width, height),
            right_mouth_2d = convertRelativeToAbsolute2d(face_landmarks.landmark(right_mouth_index), width, height);
        std::vector<cv::Point2d> image_points_2d = {nose_2d, chin_2d, left_eye_2d, right_eye_2d, left_mouth_2d, right_mouth_2d};

        const cv::Point3d nose_3d = convertRelativeToAbsolute3d(face_landmarks.landmark(nose_index), width, height),
            chin_3d = convertRelativeToAbsolute3d(face_landmarks.landmark(chin_index), width, height),
            left_eye_3d = convertRelativeToAbsolute3d(face_landmarks.landmark(left_eye_index), width, height),
            right_eye_3d = convertRelativeToAbsolute3d(face_landmarks.landmark(right_eye_index), width, height),
            left_mouth_3d = convertRelativeToAbsolute3d(face_landmarks.landmark(left_mouth_index), width, height),
            right_mouth_3d = convertRelativeToAbsolute3d(face_landmarks.landmark(right_mouth_index), width, height);
        std::vector<cv::Point3d> image_points_3d = {nose_3d, chin_3d, left_eye_3d, right_eye_3d, left_mouth_3d, right_mouth_3d};

        std::vector<cv::Point3d> model_points = {{0.0, 0.0, 0.0},
                                                {0, -63.6, -12.5},
                                                {-43.3, 32.7, -26},
                                                {43.3, 32.7, -26},
                                                {-28.9, -28.9, -24.1},
                                                {28.9, -28.9, -24.1}};


        cv::Mat right_eye_ball_center = (cv::Mat_<double>(3,1) << -29.05, 32.7, -39.5),
            left_eye_ball_center = (cv::Mat_<double>(3,1) << 29.05, 32.7, -39.5);
        double focal_length = width;
        cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << 
            focal_length, 0, width/2,
            0, focal_length, height/2,
            0, 0, 1);
        cv::Mat dist_coeffs = cv::Mat::zeros(4, 1, CV_64F);

        cv::Mat rotation_vector, translation_vector;
        const bool isSolvePnPSuccess = cv::solvePnP(model_points, image_points_2d, camera_matrix, dist_coeffs,
            rotation_vector, translation_vector, false, cv::SOLVEPNP_ITERATIVE);
        if (!isSolvePnPSuccess) {
            LOG(INFO) << "solvePnP failed, cannot compute gaze.";
            return absl::OkStatus();
        }
        cv::Mat transformation, inliers;
        const float ransac_threshold = 10.0;
        cv::estimateAffine3D(image_points_3d, model_points, transformation, inliers, ransac_threshold);
        if (transformation.empty()) {
            LOG(INFO) << "estimateAffine3D failed, cannot compute gaze.";
            return absl::OkStatus();
        }

        const int pupil_index = 0;
        cv::Point2d left_pupil = convertRelativeToAbsolute2d(left_iris_landmarks.landmark(pupil_index), width, height),
            right_pupil = convertRelativeToAbsolute2d(right_iris_landmarks.landmark(pupil_index), width, height);

        cv::Mat left_pupil_h = (cv::Mat_<double>(4, 1) << left_pupil.x, left_pupil.y, 0, 1),
            right_pupil_h = (cv::Mat_<double>(4, 1) << right_pupil.x, right_pupil.y, 0, 1);

        cv::Mat left_pupil_world = transformation * left_pupil_h,
            right_pupil_world = transformation * right_pupil_h;

        cv::Mat left_S = left_eye_ball_center + (left_pupil_world.rowRange(0,3)  - left_eye_ball_center) * 10,
            right_S = right_eye_ball_center + (right_pupil_world.rowRange(0,3)  - right_eye_ball_center) * 10;

        std::vector<cv::Point3d> left_gaze_target = {cv::Point3d(left_S.at<double>(0), left_S.at<double>(1), left_S.at<double>(2))},
            right_gaze_target = {cv::Point3d(right_S.at<double>(0), right_S.at<double>(1), right_S.at<double>(2))};
        std::vector<cv::Point2d> left_eye_pupil_2d, right_eye_pupil_2d;
        cv::projectPoints(left_gaze_target, rotation_vector, translation_vector, camera_matrix, dist_coeffs, left_eye_pupil_2d);
        cv::projectPoints(right_gaze_target, rotation_vector, translation_vector, camera_matrix, dist_coeffs, right_eye_pupil_2d);

        std::vector<cv::Point2d> left_head_pose_2d, right_head_pose_2d;
        std::vector<cv::Point3d> left_pupil_forward = {cv::Point3d(left_pupil_world.at<double>(0), left_pupil_world.at<double>(1), 40.0)},
                                right_pupil_forward = {cv::Point3d(right_pupil_world.at<double>(0), right_pupil_world.at<double>(1), 40.0)};
        cv::projectPoints(left_pupil_forward, rotation_vector, translation_vector, camera_matrix, dist_coeffs, left_head_pose_2d);
        cv::projectPoints(right_pupil_forward, rotation_vector, translation_vector, camera_matrix, dist_coeffs, right_head_pose_2d);


        cv::Point2d left_look_at_2d = left_pupil
            + (left_eye_pupil_2d[0] - left_pupil)
            - (left_head_pose_2d[0] - left_pupil);
        cv::Point2d right_look_at_2d = right_pupil
            + (right_eye_pupil_2d[0] - right_pupil)
            - (right_head_pose_2d[0] - right_pupil);

        // Add arrow to RenderData
        auto render_data = absl::make_unique<RenderData>();

        RenderAnnotation* left_gaze_anno = render_data->add_render_annotations();
        left_gaze_anno->mutable_arrow()->set_x_start(left_pupil.x);
        left_gaze_anno->mutable_arrow()->set_y_start(left_pupil.y);
        left_gaze_anno->mutable_arrow()->set_x_end(left_look_at_2d.x);
        left_gaze_anno->mutable_arrow()->set_y_end(left_look_at_2d.y);
        left_gaze_anno->mutable_arrow()->set_normalized(false);
        left_gaze_anno->mutable_color()->set_r(color_.r());
        left_gaze_anno->mutable_color()->set_g(color_.g());
        left_gaze_anno->mutable_color()->set_b(color_.b());
        left_gaze_anno->set_thickness(thickness_);

        RenderAnnotation* right_gaze_anno = render_data->add_render_annotations();
        right_gaze_anno->mutable_arrow()->set_x_start(right_pupil.x);
        right_gaze_anno->mutable_arrow()->set_y_start(right_pupil.y);
        right_gaze_anno->mutable_arrow()->set_x_end(right_look_at_2d.x);
        right_gaze_anno->mutable_arrow()->set_y_end(right_look_at_2d.y);
        right_gaze_anno->mutable_arrow()->set_normalized(false);
        right_gaze_anno->mutable_color()->set_r(color_.r());
        right_gaze_anno->mutable_color()->set_g(color_.g());
        right_gaze_anno->mutable_color()->set_b(color_.b());
        right_gaze_anno->set_thickness(thickness_);

        cc->Outputs().Tag("GAZE_RENDER_DATA").Add(render_data.release(), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(GazeCalculator);
}
#endif