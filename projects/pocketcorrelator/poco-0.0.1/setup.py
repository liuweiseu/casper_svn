import glob

__version__ = '0.0.1'

setup_args = {
    'name': 'poco',
    'author': 'Jason Manley and Sandeep Chaudhari',
    'author_email': 'jason_manley at hotmail.com',
    'license': 'GPL',
    'package_dir': {'poco':'poco'},
    'packages': ['poco'],
    'scripts': glob.glob('scripts/*.*'),
    'package_data': {'poco': ['LICENSE.txt']},
    'version': __version__,
}

if __name__ == '__main__':
    from distutils.core import setup
    apply(setup, (), setup_args)
