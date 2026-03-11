import os

import pytest


def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "integration: marks tests that require OFIQ models to be present"
    )


@pytest.fixture
def ofiq_instance():
    from ofiq import OFIQ
    from ofiq.setup import resolve_data_dir

    data_dir = resolve_data_dir()
    if not os.path.exists(data_dir):
        pytest.skip(f"OFIQ data not found: {data_dir}")

    return OFIQ(data_dir)


@pytest.fixture
def face_image():
    from ofiq.setup import resolve_data_dir

    images_dir = os.path.join(resolve_data_dir(), "tests", "images")
    if not os.path.exists(images_dir):
        pytest.skip("No test images found")

    import cv2
    for name in os.listdir(images_dir):
        if name.endswith((".png", ".jpg")):
            bgr = cv2.imread(os.path.join(images_dir, name))
            if bgr is not None:
                return cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)

    pytest.skip("No readable test image found")
