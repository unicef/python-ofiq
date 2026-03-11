// Python bindings for OFIQ (Open Source Face Image Quality)

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>

#include <ofiq_lib.h>
#include <ofiq_structs.h>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#ifndef OFIQPY_VERSION
#define OFIQPY_VERSION "0.0.0"
#endif
#define OFIQPY_OFIQ_VERSION "1.1.2"

class OFIQError : public std::runtime_error {
public:
    explicit OFIQError(const std::string& msg) : std::runtime_error(msg) {}
};

class ConfigError : public OFIQError {
public:
    explicit ConfigError(const std::string& msg) : OFIQError(msg) {}
};

class QualityError : public OFIQError {
public:
    explicit QualityError(const std::string& msg) : OFIQError(msg) {}
};

class FaceDetectionError : public QualityError {
public:
    explicit FaceDetectionError(const std::string& msg) : QualityError(msg) {}
};

class LandmarkError : public QualityError {
public:
    explicit LandmarkError(const std::string& msg) : QualityError(msg) {}
};

class SegmentationError : public QualityError {
public:
    explicit SegmentationError(const std::string& msg) : QualityError(msg) {}
};

static py::object PyOFIQError;
static py::object PyConfigError;
static py::object PyQualityError;
static py::object PyFaceDetectionError;
static py::object PyLandmarkError;
static py::object PySegmentationError;

