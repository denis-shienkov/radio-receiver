import qbs

Project {
    name: "apps"
    references: [
        "fwapp/fwapp.qbs",
        "guiapp/guiapp.qbs",
    ]
}
