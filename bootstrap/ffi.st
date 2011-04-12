Class FFI Object libraryId name functions
*
Methods MetaFFI 'all'
  new
    | newObj |
    newObj <- super new.
    newObj init.
    ^newObj
|
  new: aString
    | newObj |
    newObj <- super new.
    newObj init.
    newObj open: aString.
    ^newObj
]
*
Methods FFI 'all'
  init
    name <- 'unnamed'.
    functions <- Dictionary new
|
  open: theName
    name <- theName.
    ^self open
|
  name: aString
    name <- aString
|
  name
    ^ name
|
  open
    handle <- self openPrim
|
  openPrim
    ^ <180 name>
|
  call: aString returning: aSymbol taking: argTypes as: arguments
    | fid |
    fid <- <181 handle aString>.
    ^ <182 handle fid aSymbol argTypes arguments>
]
