RawClass MetaFLTK Class MetaObject fltkFFI
Class FLTK Object
RawClass MetaFLTKWindow Class MetaObject
Class FLTKWindow Object handle


Methods MetaFLTK
fltkFFI
  (fltkFFI isNil) ifTrue: [
    fltkFFI <- FFI new: 'modules/fltk/fltk'
  ].
  ^ fltkFFI
]



Methods MetaFLTKWindow
new 
   | newWindow args result |
  newWindow <- super new.
  args <- Array new: 2.
  args at: 1 put: 340; at: 2 put: 180.
  result <- (FLTK fltkFFI) call: 'createWindow' returning: #cObject taking: #(int int) as: args.
  newWindow handle: (result at: 1).
  ^ newWindow 
]


Methods FLTKWindow
handle
  ^ handle
|
handle: aCPointer
  handle <- aCPointer
]

