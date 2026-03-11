"""Download and manage OFIQ models and configuration."""

import os
import zipfile
from pathlib import Path
from urllib.request import urlretrieve

_DEFAULT_DATA_DIR = os.path.join("~", ".ofiq", "data")
_MODELS_URL = "https://resources.eulisa.europa.eu/research/OFIQ-Models.zip"
_CONFIG_URL_TEMPLATE = (
    "https://raw.githubusercontent.com/BSI-OFIQ/OFIQ-Project/"
    "v{version}/data/ofiq_config.jaxn"
)


def resolve_data_dir() -> str:
    """Return the OFIQ data directory.

    Checks ``OFIQ_DATA_DIR`` environment variable first,
    falls back to ``~/.ofiq/data``.
    """
    return os.path.expanduser(
        os.environ.get("OFIQ_DATA_DIR", _DEFAULT_DATA_DIR)
    )


def _progress_hook(block_num: int, block_size: int, total_size: int) -> None:
    if total_size <= 0:
        print(f"\r  Downloaded {block_num * block_size / 1024 / 1024:.1f} MB", end="", flush=True)
        return
    pct = min(100.0, block_num * block_size / total_size * 100)
    mb = block_num * block_size / 1024 / 1024
    total_mb = total_size / 1024 / 1024
    print(f"\r  {pct:5.1f}%  ({mb:.1f} / {total_mb:.1f} MB)", end="", flush=True)


def setup(*, force_download: bool = False) -> str:
    """Download OFIQ models and config to the resolved data directory.

    Args:
        force_download: Re-download even if files already exist.

    Returns:
        The resolved data directory path.
    """
    from ofiq._core import __ofiq_version__

    data_dir = resolve_data_dir()
    config_path = os.path.join(data_dir, "ofiq_config.jaxn")
    models_dir = os.path.join(data_dir, "models")

    print(f"python-ofiq setup (OFIQ v{__ofiq_version__})")
    print(f"Data directory: {data_dir}")

    if os.path.isfile(config_path) and os.path.isdir(models_dir) and not force_download:
        print("Models and config already present. Use --force-download to re-download.")
        return data_dir

    Path(data_dir).mkdir(parents=True, exist_ok=True)

    config_url = _CONFIG_URL_TEMPLATE.format(version=__ofiq_version__)
    print(f"Downloading config from OFIQ v{__ofiq_version__}...")
    urlretrieve(config_url, config_path)
    print(f"  Saved {config_path}")

    zip_path = os.path.join(data_dir, "_OFIQ-Models.zip")
    print("Downloading models (~400 MB)...")
    urlretrieve(_MODELS_URL, zip_path, reporthook=_progress_hook)
    print()

    print("Extracting models...")
    with zipfile.ZipFile(zip_path, "r") as zf:
        zf.extractall(data_dir)
    os.remove(zip_path)
    print(f"  Extracted to {data_dir}")

    print("Setup complete.")
    return data_dir
