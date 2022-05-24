var File = require("qbs.File");
var FileInfo = require("qbs.FileInfo");
var TextFile = require("qbs.TextFile");
var Utilities = require("qbs.Utilities");

function sourcesPath(project) {
    return FileInfo.joinPaths(project.sourceDirectory, "src/shared/libopencm3/content")
}

function submoduleExists(project) {
    var path = sourcesPath(project);
    return File.exists(FileInfo.joinPaths(path, "Makefile"))
}

function targetSubPath(targetMcu) {
    if (targetMcu === "stm32f1")
        return "stm32/f1";
    else if (targetMcu === "stm32f4")
        return "stm32/f4";
}

function targetDefines(targetMcu) {
    if (targetMcu === "stm32f1")
        return ["STM32F1"];
    else if (targetMcu === "stm32f4")
        return ["STM32F4"];
}

function targetFlags(targetMcu) {
    var flags = ["-fdata-sections", "-ffunction-sections", "-fno-common", "-mthumb"];
    if (targetMcu === "stm32f1")
        return flags.concat("-mcpu=cortex-m3", "-mfix-cortex-m3-ldrd", "-msoft-float");
    else if (targetMcu === "stm32f4")
        return flags.concat("-mcpu=cortex-m4", "-mfloat-abi=hard", "-mfpu=fpv4-sp-d16");
}

function compilerFlags(targetMcu) {
    return ["-Wextra", "-Wimplicit-function-declaration", "-Wmissing-prototypes",
            "-Wredundant-decls", "-Wshadow", "-Wstrict-prototypes", "-Wundef"]
}

function libopencm3SubPath(targetMcu) {
    return FileInfo.joinPaths("libopencm3", targetSubPath(targetMcu));
}

function libopencmsisSubPath(targetMcu) {
    return FileInfo.joinPaths("libopencmsis", targetSubPath(targetMcu));
}

function commonSources(targetMcu) {
    var sources = ["crc_common_all.c", "dac_common_all.c", "desig_common_all.c", "desig_common_v1.c",
                   "exti_common_all.c", "flash_common_all.c", "gpio_common_all.c", "iwdg_common_all.c",
                   "rcc_common_all.c", "spi_common_all.c",  "timer_common_all.c", "usart_common_all.c"];
    if (targetMcu === "stm32f1")
        return sources.concat("adc_common_v1.c", "dac_common_v1.c", "dma_common_l1f013.c",
                              "flash_common_f.c", "flash_common_f01.c", "i2c_common_v1.c",
                              "pwr_common_v1.c", "spi_common_v1.c", "st_usbfs_core.c",
                              "usart_common_f124.c");
    else if (targetMcu === "stm32f4")
        return sources.concat("adc_common_f47.c", "adc_common_v1.c", "adc_common_v1_multi.c",
                              "crypto_common_f24.c", "dac_common_v1.c", "dcmi_common_f47.c",
                              "dma2d_common_f47.c", "dma_common_f24.c", "dsi_common_f47.c",
                              "flash_common_f.c", "flash_common_f24.c", "flash_common_idcache.c",
                              "fmc_common_f47.c", "gpio_common_f0234.c", "hash_common_f24.c",
                              "i2c_common_v1.c", "lptimer_common_all.c", "ltdc_common_f47.c",
                              "pwr_common_v1.c", "quadspi_common_v1.c", "rng_common_v1.c",
                              "rtc_common_l1f024.c", "spi_common_v1.c", "spi_common_v1_frf.c",
                              "timer_common_f0234.c", "timer_common_f24.c", "usart_common_f124.c");
}

function baseSources(targetMcu) {
    var sources = ["can.c"];
    if (targetMcu === "stm32f1")
        return sources.concat("st_usbfs_v1.c");
    else if (targetMcu === "stm32f4")
        return sources;
}

function usbSources(targetMcu) {
    var sources = ["usb.c", "usb_audio.c", "usb_cdc.c", "usb_control.c", "usb_dwc_common.c",
                   "usb_hid.c", "usb_midi.c", "usb_msc.c", "usb_standard.c"];
    if (targetMcu === "stm32f1")
        return sources.concat("usb_f107.c");
    else if (targetMcu === "stm32f4")
        return sources.concat("usb_f107.c", "usb_f207.c");
}

function findJsonFile(project) {
    var cm3SubPath = libopencm3SubPath(project.targetMcu);
    return FileInfo.joinPaths(project.libopencm3SourcesPath, "include", cm3SubPath, "irq.json");
}

function nvicGeneratorOutputArtifacts(project, product) {
    console.error("GP: " + product.generatedPath)
    var cm3SubPath = libopencm3SubPath(project.targetMcu);
    var cmsisSubPath = libopencmsisSubPath(project.targetMcu);
    var tgtSubPath = targetSubPath(project.targetMcu);
    return [{
                fileTags: ["hpp", "nvic_hpp"],
                filePath: FileInfo.joinPaths(product.generatedPath, cm3SubPath, "nvic.h"),
            },
            {
                fileTags: ["hpp", "cmsis_hpp"],
                filePath: FileInfo.joinPaths(product.generatedPath, cmsisSubPath, "irqhandlers.h"),
            },
            {
                fileTags: ["hpp", "nvic_cpp"],
                filePath: FileInfo.joinPaths(product.generatedPath, "libopencm3/" + tgtSubPath +"/vector_nvic.c")
            }];
}

