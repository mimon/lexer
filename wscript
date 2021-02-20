out = 'build'
APPNAME = 'lexer'

def options(opt):
	opt.load('compiler_cxx compiler_c')

def configure(cnf):
	cnf.env.CLANG_CXX_LIBRARY='libc++'
	cnf.env.CLANG_CXX_LANGUAGE_STANDARD='c++11'
	cnf.env.GCC_WARN_ABOUT_RETURN_TYPE='Yes'
	cnf.env.GCC_OPTIMIZATION_LEVEL='0'
	cnf.load('compiler_cxx')
	cnf.check(cxxflags='-std=c++11', uselib_store='STD11', mandatory=False)

def build(bld):

	bld.program(
		target='tests',
		source=bld.path.ant_glob('src/lexer/tests/*.cpp'),
		includes='src',
		cxxflags=['-std=c++11']
	)

	bld.stlib(
		target='lexer',
		source=bld.path.ant_glob('src/lexer/*.cpp'),
		includes='src',
		export_includes='src',
		cxxflags='-std=c++11'
	)
