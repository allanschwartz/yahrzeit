import AppKit
import Foundation
import PDFKit

let args = CommandLine.arguments
guard args.count == 5 else {
    fputs("usage: render-pdf-pages input.pdf firstPage lastPage outputDir\n", stderr)
    exit(2)
}

let input = URL(fileURLWithPath: args[1])
let first = Int(args[2])! - 1
let last = Int(args[3])! - 1
let outputDir = URL(fileURLWithPath: args[4], isDirectory: true)
try FileManager.default.createDirectory(at: outputDir, withIntermediateDirectories: true)

guard let document = PDFDocument(url: input) else {
    fputs("unable to open PDF\n", stderr)
    exit(1)
}

print("pages: \(document.pageCount)")
for index in first...last {
    guard let page = document.page(at: index) else { continue }
    let box = page.bounds(for: .mediaBox)
    let scale: CGFloat = 2.0
    let width = Int(box.width * scale)
    let height = Int(box.height * scale)
    guard let bitmap = NSBitmapImageRep(
        bitmapDataPlanes: nil,
        pixelsWide: width,
        pixelsHigh: height,
        bitsPerSample: 8,
        samplesPerPixel: 4,
        hasAlpha: true,
        isPlanar: false,
        colorSpaceName: .deviceRGB,
        bytesPerRow: 0,
        bitsPerPixel: 0
    ) else { continue }
    NSGraphicsContext.saveGraphicsState()
    let context = NSGraphicsContext(bitmapImageRep: bitmap)!
    NSGraphicsContext.current = context
    context.cgContext.setFillColor(NSColor.white.cgColor)
    context.cgContext.fill(CGRect(x: 0, y: 0, width: width, height: height))
    context.cgContext.scaleBy(x: scale, y: scale)
    page.draw(with: .mediaBox, to: context.cgContext)
    NSGraphicsContext.restoreGraphicsState()
    let data = bitmap.representation(using: .png, properties: [:])!
    let out = outputDir.appendingPathComponent("page-\(index + 1).png")
    try data.write(to: out)
    print(out.path)
}
