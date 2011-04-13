*
* Foreign Frame Interface wrapper object
*
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
|
  test
    | ffi |
    ffi <- FFI new: 'simple1'.
    ffi call: 'my_print' returning: #int taking: #(string) as: #('Hello from Smalltalk')
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
    | fid ret |
    ^ (handle >= 0)
        ifFalse: [ nil ]
        ifTrue: [ 
          fid <- <181 handle aString>.
          (fid >= 0)
            ifTrue: 
              [ <182 handle fid aSymbol argTypes arguments> ]
            ifFalse: 
              [ nil ]
        ]
]
