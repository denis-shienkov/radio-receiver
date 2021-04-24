import qbs

QtApplication {
    condition: qbs.targetOS.contains("windows") || qbs.targetOS.contains("linux")
    name: "guiapp"

    Depends { name: "Qt"; submodules: ["widgets"] }

    files: [
        "controlappwindow.cpp",
        "controlappwindow.h",
        "controlappwindow.ui",
        "controldeviceinfo.cpp",
        "controldeviceinfo.h",
        "controldeviceinfo_p.h",
        "guiapp.cpp",
    ]

    Group {
        name: "windows"
        files: [
            "controldeviceinfo_win.cpp",
        ]
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.staticLibraries: [ "setupapi", "hid" ]
    }
}
