# 
# The Violet Project
# 
# Copyright (c) 2019, Johns Hopkins University - Order Lab.
#
#    All rights reserved.
#    Licensed under the Apache License, Version 2.0 (the "License");
#

cmake_minimum_required(VERSION 3.4.3)

project(violet_lib)

if(DEFINED ENV{S2EDIR})
  MESSAGE(STATUS "S2EDIR is set to $ENV{S2EDIR}")
  SET(S2ESRC "$ENV{S2EDIR}/source/s2e")
  SET(S2ESRC_INCLUDE "${S2ESRC}/guest/common/include")
  include_directories(${S2ESRC_INCLUDE})
  SET(HAVE_VIOLET_S2E ON CACHE BOOL "Violet S2E test")
else()
  MESSAGE(WARNING "S2EDIR environment variable is not set, building libviolet without S2E.")
endif()

add_subdirectory(lib)