from setuptools import setup

setup(
    name='TuGraphClient',
    version='2.0',
    description='rest client for TuGraph',
    install_requires=[
        'httpx',
    ],
    test_suite="unit_tests",
    packages=[''],
    entry_points={}
)
