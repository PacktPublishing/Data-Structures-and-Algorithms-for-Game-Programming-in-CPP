plugins {
    id("com.android.application")
}

val knightGame = providers.gradleProperty("knightGame")
    .orElse("Demo1")
    .get()

android {
    namespace = "com.knight.cookbook"
    compileSdk = 36
    ndkVersion = "30.0.16248370"

    signingConfigs {
        create("knight") {
            storeFile = rootProject.file("knight.keystore")
            storePassword = "knightapp"
            keyAlias = "knightapp"
            keyPassword = "knightapp"
        }
    }

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
                    "-DKNIGHT_GAME=$knightGame",
                    "-DANDROID_STL=c++_shared"
                )
                targets += knightGame
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.31.6"
        }
    }

    buildTypes {
        getByName("debug") {
            signingConfig = signingConfigs.getByName("knight")
        }
        getByName("release") {
            signingConfig = signingConfigs.getByName("knight")
        }
    }

    sourceSets["main"].assets.srcDir("../../resources")
}

dependencies {
    // KnightActivity: fold state (WindowInfoTracker) for Knight's Display class.
    implementation("androidx.window:window:1.4.0")
    implementation("androidx.window:window-java:1.4.0")
    implementation("androidx.core:core:1.13.1")
}

