import qbs

Project {
    name: "radio-receiver"

    property string targetMcu: "stm32f1"
    PropertyOptions {
        name: "targetMcu"
        description: "Target MCUs"
        allowedValues: ["stm32f0", "stm32f1"]
    }

    references: [
        "src/src.qbs"
    ]
}
