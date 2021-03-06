import qbs

QtApplication {
    condition: qbs.targetOS.contains("windows") || qbs.targetOS.contains("linux")
    name: "guiapp"

    Depends { name: "Qt"; submodules: ["core-private", "widgets"] }

    files: [
        "controlappwindow.cpp",
        "controlappwindow.h",
        "controlappwindow.ui",
        "controldevice.cpp",
        "controldevice.h",
        "controldevice_p.h",
        "controldeviceinfo.cpp",
        "controldeviceinfo.h",
        "controldeviceinfo_p.h",
        "controlreport.cpp",
        "controlreport.h",
        "guiapp.cpp",
    ]

    Group {
        name: "windows"
        files: [
            "controldevice_win.cpp",
            "controldeviceinfo_win.cpp",
        ]
    }

    Properties {
        condition: qbs.targetOS.contains("windows")
        cpp.staticLibraries: [ "setupapi", "hid" ]
    }
}
