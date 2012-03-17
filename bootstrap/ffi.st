*
* Foreign Frame Interface wrapper object
*
RawClass MetaFFI Class MetaObject libraries
Class FFI Object handle name functions
*
Methods MetaFFI 'all'
  new: aString
    | newObj |
    newObj <- super new.
    newObj init: aString.
    libraries isNil ifTrue: [
      libraries <- Dictionary new
    ].
    libraries at: aString put: newObj.
    ^newObj
|
  test
    | ffi |
    ffi <- FFI new: 'simple1'.
    ffi call: 'my_print' returning: #int taking: #(string) as: #('Hello from Smalltalk')
]
Methods MetaFFI 'misc'
  libraries
    ^ libraries
|
  prepareForSnapshot
    libraries isNil ifFalse: [
    libraries binaryDo: [ :x :ffi |
      ffi close ]
    ]
|
  returnFromSnapshot
    libraries isNil ifFalse: [
    libraries binaryDo: [ :x :ffi |
      ffi open ]
    ]      
]
*
Methods FFI 'all'
  init: aString
    self open: aString.
    functions <- Dictionary new.
|
  open: theName
    name <- theName.
    ^ self open
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
  close
    | result |
    result <- <184 handle>.
    handle <- nil.
    ^ result
|
  handle
    ^ handle
|
  add: aString
    | fhandle |
    fhandle <- <181 handle aString>.
    fhandle isNil ifFalse: [ function at: aString put: fhandle ].
    ^ self
|
  call: aString returning: aSymbol taking: argTypes as: arguments
    | fid ret |
    ^ handle isNil
        ifTrue: [ nil ]
        ifFalse: [ 
          (functions includesKey: aString)
            ifTrue: [ fid <- functions at: aString ]
            ifFalse: [ fid <- <181 handle aString> ].
          fid isNil
            ifFalse: 
              [ <182 fid aSymbol argTypes arguments> ]
            ifTrue: 
              [ nil ]
        ]
]
