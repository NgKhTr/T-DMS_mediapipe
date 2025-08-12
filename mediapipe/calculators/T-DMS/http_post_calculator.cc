#ifndef MEDIAPIPE_CALCULATORS_T_DMS_HTTP_POST_CALCULATOR_H_
#define MEDIAPIPE_CALCULATORS_T_DMS_HTTP_POST_CALCULATOR_H_

#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/calculators/T-DMS/http_post_calculator_options.pb.h"
#include "curl/curl.h"
#include <fstream>

namespace mediapipe {

class HttpPostCalculator : public CalculatorBase {
private:
    std::string url_;
    std::string payload_path_;
    std::string payload_;
    bool verbose_;
    CURL* curl_;

    // Callback function for curl write operation
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t total_size = size * nmemb;
        std::string* response = static_cast<std::string*>(userp);
        response->append(static_cast<char*>(contents), total_size);
        return total_size;
    }
public:
    static absl::Status GetContract(CalculatorContract* cc) {
        cc->Inputs().Tag("TRIGGER").Set<bool>();
        cc->Outputs().Tag("RESPONSE").Set<std::string>();
        return absl::OkStatus();
    }
    absl::Status Open(CalculatorContext* cc) override {
		cc->SetOffset(TimestampDiff(0));
        const auto& opts = cc->Options<HttpPostCalculatorOptions>();
        url_ = opts.url();
        payload_path_ = opts.payload_path();
        verbose_ = opts.verbose();

        std::ifstream payload_file(payload_path_);
        if (!payload_file.is_open()) {
            LOG(ERROR) << "Failed to open payload file: " << payload_path_;
        }
        payload_ = std::string((std::istreambuf_iterator<char>(payload_file)),
                            std::istreambuf_iterator<char>());
        payload_file.close();

        curl_ = curl_easy_init();
        if (!curl_) {
            LOG(ERROR) << "Failed to initialize CURL";
            return absl::InternalError("Failed to initialize CURL");
        }

        return absl::OkStatus();
    }
    absl::Status Close(CalculatorContext* cc) override {
        if (curl_) {
            curl_easy_cleanup(curl_);
            curl_ = nullptr;
        }
    }
    absl::Status Process(CalculatorContext* cc) override {
        if (cc->Inputs().Tag("TRIGGER").IsEmpty()
            || !cc->Inputs().Tag("TRIGGER").Get<bool>()
        ) {
            return absl::OkStatus();
        }
        curl_easy_reset(curl_);
        curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str());
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_.c_str());

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 0L);

        std::string response_string;
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_string);
        CURLcode res = curl_easy_perform(curl_);
        if (res != CURLE_OK) {
            LOG(INFO) << "HTTP post request failed: " << curl_easy_strerror(res);
        } else if (verbose_) {
            LOG(INFO) << "HTTP post request sent successfully";
        }

        curl_slist_free_all(headers);
        cc->Outputs().Tag("RESPONSE").Add(new std::string(response_string), cc->InputTimestamp());
        return absl::OkStatus();
    }
};
REGISTER_CALCULATOR(HttpPostCalculator);
}  // namespace mediapipe
#endif  // MEDIAPIPE_CALCULATORS_T_DMS_HTTP_POST_CALCULATOR_H_
