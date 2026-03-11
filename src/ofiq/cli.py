"""Command-line interface for python-ofiq."""

import argparse
import sys


def _cmd_setup(args: argparse.Namespace) -> None:
    from ofiq.setup import setup
    setup(force_download=args.force_download)


def main(argv: list[str] | None = None) -> None:
    parser = argparse.ArgumentParser(
        prog="python-ofiq",
        description="Python bindings for OFIQ (Open Source Face Image Quality)",
    )
    sub = parser.add_subparsers(dest="command")

    setup_parser = sub.add_parser("setup", help="Download OFIQ models and config")
    setup_parser.add_argument(
        "--force-download",
        action="store_true",
        help="Re-download even if files already exist",
    )
    setup_parser.set_defaults(func=_cmd_setup)

    args = parser.parse_args(argv)

    if not hasattr(args, "func"):
        parser.print_help()
        sys.exit(1)

    args.func(args)


if __name__ == "__main__":
    main()
