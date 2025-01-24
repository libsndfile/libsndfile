@file:Suppress("UnstableApiUsage")

require(gradle.gradleVersion == "8.9") {
    "Gradle version 8.9 required (current version: ${gradle.gradleVersion})"
}

plugins {
    alias(libs.plugins.library)
    id("maven-publish")
}

// project.name ("sndfile") defined in settings.gradle.kts
project.group = "com.meganerd"
project.version = "1.2.2-android-r1"

val abis = listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")

android {
    namespace = "${project.group}.${project.name}"
    compileSdk = libs.versions.compilesdk.get().toInt()

    defaultConfig {
        minSdk = libs.versions.minsdk.get().toInt()

        buildToolsVersion = libs.versions.buildtools.get()
        ndkVersion = libs.versions.ndk.get()
        ndk {
            abiFilters += abis
        }
        externalNativeBuild {
            // build static libs and testing binaries only when running :ndkTest
            val buildSharedLibs = if (isTestBuild()) "OFF" else "ON"
            val buildTesting = if (isTestBuild()) "ON" else "OFF"

            cmake {
                cppFlags += "-std=c++17"
                arguments += "-DANDROID_STL=c++_shared"
                arguments += "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON"

                arguments += "-DBUILD_SHARED_LIBS=$buildSharedLibs"
                arguments += "-DBUILD_TESTING=$buildTesting"
                arguments += "-DENABLE_EXTERNAL_LIBS=ON"
                arguments += "-DENABLE_MPEG=ON"
                arguments += "-DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON"

                arguments += "-DBUILD_PROGRAMS=OFF"
                arguments += "-DBUILD_EXAMPLES=OFF"
                arguments += "-DENABLE_CPACK=OFF"
                arguments += "-DENABLE_PACKAGE_CONFIG=OFF"
                arguments += "-DINSTALL_PKGCONFIG_MODULE=OFF"
                arguments += "-DINSTALL_MANPAGES=OFF"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("${projectDir.parentFile}/CMakeLists.txt")
            version = libs.versions.cmake.get()
        }
    }

    buildFeatures {
        prefab = true
    }
}

dependencies {
    // ogg is a transitive dependency of vorbis, so it doesn't need to be specified explicitly
    implementation(libs.vorbis)
    implementation(libs.opus)
    implementation(libs.flac)
    implementation(libs.mpg123)
    implementation(libs.lame)
}

tasks.register<Zip>("prefabAar") {
    archiveFileName = "${project.name}-release.aar"
    destinationDirectory = file("build/outputs/prefab-aar")

    from("aar-template")
    from("${projectDir.parentFile}/include") {
        include("**/*.h")
        include("**/*.hh")
        into("prefab/modules/${project.name}/include")
    }
    abis.forEach { abi ->
        from("build/intermediates/cmake/release/obj/$abi") {
            include("lib${project.name}.so")
            into("prefab/modules/${project.name}/libs/android.$abi")
        }
    }
}

tasks.register<Exec>(getTestTaskName()) {
    commandLine("./ndk-test.sh")
}

tasks.named<Delete>("clean") {
    delete.add(".cxx")
}

afterEvaluate {
    tasks.named("preBuild") {
        mustRunAfter("clean")
    }

    tasks.named("prefabAar") {
        dependsOn("externalNativeBuildRelease")
    }

    tasks.named("generatePomFileFor${project.name.cap()}Publication") {
        mustRunAfter("prefabAar")
    }
    tasks.named("publish") {
        dependsOn("clean", "prefabAar")
    }

    tasks.named(getTestTaskName()) {
        dependsOn("clean", "externalNativeBuildRelease")
    }
}


publishing {
    val githubPackagesUrl = "https://maven.pkg.github.com/jg-hot/libsndfile-android"

    repositories {
        maven {
            url = uri(githubPackagesUrl)
            credentials {
                username = properties["gpr.user"]?.toString()
                password = properties["gpr.key"]?.toString()
            }
        }
    }

    publications {
        create<MavenPublication>(project.name) {
            artifact("build/outputs/prefab-aar/${project.name}-release.aar")
            artifactId = "${project.name}-android"

            pom {
                distributionManagement {
                    downloadUrl = githubPackagesUrl
                }
                withXml {
                    val dependencies = asNode().appendNode("dependencies")
                    configurations.implementation.get().dependencies.forEach {
                        val dependency = dependencies.appendNode("dependency")
                        dependency.appendNode("groupId", it.group)
                        dependency.appendNode("artifactId", it.name)
                        dependency.appendNode("version", it.version)
                    }
                }
            }
        }
    }
}

fun getTestTaskName(): String = "ndkTest"

fun isTestBuild(): Boolean = gradle.startParameter.taskNames.contains(getTestTaskName())

// capitalize the first letter to make task names matched when written in camel case
fun String.cap(): String = this.replaceFirstChar { it.uppercase() }

