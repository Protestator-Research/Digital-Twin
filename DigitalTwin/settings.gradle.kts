
rootProject.name = "DigitalTwin"

if (file("../sysmd").exists()) {
    include(":sysmd")
    project(":sysmd").projectDir = file("../sysmd")
}
