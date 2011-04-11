Class FFI Object libraryId name functions
*
Methods FFI 'all'
  open: theName
    name <- theName.
    ^self open
|
  name
    ^ name
|
  open
    handle <- self openPrim
|
  openPrim
    ^ <180 name>
]