namespace {

void throw_ofiq_error(const OFIQ::ReturnStatus& st, const std::string& context = "")
{
    using RC = OFIQ::ReturnCode;

    std::string msg = context.empty() ? st.info : (context + ": " + st.info);

    switch (st.code) {
    case RC::MissingConfigFileError:
    case RC::MissingConfigParamError:
    case RC::UnknownConfigParamError:
        PyErr_SetString(PyConfigError.ptr(), msg.c_str());
        throw py::error_already_set();

    case RC::FaceDetectionError:
        PyErr_SetString(PyFaceDetectionError.ptr(), msg.c_str());
        throw py::error_already_set();

    case RC::FaceLandmarkExtractionError:
        PyErr_SetString(PyLandmarkError.ptr(), msg.c_str());
        throw py::error_already_set();

    case RC::FaceOcclusionSegmentationError:
    case RC::FaceParsingError:
        PyErr_SetString(PySegmentationError.ptr(), msg.c_str());
        throw py::error_already_set();

    case RC::ImageReadingError:
    case RC::ImageWritingError:
        PyErr_SetString(PyOFIQError.ptr(), msg.c_str());
        throw py::error_already_set();

    case RC::QualityAssessmentError:
    case RC::UnknownError:
    case RC::NotImplemented:
    default:
        PyErr_SetString(PyQualityError.ptr(), msg.c_str());
        throw py::error_already_set();
    }
}

std::string quality_measure_to_string(OFIQ::QualityMeasure m)
{
    using QM = OFIQ::QualityMeasure;
    switch (m) {
    case QM::UnifiedQualityScore:          return "UnifiedQualityScore";
    case QM::BackgroundUniformity:         return "BackgroundUniformity";
    case QM::IlluminationUniformity:       return "IlluminationUniformity";
    case QM::Luminance:                    return "Luminance";
    case QM::LuminanceMean:                return "LuminanceMean";
    case QM::LuminanceVariance:            return "LuminanceVariance";
    case QM::UnderExposurePrevention:      return "UnderExposurePrevention";
    case QM::OverExposurePrevention:       return "OverExposurePrevention";
    case QM::DynamicRange:                 return "DynamicRange";
    case QM::Sharpness:                    return "Sharpness";
    case QM::CompressionArtifacts:         return "CompressionArtifacts";
    case QM::NaturalColour:                return "NaturalColour";
    case QM::SingleFacePresent:            return "SingleFacePresent";
    case QM::EyesOpen:                     return "EyesOpen";
    case QM::MouthClosed:                  return "MouthClosed";
    case QM::EyesVisible:                  return "EyesVisible";
    case QM::MouthOcclusionPrevention:     return "MouthOcclusionPrevention";
    case QM::FaceOcclusionPrevention:      return "FaceOcclusionPrevention";
    case QM::InterEyeDistance:             return "InterEyeDistance";
    case QM::HeadSize:                     return "HeadSize";
    case QM::CropOfTheFaceImage:           return "CropOfTheFaceImage";
    case QM::LeftwardCropOfTheFaceImage:   return "LeftwardCropOfTheFaceImage";
    case QM::RightwardCropOfTheFaceImage:  return "RightwardCropOfTheFaceImage";
    case QM::MarginAboveOfTheFaceImage:    return "MarginAboveOfTheFaceImage";
    case QM::MarginBelowOfTheFaceImage:    return "MarginBelowOfTheFaceImage";
    case QM::HeadPose:                     return "HeadPose";
    case QM::HeadPoseYaw:                  return "HeadPoseYaw";
    case QM::HeadPosePitch:                return "HeadPosePitch";
    case QM::HeadPoseRoll:                 return "HeadPoseRoll";
    case QM::ExpressionNeutrality:         return "ExpressionNeutrality";
    case QM::NoHeadCoverings:              return "NoHeadCoverings";
    case QM::NotSet:                       return "NotSet";
    default:                               return "Unknown";
    }
}

OFIQ::Image numpy_to_ofiq_image(
    py::array_t<std::uint8_t, py::array::c_style | py::array::forcecast> image_rgb)
{
    if (image_rgb.ndim() != 3 || image_rgb.shape(2) != 3) {
        throw py::value_error(
            "Expected image shape (H, W, 3) uint8 RGB, got ndim=" +
            std::to_string(image_rgb.ndim()) +
            (image_rgb.ndim() == 3 ? ", channels=" + std::to_string(image_rgb.shape(2)) : ""));
    }

    auto h = image_rgb.shape(0);
    auto w = image_rgb.shape(1);
    if (h > 65535 || w > 65535) {
        throw py::value_error(
            "Image dimensions exceed uint16 limit (65535): " +
            std::to_string(h) + "x" + std::to_string(w));
    }

    auto height = static_cast<std::uint16_t>(h);
    auto width = static_cast<std::uint16_t>(w);

    py::buffer_info buf = image_rgb.request();
    auto* data_ptr = static_cast<std::uint8_t*>(buf.ptr);

    // Non-owning shared_ptr: OFIQ::Image::deepcopy will copy the data
    std::shared_ptr<std::uint8_t> sptr(data_ptr, [](std::uint8_t*) {});

    OFIQ::Image img;
    img.deepcopy(width, height, 24, sptr, /*isRgb=*/true);

    return img;
}

} // namespace

// Named OFIQWrapper to avoid collision with the OFIQ:: C++ namespace
class OFIQWrapper {
public:
    OFIQWrapper(const std::string& config_dir = "", const std::string& config_file = "ofiq_config.jaxn")
        : config_file_(config_file)
    {
        if (config_dir.empty()) {
            auto setup_mod = py::module_::import("ofiq.setup");
            config_dir_ = setup_mod.attr("resolve_data_dir")().cast<std::string>();
        } else {
            config_dir_ = config_dir;
        }

        impl_ = OFIQ::Interface::getImplementation();
        if (!impl_) {
            PyErr_SetString(PyOFIQError.ptr(), "OFIQ::Interface::getImplementation() returned null");
            throw py::error_already_set();
        }

        OFIQ::ReturnStatus st = impl_->initialize(config_dir_, config_file);
        if (st.code != OFIQ::ReturnCode::Success) {
            throw_ofiq_error(st, "Initialization failed");
        }
    }

    double scalar_quality(
        py::array_t<std::uint8_t, py::array::c_style | py::array::forcecast> image_rgb)
    {
        OFIQ::Image img = numpy_to_ofiq_image(image_rgb);

        double q = -1.0;
        OFIQ::ReturnStatus st = impl_->scalarQuality(img, q);

        if (st.code != OFIQ::ReturnCode::Success) {
            throw_ofiq_error(st);
        }

        return q;
    }

