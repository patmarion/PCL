# Find and set Boost flags

if(NOT PCL_SHARED_LIBS)
    set(Boost_USE_STATIC_LIBS ON)
endif(NOT PCL_SHARED_LIBS)

find_package(Boost 1.40.0 COMPONENTS system thread date_time)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
