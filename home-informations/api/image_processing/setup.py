import numpy
from Cython.Build import cythonize
from setuptools import Extension, setup

# Define the extension module
ext_modules = [
    Extension(
        "image_processing.floyd_steinberg",
        sources=["src/image_processing/floyd_steinberg.pyx"],
    )
]

# Setup function
setup(
    ext_modules=cythonize(ext_modules),
    include_dirs=[numpy.get_include()],
)
