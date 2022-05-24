import qbs

CppApplication {
    condition: !qbs.targetOS.contains("windows") && !qbs.targetOS.contains("linux")
    name: "fwapp"

    Depends { name: "libopencm3" }

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

    cpp.driverFlags: [
        "-u_printf_float",
    ]

    files: [
        "fwapp.c",
        "fwapp.h",
//        "fwapp_adc.c",
//        "fwapp_adc.h",
        "fwapp_cbuf.c",
        "fwapp_cbuf.h",
        "fwapp_hid.c",
        "fwapp_hid.h",
        "fwapp_led.c",
        "fwapp_led.h",
//        "fwapp_proto.c",
//        "fwapp_proto.h",
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
