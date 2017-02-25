import PackageDescription

let package = Package(
    name: "mmd-swift",
    targets: [
    	Target(name: "mmd-swift", dependencies: ["libMultiMarkdown"])
    ],
    exclude: ["tests", "Tests", "Sources/multimarkdown"]
)
