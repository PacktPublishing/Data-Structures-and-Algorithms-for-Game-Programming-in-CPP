plugins {
    id("com.android.application")
}

android {
    namespace = "com.knight.cookbook"
    compileSdk = 36
    ndkVersion = "30.0.16248370"

    defaultConfig {
        applicationId = "com.knight.cookbook"
        minSdk = 23
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"

        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                arguments += listOf(
                    "-DKNIGHT_BUILD_ALL_GAMES=OFF",
                    "-DKNIGHT_GAME=Demo1",
                    "-DANDROID_STL=c++_shared"
                )
                targets += "Demo1"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.31.6"
        }
    }

    sourceSets["main"].assets.srcDir("../../resources")
}

