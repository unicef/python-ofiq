# python-ofiq

Python bindings for [OFIQ](https://github.com/BSI-OFIQ/OFIQ-Project) (Open Source Face Image Quality) — the reference implementation for **ISO/IEC 29794-5**.

## Features

- One-time initialization, process many images efficiently
- Unified quality score [0-100] and 25+ individual quality metrics
- Custom exception hierarchy for precise error handling
- Full type hint support

## Installation

```bash
pip install python-ofiq
```

Download OFIQ models and config (~400 MB):

```bash
python-ofiq setup
```

This stores data in `~/.ofiq/data`. Set `OFIQ_DATA_DIR` to use a custom directory.

## Quick Start

```python
import cv2
from ofiq import OFIQ, FaceDetectionError

ofiq = OFIQ()

bgr = cv2.imread("face.png")
rgb = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)

try:
    score = ofiq.scalar_quality(rgb)
    print(f"Quality score: {score}")
except FaceDetectionError:
    print("No face detected")

measures = ofiq.vector_quality(rgb)
print(measures)
# {'UnifiedQualityScore': 85.2, 'Sharpness': 92.1, ...}

measures_with_raw = ofiq.vector_quality(rgb, include_raw=True)
print(measures_with_raw["Sharpness"])
# {'scalar': 92.1, 'raw': 0.847}
```

## Exception Handling

```python
from ofiq import (
    OFIQ,
    OFIQError,
    ConfigError,
    QualityError,
    FaceDetectionError,
    LandmarkError,
    SegmentationError,
)

try:
    score = ofiq.scalar_quality(image)
except FaceDetectionError:
    print("No face in image")
except LandmarkError:
    print("Could not extract facial landmarks")
except QualityError as e:
    print(f"Quality assessment failed: {e}")
except OFIQError as e:
    print(f"OFIQ error: {e}")
```

## Thread Safety

`OFIQ` instances are **not thread-safe**. The underlying C++ library uses shared mutable state without synchronization:

- Use one `OFIQ` instance per thread, or
- Serialize access with a lock, or
- Use `multiprocessing` instead of `threading`

## Scalar vs Raw Scores

Scalar scores are normalized to [0, 100] where higher is always better. Raw scores are the native OFIQ values before normalization — their range and direction vary per measure. Use `include_raw=True` when you need the original measurement values.

## Available Quality Measures

All measures return scalar values in [0, 100] where higher is better, or `None` if the measure failed:

| Measure | Description |
|---------|-------------|
| `UnifiedQualityScore` | Overall quality score |
| `Sharpness` | Image sharpness/focus |
| `BackgroundUniformity` | Background consistency |
| `IlluminationUniformity` | Lighting evenness |
| `LuminanceMean` | Average brightness |
| `LuminanceVariance` | Brightness variation |
| `DynamicRange` | Contrast range |
| `CompressionArtifacts` | JPEG artifact detection |
| `NaturalColour` | Color naturalness |
| `HeadPoseYaw` | Face yaw angle |
| `HeadPosePitch` | Face pitch angle |
| `HeadPoseRoll` | Face roll angle |
| `EyesOpen` | Eye openness |
| `MouthClosed` | Mouth closure |
| `EyesVisible` | Eye visibility |
| `ExpressionNeutrality` | Neutral expression |
| `SingleFacePresent` | Single face detection |
| `InterEyeDistance` | Distance between eyes |
| `HeadSize` | Face size in frame |
| ... | and more |

## API Reference

### `OFIQ(config_dir="", config_file="ofiq_config.jaxn")`

Initialize OFIQ. If `config_dir` is omitted, uses `OFIQ_DATA_DIR` env var or `~/.ofiq/data`.

### `scalar_quality(image_rgb) -> float`

Compute unified quality score. `image_rgb`: NumPy uint8 array of shape (H, W, 3) in RGB.

### `vector_quality(image_rgb, include_raw=False) -> dict`

Compute all quality measures. Returns `None` for failed individual measures.

### `ofiq.setup(force_download=False) -> str`

Download models and config. Returns the data directory path.

### `ofiq.resolve_data_dir() -> str`

Return the resolved data directory.

### CLI

```bash
python-ofiq setup                    # download models and config
python-ofiq setup --force-download   # re-download even if present
OFIQ_DATA_DIR=/custom/path python-ofiq setup
```

## Version Compatibility

| python-ofiq | OFIQ C++ |
|-------------|----------|
| 0.1.x       | 1.1.2    |

## Development

### Build from source

```bash
git clone https://github.com/unicef/python-ofiq.git
cd python-ofiq

docker build -f docker/Dockerfile -t python-ofiq-test .
docker run -it python-ofiq-test python -c "from ofiq import OFIQ; print('OK')"
```

### Run tests

```bash
pip install pytest numpy

pytest tests/ -v -m "not integration"

# Integration tests require models
pytest tests/ -v
```

## License

MIT — see [LICENSE](LICENSE).

This project wraps the [OFIQ C++ library](https://github.com/BSI-OFIQ/OFIQ-Project) which is also MIT licensed.

## Acknowledgments

- [BSI](https://www.bsi.bund.de/) for developing OFIQ
- [pybind11](https://github.com/pybind/pybind11) for the bindings framework
