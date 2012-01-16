SET(PIRANHA_ENABLE_BOOST_THREAD FALSE)

IF(UNIX)
	# Install path for libraries.
	SET(LIB_INSTALL_PATH "lib")
	# Enable the pthread flag in Unix only if the compiler is GNU.
	# NOTE: here the situation is very hairy: different systems require different GCC flags:
	# http://gcc.gnu.org/onlinedocs/libstdc++/manual/using_concurrency.html
	# This will work at least on Linux/x86 and FreeBSD.
	IF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_USE_PTHREADS_INIT)
		MESSAGE(STATUS "GCC with POSIX threads detected.")
		# NOTE: according to GCC docs, this sets the flag for both compiler and linker.
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
		# Set definitions in config.hpp.
		SET(PIRANHA_THREAD_MODEL "#define PIRANHA_THREAD_MODEL_PTHREADS")
	ENDIF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_USE_PTHREADS_INIT)
ENDIF(UNIX)

IF(MINGW)
	# The -mthreads flag is needed both in compiling and linking.
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mthreads")
	SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mthreads")
	SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -mthreads")
	SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -mthreads")
	# NOTE: in MinGW there is no support for c++0x threads yet.
	SET(PIRANHA_ENABLE_BOOST_THREAD TRUE)
ENDIF(MINGW)

# Setup for the machinery to detect cache line size in Windows. It's not supported everywhere, so we
# check for the existence of the SYSTEM_LOGICAL_PROCESSOR_INFORMATION type.
# http://msdn.microsoft.com/en-us/library/ms686694(v=vs.85).aspx
IF(WIN32)
    INCLUDE(CheckTypeSize)
	SET(CMAKE_EXTRA_INCLUDE_FILES Windows.h)
	CHECK_TYPE_SIZE("SYSTEM_LOGICAL_PROCESSOR_INFORMATION" SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
	SET(CMAKE_EXTRA_INCLUDE_FILES)
	IF(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
		SET(PIRANHA_SYSTEM_LOGICAL_PROCESSOR_INFORMATION "#define PIRANHA_HAVE_SYSTEM_LOGICAL_PROCESSOR_INFORMATION")
	ENDIF(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
	SET(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)
ENDIF(WIN32)

IF(UNIX OR MINGW)
	# Look for the math library and if found set it as mandatory
	FIND_LIBRARY(SYSTEM_M_LIBRARY NAMES m)
	IF(SYSTEM_M_LIBRARY)
		SET(MANDATORY_LIBRARIES ${MANDATORY_LIBRARIES} ${SYSTEM_M_LIBRARY})
		MESSAGE(STATUS "Math library found: ${SYSTEM_M_LIBRARY}")
	ENDIF(SYSTEM_M_LIBRARY)
ENDIF(UNIX OR MINGW)
