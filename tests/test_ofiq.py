import pytest
import numpy as np


def test_import():
    from ofiq import OFIQ
    assert OFIQ is not None


def test_version():
    from ofiq import __version__, __ofiq_version__
    assert isinstance(__version__, str)
    assert isinstance(__ofiq_version__, str)
    assert len(__version__) > 0
    assert len(__ofiq_version__) > 0


def test_class_methods():
    from ofiq import OFIQ
    assert hasattr(OFIQ, "scalar_quality")
    assert hasattr(OFIQ, "vector_quality")


def test_exception_hierarchy():
    from ofiq import (
        OFIQError,
        ConfigError,
        QualityError,
        FaceDetectionError,
        LandmarkError,
        SegmentationError,
    )
    assert issubclass(ConfigError, OFIQError)
    assert issubclass(QualityError, OFIQError)
    assert issubclass(FaceDetectionError, QualityError)
    assert issubclass(LandmarkError, QualityError)
    assert issubclass(SegmentationError, QualityError)


def test_config_error_on_bad_path():
    from ofiq import OFIQ, ConfigError, OFIQError
    with pytest.raises((ConfigError, OFIQError)):
        OFIQ("/nonexistent/path/that/does/not/exist")


@pytest.mark.integration
def test_scalar_quality_blank_image(ofiq_instance):
    from ofiq import FaceDetectionError
    img = np.zeros((480, 640, 3), dtype=np.uint8)
    with pytest.raises(FaceDetectionError):
        ofiq_instance.scalar_quality(img)


@pytest.mark.integration
def test_scalar_quality_with_face(ofiq_instance, face_image):
    score = ofiq_instance.scalar_quality(face_image)
    assert isinstance(score, float)
    assert 0.0 <= score <= 100.0


@pytest.mark.integration
def test_vector_quality(ofiq_instance, face_image):
    scores = ofiq_instance.vector_quality(face_image)
    assert isinstance(scores, dict)
    assert len(scores) > 0
    for key, value in scores.items():
        assert isinstance(key, str)
        assert value is None or isinstance(value, float)


@pytest.mark.integration
def test_vector_quality_with_raw(ofiq_instance, face_image):
    scores = ofiq_instance.vector_quality(face_image, include_raw=True)
    assert isinstance(scores, dict)
    for key, value in scores.items():
        assert isinstance(key, str)
        if value is not None:
            assert isinstance(value, dict)
            assert "scalar" in value
            assert "raw" in value


@pytest.mark.integration
def test_repr(ofiq_instance):
    r = repr(ofiq_instance)
    assert "OFIQ" in r
    assert "config_dir" in r


@pytest.mark.integration
def test_exception_catchable_at_parent_level(ofiq_instance):
    from ofiq import FaceDetectionError, QualityError

    img = np.zeros((480, 640, 3), dtype=np.uint8)

    with pytest.raises(FaceDetectionError):
        ofiq_instance.scalar_quality(img)

    with pytest.raises(QualityError):
        ofiq_instance.scalar_quality(img)
