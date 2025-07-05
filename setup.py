import pybind11
from setuptools import Extension
from setuptools import setup

setup(
    name="orderbook",
    version="0.1.0",
    packages=["orderbook"],
    package_dir={"orderbook": "orderbook"},
    include_package_data=True,
    zip_safe=False,
)
