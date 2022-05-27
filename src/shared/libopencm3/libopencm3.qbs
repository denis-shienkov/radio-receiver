import qbs
import qbs.File
import qbs.FileInfo
import "libopencm3.js" as LIBOPENCM3

Project {
    condition: !qbs.targetOS.contains("windows") && !qbs.targetOS.contains("linux")
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
        property stringList compilerFlags: LIBOPENCM3.compilerFlags(project.targetMcu)
        property stringList targetDefines: LIBOPENCM3.targetDefines(project.targetMcu)
        property stringList targetFlags: LIBOPENCM3.targetFlags(project.targetMcu)
        property stringList targetIncludes: [
            generatedPath + "/libopencm3/stm32",
            generatedPath,
            project.libopencm3SourcesPath + "/include",
        ]

        Depends { name: "cpp" }
        cpp.cLanguageVersion: "c99"
        cpp.positionIndependentCode: false
        cpp.includePaths: [product.includeBase]
        cpp.driverFlags: product.targetFlags
        cpp.defines: product.targetDefines
        cpp.commonCompilerFlags: product.compilerFlags

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
            cpp.includePaths: product.targetIncludes
        }

        Group {
            name: "Cm3 vector"
            prefix: project.libopencm3SourcesPath + "/lib/cm3/"
            files: ["vector.c"]
            cpp.includePaths: product.targetIncludes
        }

        Group {
            name: "Specific for " + project.targetMcu
            prefix: project.libopencm3SourcesPath + "/lib/" + LIBOPENCM3.targetSubPath(project.targetMcu) + "/"
            excludeFiles: ["vector_chipset.c"]
            files: ["*.c"]
            cpp.includePaths: product.targetIncludes
        }

        Group {
            name: "Common for " + project.targetMcu
            prefix: project.libopencm3SourcesPath + "/lib/stm32/common/"
            files: LIBOPENCM3.commonSources(project.targetMcu)
        }

        Group {
            name: "Base for " + project.targetMcu
            prefix: project.libopencm3SourcesPath + "/lib/stm32/"
            files: LIBOPENCM3.baseSources(project.targetMcu)
        }

        Group {
            name: "Common Usb"
            prefix: project.libopencm3SourcesPath + "/lib/usb/"
            files: LIBOPENCM3.usbSources(project.targetMcu)
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
            cpp.systemIncludePaths: [exportingProduct.includeBase, exportingProduct.generatedPath]
            cpp.defines: exportingProduct.targetDefines
            cpp.driverFlags: exportingProduct.targetFlags
            cpp.commonCompilerFlags: exportingProduct.compilerFlags
            cpp.cLanguageVersion: "c99"
            cpp.positionIndependentCode: false
        }
    }
}
