cmake_minimum_required (VERSION 3.4)

find_package(gstreamer-video-1.0 REQUIRED)

add_library(gstbreakout
	MODULE gstbreakout.h
               gstbreakout.c)

target_link_libraries(gstbreakout)
