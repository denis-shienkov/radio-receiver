import qbs
import qbs.File
import qbs.FileInfo
import "libopencm3.js" as LIBOPENCM3

Project {
    name: "libopencm3-project"

    property path libopencm3SourcesPath: LIBOPENCM3.sourcesPath(project)
    property bool libopencm3SubModuleExists: LIBOPENCM3.submoduleExists(project)

    StaticLibrary {
        name: "libopencm3"
        condition: libopencm3SubModuleExists

        property bool foo: {
            console.info(cpp.includePaths)
        }

        property path generatedPath: buildDirectory + "/generated"
        property path includeBase: project.libopencm3SourcesPath + "/include"
        property string defineBase: LIBOPENCM3.targetDefine(project.targetMcu)

        Depends { name: "cpp" }
        cpp.cLanguageVersion: "c99"
        cpp.positionIndependentCode: false
        cpp.includePaths: [product.includeBase]

        cpp.commonCompilerFlags: [
            "-Wextra",
            "-Wimplicit-function-declaration",
            "-Wmissing-prototypes",
            "-Wredundant-decls",
            "-Wshadow",
            "-Wstrict-prototypes",
            "-Wundef",
        ]

        Properties {
            condition: project.targetMcu === "stm32f1"
            cpp.driverFlags: [
                "-fdata-sections",
                "-ffunction-sections",
                "-fno-common",
                "-mcpu=cortex-m3",
                "-mthumb",
            ]
            cpp.defines: [defineBase]
        }

        Group {
            name: "JSON descriptors"
            files: [LIBOPENCM3.findJsonFile(project)]
            fileTags: ["json_descr"]
        }

        Group {
            name: "Cm3"
            prefix: project.libopencm3SourcesPath + "/lib/cm3/"
            files: [
                "assert.c",
                "dwt.c",
                "scb.c",
                "sync.c",
                "systick.c",
            ]
        }

        Group {
            name: "Cm3 nvic"
            prefix: project.libopencm3SourcesPath + "/lib/cm3/"
            files: ["nvic.c"]
            cpp.includePaths: [
                product.generatedPath,
                project.libopencm3SourcesPath + "/include",
            ]
        }

        Group {
            name: "Cm3 vector"
            prefix: project.libopencm3SourcesPath + "/lib/cm3/"
            files: ["vector.c"]
            cpp.includePaths: [
                product.generatedPath + "/libopencm3/stm32",
                product.generatedPath,
                project.libopencm3SourcesPath + "/include",
            ]
        }

        Group {
            condition: project.targetMcu === "stm32f1"
            name: "Specific for " + project.targetMcu
            prefix: project.libopencm3SourcesPath + "/lib/stm32/f1/"
            files: ["*.c"]
        }

        Group {
            condition: project.targetMcu === "stm32f1"
            name: "Common for " + project.targetMcu
            prefix: project.libopencm3SourcesPath + "/lib/stm32/common/"
            files: [
                "adc_common_v1.c",
                "crc_common_all.c",
                "dac_common_all.c",
                "dac_common_v1.c",
                "desig_common_all.c",
                "desig_common_v1.c",
                "dma_common_l1f013.c",
                "exti_common_all.c",
                "flash_common_all.c",
                "flash_common_f.c",
                "flash_common_f01.c",
                "gpio_common_all.c",
                "i2c_common_v1.c",
                "iwdg_common_all.c",
                "pwr_common_v1.c",
                "rcc_common_all.c",
                "spi_common_all.c",
                "spi_common_v1.c",
                "st_usbfs_core.c",
                "timer_common_all.c",
                "usart_common_all.c",
                "usart_common_f124.c",
            ]
        }

        Group {
            condition: project.targetMcu === "stm32f1"
            name: "Base for " + project.targetMcu
            prefix: project.libopencm3SourcesPath + "/lib/stm32/"
            files: [
                "can.c",
                "st_usbfs_v1.c",
            ]
        }

        Group {
            name: "Common Usb"
            prefix: project.libopencm3SourcesPath + "/lib/usb/"
            files: [
                "usb.c",
                "usb_audio.c",
                "usb_cdc.c",
                "usb_control.c",
                "usb_dwc_common.c",
                "usb_f107.c",
                "usb_hid.c",
                "usb_midi.c",
                "usb_msc.c",
                "usb_standard.c",
            ]
        }

        Rule {
            id: nvicGenerator
            inputs: ["json_descr"]
            outputFileTags: ["nvic_hpp", "cmsis_hpp", "nvic_cpp", "hpp"]
            outputArtifacts: LIBOPENCM3.nvicGeneratorOutputArtifacts(project, product)
            prepare: LIBOPENCM3.prepareNvicGenerator.apply(LIBOPENCM3, arguments)
        }

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: [product.includeBase, product.generatedPath]
            cpp.defines: [product.defineBase]
        }
    }
}
