from setuptools import setup

setup(
    name="lgraph_cypher",
    description='cypher quarry command line tool for TuGraph',
    install_requires=[
        'click==7.0',
        'prompt_toolkit==2.0.9',
        'prettytable==0.7.2',
        'requests==2.32.2',
    ],
    packages=['lgraph_shell'],
    entry_points={
        'console_scripts': [
            'lgraph_cypher = lgraph_shell.lgraph_cypher:start'
        ]
    }
)
