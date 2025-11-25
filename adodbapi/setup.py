"""adodbapi -- a pure Python PEP 249 DB-API package using Microsoft ADO

Adodbapi can be run on CPython 3.9 and later.
"""

from setuptools import setup

VERSION = None  # in case searching for version fails
with open("adodbapi.py") as a:  # find the version string in the source code
    for line in a:
        if "__version__" in line:
            VERSION = line.split('"')[1]  # pyright: ignore[reportConstantRedefinition]
            print(f'adodbapi version="{VERSION}"')
            break
assert VERSION

setup(
    name="adodbapi",
    maintainer="Vernon Cole",
    maintainer_email="vernondcole@gmail.com",
    description="A pure Python package implementing PEP 249 DB-API using Microsoft ADO.",
    url="https://sourceforge.net/projects/adodbapi",
    keywords="database ado odbc dbapi db-api Microsoft SQL",
    long_description=open("README.txt").read(),
    license="LGPL",
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)",
        "Operating System :: Microsoft :: Windows",
        "Operating System :: POSIX :: Linux",
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "Programming Language :: SQL",
        "Topic :: Software Development",
        "Topic :: Software Development :: Libraries :: Python Modules",
        "Topic :: Database",
    ],
    author="Henrik Ekelund, Vernon Cole, et.al.",
    author_email="vernondcole@gmail.com",
    platforms=["Windows", "Linux"],
    version=VERSION,
    package_dir={"adodbapi": ""},
    packages=["adodbapi"],
)
