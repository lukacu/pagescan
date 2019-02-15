# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.4.1)

add_library( # Sets the name of the library.
             pagescan

             # Sets the library as a shared library.
             SHARED

             src/main/cpp/conversions.cpp
             src/main/cpp/native.cpp
             src/main/cpp/detector.cpp
             )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              AndroidLibs

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

target_link_libraries( # Specifies the target library.
                       pagescan

                       # Links the target library to the log library
                       # included in the NDK.
                       ${AndroidLibs}  android jnigraphics )

FIND_PACKAGE(OpenCV REQUIRED)

INCLUDE_DIRECTORIES(AFTER ${OpenCV_INCLUDE_DIR})
LINK_LIBRARIES(${OpenCV_LIBS})
TARGET_LINK_LIBRARIES(pagescan ${OpenCV_LIBS}) # Add other OpenCV modules if needed
#TARGET_LINK_LIBRARIES(pagescan IlmImf libjasper libjpeg libpng libtiff libwebp tbb z log m log android jnigraphics)