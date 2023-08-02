plugins {
    kotlin("jvm") version "1.8.20"
    application
    id("org.javamodularity.moduleplugin") version "1.8.12"
    id("org.openjfx.javafxplugin") version "0.0.13"
    id("org.beryx.jlink") version "2.25.0"
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

    // LaTeX Rendering for UI
    maven ("https://jitpack.io")
}

dependencies {
    if(file("../jaadd").exists()) {
        println("  *** using jaadd from local clone in ./jaadd           ***")
        implementation(project(":jaadd"))
    } else {
        println("  *** using jaadd $jaaddVersion from CPS Maven repo     ***")
        implementation("com.github.tukcps:jaadd:$jaaddVersion")
    }

    implementation("io.moquette:moquette-broker:0.16")

    // Needed for annotations for Spring Boot in package rest
    implementation("org.json:json:20230227")
    implementation("com.fasterxml.jackson.core:jackson-databind:2.14.0")
    implementation("com.fasterxml.jackson.module:jackson-module-kotlin:2.14.+")
    implementation("com.fasterxml.jackson.datatype:jackson-datatype-jsr310:2.14.+")

    implementation(platform("com.hivemq:hivemq-mqtt-client-websocket:1.3.0"))
    implementation(platform("com.hivemq:hivemq-mqtt-client-proxy:1.3.0"))
    implementation(platform("com.hivemq:hivemq-mqtt-client-epoll:1.3.0"))
    implementation("com.hivemq:hivemq-mqtt-client-reactor:1.3.0")

    // Parsing markdown to AST
    implementation("org.commonmark:commonmark:0.21.0")
    implementation("org.commonmark:commonmark-ext-gfm-tables:0.21.0")
    implementation("org.commonmark:commonmark-ext-image-attributes:0.21.0")
    implementation("org.commonmark:commonmark-ext-yaml-front-matter:0.21.0")

    implementation("org.springframework:spring-web:5.3.27")


    testImplementation(kotlin("test"))
}

tasks.test {
    useJUnitPlatform()
}

kotlin {
    jvmToolchain(17)
}

application {
    mainClass.set("DT-Server.kt")
}


javafx {
    version = "20"
    modules = mutableListOf( "javafx.controls", "javafx.fxml" )
}