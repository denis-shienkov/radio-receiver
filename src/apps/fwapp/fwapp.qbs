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
        "c", "gcc", "m", "nosys"
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
            "-u_printf_float",
        ]
    }

    files: [
        "fwapp.c",
        "fwapp_cbuf.c",
        "fwapp_cbuf.h",
        "fwapp_hid.c",
        "fwapp_hid.h",
        "fwapp_led.c",
        "fwapp_led.h",
        "fwapp_systick.c",
        "fwapp_systick.h",
        "fwapp_trace.c",
        "fwapp_trace.h",
        "fwapp_uac.c",
        "fwapp_uac.h",
        "fwapp_usb.c",
        "fwapp_usb.h",
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
