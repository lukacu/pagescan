PageScan - Android scanner app
==============================

This app is roughly based on a student project [ScanIt](https://github.com/AndrejHafner/ScanIt), modified, improved, and cleaned up.

The app is an example of using OpenCV in your Android app without using generated Java bindings. This produces a much smaller app because you have control over what is actually embedded into it and does not require installation of other apps, such as OpenCV Manager.

Setup
-----

The project is compiled using Android Studio and uses Gradle and CMake. You must have android NDK and OpenCV for Android installed. Paths to local installations of requirements should be set using `local.properties` file:

```
ndk.dir=<path to Android NDK>
sdk.dir=<path to Android SDK>
opencv.dir=<path to OpenCV for Android SDK>
```

Structure
---------

The project is organized in a standard Gradle Android app structure. The native C++ code is available in `pagescan/src/main/cpp`, Java code is available in `pagescan/src/main/java`. The main class of the demo application is `si.vicos.pagescan.CameraActivity`.

