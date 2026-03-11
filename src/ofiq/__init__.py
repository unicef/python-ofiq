"""Python bindings for OFIQ (Open Source Face Image Quality).

OFIQ is the reference implementation for ISO/IEC 29794-5.

Example::

    from ofiq import OFIQ, FaceDetectionError

    ofiq = OFIQ()
    try:
        score = ofiq.scalar_quality(rgb_image)
    except FaceDetectionError:
        print("No face detected")
"""

from ofiq._core import (
    OFIQ,
    __version__,
    __ofiq_version__,
    OFIQError,
    ConfigError,
    QualityError,
    FaceDetectionError,
    LandmarkError,
    SegmentationError,
)
from ofiq.setup import resolve_data_dir, setup

__all__ = [
    "OFIQ",
    "__version__",
    "__ofiq_version__",
    "OFIQError",
    "ConfigError",
    "QualityError",
    "FaceDetectionError",
    "LandmarkError",
    "SegmentationError",
    "resolve_data_dir",
    "setup",
]
