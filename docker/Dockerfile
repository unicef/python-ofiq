FROM quay.io/pypa/manylinux_2_28_x86_64 AS builder

RUN yum install -y git

WORKDIR /project
COPY . /project/

RUN bash /project/scripts/build_ofiq.sh

ENV OFIQ_ROOT=/opt/OFIQ-Project/install_x86_64_linux/Release

RUN /opt/python/cp311-cp311/bin/pip install build pybind11 scikit-build-core
RUN /opt/python/cp311-cp311/bin/python -m build --wheel --outdir /wheels/raw

RUN auditwheel repair /wheels/raw/*.whl -w /wheels/final

FROM python:3.14-slim AS runtime

RUN apt-get update && apt-get install -y --no-install-recommends \
    libgl1 \
    libglib2.0-0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /wheels/final/*.whl /tmp/wheels/

RUN pip install --no-cache-dir /tmp/wheels/*.whl numpy opencv-python-headless

COPY --from=builder /opt/OFIQ-Project/data /opt/OFIQ-Project/data

ENV OFIQ_DATA_DIR=/opt/OFIQ-Project/data

RUN python -c "from ofiq import OFIQ, __version__; print(f'python-ofiq {__version__} installed')"

WORKDIR /workspace
CMD ["python"]
