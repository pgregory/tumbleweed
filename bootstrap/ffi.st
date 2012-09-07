*
* Foreign Frame Interface wrapper object
*
RawClass MetaFFI Class MetaObject libraries
Class FFI Object handle name functions
Class ExternalData Object members
Class EDTeset ExternalData
*
Methods MetaExternalData 'creation'
  new
    | newObj |
    newObj <- super new.
    newObj init.
    ^ newObj
]
*
Methods ExternalData 'all'
  init
    members <- Array new: (self fields size)
|
  members
    ^ members
|
  fields
    ^ #((string string)
        (number int))
|
  member: index
    ^ members at: index
|
  setMember: index to: anObject
    members at: index put: anObject
]
*
Methods EDTest 'all'
  fields
    ^ #((string string)
        (number int))
]
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
    | ffi args result etype |
    etype <- ExternalData new.
    etype members at: 1 put: 'Hello from FFI test'; at: 2 put: 69.
    args <- Array new: 3.
    args at: 1 put: 'Hello from Smalltalk'; at: 2 put: 0; at: 3 put: etype.
    ffi <- FFI new: 'simple1'.
    result <- ffi call: 'my_print' returning: #int taking: #(string intOut externalOut) as: args.
    (result at: 3) print.
    (result at: 4) members print
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
