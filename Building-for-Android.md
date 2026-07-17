# Building for Android

An Android `gradle` project is located in the `android/` directory. The project
uses the standard [NDK CMake](https://developer.android.com/ndk/guides/cmake)
build system to generate a [prefab](https://google.github.io/prefab/) NDK package.

The default build script uses [NDK r27 LTS](https://developer.android.com/ndk/downloads/revision_history)
and supports [16 KB page sizes](https://developer.android.com/guide/practices/page-sizes)
as required in Android 15 (API 35+).

## Building the prefab package / .aar
The following commands will build `libsndfile` as a prefab NDK package and place
it into an [.aar](https://developer.android.com/studio/projects/android-library) library.

You will need `gradle` version 8.9 (exactly) installed in in your path to ensure compatibility
with AGP.
```
cd android/
gradle assembleRelease
```

The resulting `.aar` will be located at:
`android/build/outputs/aar/sndfile-release.aar`

If you need to specify additional arguments to the `cmake` build, change the
NDK version used for the build, etc, you can do so by editing the `gradle` build
script located at:

`android/build.gradle.kts`

## Using as a dependency
After building the `.aar`, do one of the following:
1. `gradle publishToMavenLocal` is already supported in the build script
2. `gradle publishToMavenRepository` is not setup, but you can edit `android/build.gradle.kts`
   to add your own maven repository to publish to
3. Copy the `.aar` directly to the `libs/` directory of your project (not recommended)

Then, add the library to your project's dependencies in your `build.gradle.kts`:
```
dependencies {
    implementation("com.meganerd:sndfile-android:1.2.2-android-rc2")
}
```

Enable `prefab` support in your `build.gradle.kts`:
```
android {
    buildFeatures {
        prefab = true
    }
}
```

Update your `CMakeLists.txt` to find and link the prefab package, which will be
extracted from the `aar` by the build system:

```
find_package(sndfile REQUIRED CONFIG)

target_link_libraries(${CMAKE_PROJECT_NAME} sndfile::sndfile)
```

That's it! You can now `#include <sndfile.hh>` in your NDK source code.

## Testing on a device
To run the tests, follow these steps:
1. Ensure `adb` is in your path.
2. Have a single device (or emulator) connected and in debug mode. The testing task
only supports a single device. If you have more than one connected (or none) it will
notify you with an error.
3. You will also need `bash` to run the test script

Run the following commands:
```
cd android/
gradle ndkTest
```

The test task `:ndkTest` will run `gradle clean assembleRelease` with the following
options set for testing:
* `-DBUILD_SHARED_LIBS=OFF`
* `-DBUILD_TESTING=ON`

Then it runs `android/ndk-test.sh`, which pushes the binaries located at 
`android/build/intermediates/cmake/release/obj/$ABI` to `/data/local/tmp/libsndfile/test`
on the device, and uses `adb` to execute them. The results will be printed to the console.

# Building for Android (old instructions)

Assuming the Android Ndk is installed at location `/path/to/toolchain`, building
libsndfile for Android (arm-linux-androideabi) should be as simple as:
```
autoreconf -vif
export ANDROID_TOOLCHAIN_HOME=/path/to/android/toolchain
./Scripts/android-configure.sh
make
```
The `Scripts/android-configure.sh` contains four of variables; `ANDROID_NDK_VER`,
`ANDROID_GCC_VER`, `ANDROID_API_VER` and `ANDROID_TARGET` that can be overridden
by setting them before the script is run.

Since I (erikd), do almost zero Android development, I am happy accept patches
for this documentation and script to improve its utility for real Android
developers.

---

## Using CMake

(Tested on Linux)

For convenience, export the following variables:

```
export ANDROID_ABI=arm64-v8a
export ANDROID_PLATFORM_API_LEVEL=29
export NDK_ROOT=/path/to/android/ndk
```

Set `ANDROID_ABI`,  `ANDROID_PLATFORM_API_LEVEL`  according to your target system. Now cd into the libsndfile root directory, and run

```
cmake -S . -B build  -DCMAKE_TOOLCHAIN_FILE=$NDK_ROOT/build/cmake/android.toolchain.cmake -DANDROID_ABI=$ANDROID_ABI -DANDROID_PLATFORM=$ANDROID_PLATFORM_API_LEVEL
```

cd into `build` and run make

```
cd build
make [-j <number of parallel jobs>]
```

This will build libsndfile for android.
