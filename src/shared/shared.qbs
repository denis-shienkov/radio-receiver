import qbs
import qbs.File
import qbs.FileInfo

Project {
    name: "shared"

    references: [
        "libopencm3/libopencm3.qbs"
    ]
}
