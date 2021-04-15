import qbs

CppApplication {
    name: "fwapp"

    Depends { name: "libopencm3" }

    cpp.cLanguageVersion: "c99"
    cpp.positionIndependentCode: false

    cpp.commonCompilerFlags: [
        "-Wextra",
        "-Wimplicit-function-declaration",
        "-Wmissing-prototypes",
        "-Wredundant-decls",
        "-Wshadow",
        "-Wstrict-prototypes",
        "-Wundef",
    ]

    cpp.driverLinkerFlags: [
        "--static",
        "-nostartfiles",
    ]

    cpp.linkerFlags: [
        "--cref",
        "--gc-sections",
    ]

    cpp.staticLibraries: [
        "c", "gcc", "nosys"
    ]

    Properties {
        condition: project.targetMcu === "stm32f1"
        cpp.driverFlags: [
            "-fdata-sections",
            "-ffunction-sections",
            "-fno-common",
            "-mcpu=cortex-m3",
            "-mfix-cortex-m3-ldrd",
            "-msoft-float",
            "-mthumb",
        ]
    }

    files: [
        "fwapp.c",
        "fwapp_led.c",
        "fwapp_led.h",
        "fwapp_systick.c",
        "fwapp_systick.h",
        "fwapp_usb_composite.c",
        "fwapp_usb_composite.h",
    ]

    Group {
        name: "Linker scripts"
        fileTags: ["linkerscript"]
        files: [
            "fwapp.ld",
            "../../shared/libopencm3/content/lib/cortex-m-generic.ld",
        ]
    }
}
