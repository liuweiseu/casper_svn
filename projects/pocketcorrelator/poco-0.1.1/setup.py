import glob

__version__ = '0.1.1'

setup_args = {
    'name': 'poco',
    'author': 'Griffin Foster',
    'author_email': 'gfoster at berkeley.edu',
    'license': 'GPL',
    'package_dir': {'poco':'poco'},
    'packages': ['poco'],
    'scripts': glob.glob('scripts/*.py'),
    'version': __version__,
}

if __name__ == '__main__':
    from distutils.core import setup
    apply(setup, (), setup_args)

