import qbs

Project {
    name: "radio-receiver"

    property string targetMcu: "stm32f4"
    PropertyOptions {
        name: "targetMcu"
        description: "Target MCUs"
        allowedValues: ["stm32f1", "stm32f4"]
    }

    references: [
        "src/src.qbs"
    ]
}
