import qbs

QtApplication {
    condition: qbs.targetOS.contains("windows") || qbs.targetOS.contains("linux")
    name: "guiapp"

    Depends { name: "Qt"; submodules: ["widgets"] }

    files: [
        "guiapp.cpp"
    ]
}
