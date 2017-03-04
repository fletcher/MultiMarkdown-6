/// Trivial example of using libMultiMarkdown within a Swift
/// project.  Needs a lot of work to be functional.

import libMultiMarkdown


token_pool_init()


let text = "This is a *test* string"
let output = d_string_new("")
let format = Int16(FORMAT_HTML.rawValue)

let e = mmd_engine_create_with_string(text, 0)


mmd_engine_parse_string(e)
mmd_export_token_tree(output, e, format)
token_pool_drain()


let str = String(cString: output!.pointee.str!)
print(str)

token_pool_free()
