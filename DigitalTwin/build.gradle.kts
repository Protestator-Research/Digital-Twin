plugins {
    kotlin("jvm") version "1.8.20"
    application
    id("org.openjfx.javafxplugin") version "0.0.13"
}

group   = "com.github.tukcps"
version = "1.0-SNAPSHOT"
val jaaddVersion = "3.0.41"
val sysmdVersion = "2.10.22"

repositories {
    mavenCentral()
    google()

    //jAADD
    maven (url = "https://cpsgit.informatik.uni-kl.de/api/v4/projects/87/packages/maven") {
        name = "GitLab"
        credentials(HttpHeaderCredentials::class) {
            name = "Deploy-Token"
            value = "m2XeQuM-1sqMXeUX-2-X"
        }
        authentication {
            create<HttpHeaderAuthentication>("header")
        }
    }

    // SysMD
    maven("https://cpsgit.informatik.uni-kl.de/api/v4/projects/117/packages/maven") {
        name = "GitLab"
        credentials(HttpHeaderCredentials::class) {
            name = "Deploy-Token"
            value = "jDN-qwxvyFW9DxXPqCJL"
        }
        authentication { create<HttpHeaderAuthentication>("header") }
    }

    // Jetpack Compose development
    maven {
        url = uri("https://maven.pkg.jetbrains.space/public/p/compose/dev")
        name = "Compose for Desktop DEV"
    }

    // LaTeX Rendering for UI
    maven ("https://jitpack.io")
}

dependencies {
//    val standalone: Boolean = if (org.gradle.internal.os.OperatingSystem.current().isWindows)
//        !File("${System.getProperty("user.home")}\\agila.hierarchical.build").exists()
//    else
//        !File("/tmp/agila.hierarchical.build").exists()

    if(file("../jaadd").exists()) {
        println("  *** using jaadd from local clone in ./jaadd           ***")
        implementation(project(":jaadd"))
    } else {
        println("  *** using jaadd $jaaddVersion from CPS Maven repo     ***")
        implementation("com.github.tukcps:jaadd:$jaaddVersion")
    }

    if (file("../sysmd").exists()) {
        println("  *** using SysMD from local clone in ./sysmd           ***")
        implementation(project(":sysmd"))
    } else {
        println("  *** using SysMD $sysmdVersion from CPS Maven repo     ***")
        implementation("com.github.tukcps:sysmd:$sysmdVersion")
    }

    testImplementation(kotlin("test"))

}

tasks.test {
    useJUnitPlatform()
}

kotlin {
    jvmToolchain(17)
}

application {
    mainClass.set("MainKt")
}


javafx {
    version = "20"
    modules = mutableListOf( "javafx.controls" )
}