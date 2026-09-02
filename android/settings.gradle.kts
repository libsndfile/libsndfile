@file:Suppress("UnstableApiUsage")

rootProject.name = "sndfile"

pluginManagement {
    repositories {
        google {
            content {
                includeGroupByRegex("com\\.android.*")
                includeGroupByRegex("com\\.google.*")
                includeGroupByRegex("androidx.*")
            }
        }
        mavenCentral()
        gradlePluginPortal()
    }
}
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        google()
        mavenCentral()
        listOf(
            "jg-hot" to "libogg-android",
            "jg-hot" to "libvorbis-android",
            "jg-hot" to "libopus-android",
            "jg-hot" to "libflac-android",
            "jg-hot" to "libmpg123-android",
            "jg-hot" to "libmp3lame-android",
        ).forEach { (user, repo) ->
            maven {
                name = "gpr:$repo"
                url = uri("https://maven.pkg.github.com/$user/$repo")
                credentials {
                    username = extra.properties["gpr.user"].toString()
                    password = extra.properties["gpr.key"].toString()
                }
            }
        }
    }
}