    py::object vector_quality(
        py::array_t<std::uint8_t, py::array::c_style | py::array::forcecast> image_rgb,
        bool include_raw = false)
    {
        OFIQ::Image img = numpy_to_ofiq_image(image_rgb);

        OFIQ::FaceImageQualityAssessment assessments;
        OFIQ::ReturnStatus st = impl_->vectorQuality(img, assessments);

        if (st.code != OFIQ::ReturnCode::Success) {
            throw_ofiq_error(st);
        }

        py::dict out;
        for (const auto& kv : assessments.qAssessments) {
            const auto measure = kv.first;
            const auto& result = kv.second;
            std::string key = quality_measure_to_string(measure);

            if (result.code != OFIQ::QualityMeasureReturnCode::Success) {
                out[py::str(key)] = py::none();
                continue;
            }

            if (include_raw) {
                py::dict measure_dict;
                measure_dict["scalar"] = result.scalar;
                measure_dict["raw"] = result.rawScore;
                out[py::str(key)] = measure_dict;
            } else {
                out[py::str(key)] = result.scalar;
            }
        }

        return out;
    }

    std::string repr() const
    {
        std::ostringstream oss;
        oss << "<OFIQ config_dir='" << config_dir_ << "' config_file='" << config_file_ << "'>";
        return oss.str();
    }

private:
    std::shared_ptr<OFIQ::Interface> impl_;
    std::string config_dir_;
    std::string config_file_;
};

PYBIND11_MODULE(_core, m)
{
    m.doc() = "Python bindings for OFIQ (Open Source Face Image Quality)";

    PyOFIQError = py::exception<OFIQError>(m, "OFIQError");
    PyConfigError = py::exception<ConfigError>(m, "ConfigError", PyOFIQError);
    PyQualityError = py::exception<QualityError>(m, "QualityError", PyOFIQError);
    PyFaceDetectionError = py::exception<FaceDetectionError>(m, "FaceDetectionError", PyQualityError);
    PyLandmarkError = py::exception<LandmarkError>(m, "LandmarkError", PyQualityError);
    PySegmentationError = py::exception<SegmentationError>(m, "SegmentationError", PyQualityError);

    m.attr("__version__") = OFIQPY_VERSION;
    m.attr("__ofiq_version__") = OFIQPY_OFIQ_VERSION;

    py::class_<OFIQWrapper>(m, "OFIQ", R"pbdoc(
        OFIQ face image quality assessment interface.

        Initialize once, then call quality methods repeatedly.
        Not thread-safe — use one instance per thread or serialize access.

        Args:
            config_dir: Path to OFIQ data directory. Defaults to
                OFIQ_DATA_DIR env var or ~/.ofiq/data.
            config_file: Config filename (default: "ofiq_config.jaxn").

        Raises:
            ConfigError: If config is missing or invalid.
            OFIQError: If initialization fails.
    )pbdoc")
        .def(py::init<const std::string&, const std::string&>(),
             py::arg("config_dir") = "",
             py::arg("config_file") = "ofiq_config.jaxn")

        .def("scalar_quality",
             &OFIQWrapper::scalar_quality,
             py::arg("image_rgb"),
             R"pbdoc(
                Compute unified scalar quality score from an RGB image.

                Args:
                    image_rgb: NumPy array of shape (H, W, 3), dtype uint8, RGB.

                Returns:
                    float: Quality score in [0, 100]. Higher is better.
             )pbdoc")

        .def("vector_quality",
             &OFIQWrapper::vector_quality,
             py::arg("image_rgb"),
             py::arg("include_raw") = false,
             R"pbdoc(
                Compute all quality measures from an RGB image.

                Args:
                    image_rgb: NumPy array of shape (H, W, 3), dtype uint8, RGB.
                    include_raw: If True, return dicts with 'scalar' and 'raw' keys.

                Returns:
                    dict: Measure names to scores. None for failed measures.
             )pbdoc")

        .def("__repr__", &OFIQWrapper::repr);
}
