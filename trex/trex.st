Class TRex Object txFFI txHandle

Methods TRex 'misc'
  loadFFI
    (txFFI isNil) ifTrue: [ txFFI <- FFI new: 'modules/trex/trex' ].
]
*
Methods MetaTRex 'creation'
  new: aString
  | newobj |
  newobj <- super new.
  newobj init: aString.
  ^ newobj
]
*
Methods TRex 'accessors'
  handle
  ^ txHandle
]
*
Methods TRex 'creation'
  new
    (txFFI isNil) ifTrue: [ txFFI <- FFI new: 'modules/trex/trex' ]
|
  init: aString
  | args error |
  args <- Array new: 2.
  error <- ByteArray new: 255.
  args at: 1 put: aString; at: 2 put: CPointer new.
  txHandle <- (txFFI call: 'trex_compile' returning: #cObject taking: #(string cObject) as: args) at: 1
]  
*
Methods TRex 'match'
  match: aString
  | args result |
  args <- Array new: 2.
  args at: 1 put: txHandle; at: 2 put: aString.
  result <- (txFFI call: 'trex_match' returning: #int taking: #(cObject string) as: args) at: 1.
  ^ ((result = 0) not)
]