function generateNvicHeader(project, product, inputs, outputs, input, output, data) {
    var cmd = new JavaScriptCommand();
    cmd.sourcesRoot = project.libopencm3SourcesPath;
    cmd.data = data;
    cmd.inputFile = input.filePath;
    cmd.outputFile = outputs.nvic_hpp[0].filePath;
    cmd.description = "generating " + outputs.nvic_hpp[0].fileName + " from "
            + FileInfo.relativePath(project.libopencm3SourcesPath, input.filePath);
    cmd.sourceCode = function() {
        var irqs = data["irqs"] || [];

        var tf = new TextFile(outputFile, TextFile.WriteOnly);
        tf.writeLine("// This file is part of the libopencm3 project.");
        tf.writeLine("// It was generated by the Qbs from "
                     + FileInfo.relativePath(sourcesRoot, inputFile));
        tf.writeLine("");
        tf.writeLine("#ifndef " + data["includeguard"]);
        tf.writeLine("#define " + data["includeguard"]);
        tf.writeLine("");
        tf.writeLine("#include <libopencm3/cm3/nvic.h>");
        tf.writeLine("");

        for (var i = 0; i < irqs.length; ++i)
            tf.writeLine("#define NVIC_" + irqs[i].toUpperCase() + "_IRQ " + i);

        tf.writeLine("");
        tf.writeLine("#define NVIC_IRQ_COUNT " + irqs.length);
        tf.writeLine("");
        tf.writeLine("BEGIN_DECLS");
        tf.writeLine("");

        for (var j = 0; j < irqs.length; ++j)
            tf.writeLine("void " + irqs[j].toLowerCase() + "_isr(void);");

        tf.writeLine("");
        tf.writeLine("END_DECLS");
        tf.writeLine("");
        tf.writeLine("#endif // " + data["includeguard"]);
    };
    return cmd;
}

function generateCmsisHeader(project, product, inputs, outputs, input, output, data) {
    var cmd = new JavaScriptCommand();
    cmd.sourcesRoot = project.libopencm3SourcesPath;
    cmd.data = data;
    cmd.inputFile = input.filePath;
    cmd.outputFile = outputs.cmsis_hpp[0].filePath;
    cmd.description = "generating " + outputs.cmsis_hpp[0].fileName + " from "
            + FileInfo.relativePath(project.libopencm3SourcesPath, input.filePath);
    cmd.sourceCode = function() {
        var irqs = data["irqs"] || [];

        var tf = new TextFile(outputFile, TextFile.WriteOnly);
        tf.writeLine("// This file is part of the libopencm3 project.");
        tf.writeLine("// It was generated by the Qbs from "
                     + FileInfo.relativePath(sourcesRoot, inputFile));
        tf.writeLine("");
        tf.writeLine("// These definitions bend every interrupt handler that is defined CMSIS style");
        tf.writeLine("// to the weak symbol exported by libopencm3.")
        tf.writeLine("");

        for (var i = 0; i < irqs.length; ++i)
            tf.writeLine("#define " + irqs[i].toUpperCase() + "_IRQHandler " + irqs[i].toLowerCase() + "_isr");
    };
    return cmd;
}

function generateNvicVector(project, product, inputs, outputs, input, output, data) {
    var cmd = new JavaScriptCommand();
    cmd.sourcesRoot = project.libopencm3SourcesPath;
    cmd.data = data;
    cmd.inputFile = input.filePath;
    cmd.outputFile = outputs.nvic_cpp[0].filePath;
    cmd.description = "generating " + outputs.nvic_cpp[0].fileName + " from "
            + FileInfo.relativePath(project.libopencm3SourcesPath, input.filePath);
    cmd.sourceCode = function() {
        var irqs = data["irqs"] || [];

        var tf = new TextFile(outputFile, TextFile.WriteOnly);
        tf.writeLine("// This file is part of the libopencm3 project.");
        tf.writeLine("// It was generated by the Qbs from "
                     + FileInfo.relativePath(sourcesRoot, inputFile));
        tf.writeLine("");
        tf.writeLine("// This part needs to get included in the compilation unit where");
        tf.writeLine("// blocking_handler gets defined due to the way #pragma works.")
        tf.writeLine("");

        for (var i = 0; i < irqs.length; ++i)
            tf.writeLine("void " + irqs[i].toLowerCase() + "_isr(void) __attribute__((weak, alias(\"blocking_handler\")));");

        tf.writeLine("");
        tf.writeLine("// Initialization template for the interrupt vector table. This definition is");
        tf.writeLine("// used by the startup code generator (vector.c) to set the initial values for");
        tf.writeLine("// the interrupt handling routines to the chip family specific _isr weak");
        tf.writeLine("// symbols.");
        tf.writeLine("");

        var defines = irqs.map(function(irq) {
            return "[NVIC_" + irq.toUpperCase() + "_IRQ] = " + irq.toLowerCase() + "_isr";
        });

        tf.writeLine("#define IRQ_HANDLERS \\\n    " + defines.join(", \\\n    "));
    };
    return cmd;
}

function prepareNvicGenerator(project, product, inputs, outputs, input, output) {
    var cmds = [];
    var tf = new TextFile(input.filePath, TextFile.ReadOnly);
    var content = tf.readAll();
    var data = JSON.parse(content);

    cmds.push(generateNvicHeader(project, product, inputs, outputs, input, output, data));
    cmds.push(generateCmsisHeader(project, product, inputs, outputs, input, output, data));
    cmds.push(generateNvicVector(project, product, inputs, outputs, input, output, data));
    return cmds;
}
