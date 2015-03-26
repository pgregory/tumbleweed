RawClass MetaFLTK Class MetaObject fltkFFI
Class FLTK Object
Class FLTKWidget Object handle
Class FLTKWindow FLTKWidget
Class FLTKButton FLTKWidget
Class FLTKInput FLTKWidget


Methods MetaFLTK
fltkFFI
  (fltkFFI isNil) ifTrue: [
    fltkFFI <- FFI new: 'modules/fltk/fltk'
  ].
  ^ fltkFFI
|
wait
  | result |
  result <- (self fltkFFI) call: 'Fl_wait' returning: #int taking: #(double) as: ((Array new: 1) at: 1 put: 0.0).
  ^ result at: 1
]



Methods MetaFLTKWindow
new 
   | newWindow result |
  newWindow <- super new.
  result <- (FLTK fltkFFI) call: 'createWindow' returning: #cObject taking: #(int int) as: 
      ((Array new: 2) at: 1 put: 340; at: 2 put: 180).
  newWindow handle: (result at: 1).
  ^ newWindow 
]


Methods FLTKWidget
handle
  ^ handle
|
handle: aCPointer
  handle <- aCPointer
|
setCallback: aBlock
  | cb args |
  args <- Array new: 2.
  cb <- <183 #void #(cObject) aBlock >.
  args at: 1 put: handle; at: 2 put: cb. 
  (FLTK fltkFFI) call: 'Fl_setCallback' returning: #void taking: #(cObject cObject) as: args
]

Methods FLTKWindow
show
  (FLTK fltkFFI) call: 'show' returning: #void taking: #(cObject) as: ((Array new: 1) at: 1 put: handle)
]


Methods MetaFLTKButton
newAtX: x Y: y Width: w Height: h Label: aString
   | newButton args result |
  newButton <- super new.
  result <- (FLTK fltkFFI) call: 'createButton' returning: #cObject taking: #(int int int int string) as: 
    ((Array new: 5) at: 1 put: x; at: 2 put: y; at: 3 put: w; at: 4 put: h; at: 5 put: aString).
  newButton handle: (result at: 1).
  ^ newButton 
]

Methods MetaFLTKInput
newAtX: x Y: y Width: w Height: h Label: aString
   | newInput args result |
  newInput <- super new.
  result <- (FLTK fltkFFI) call: 'Fl_createInput' returning: #cObject taking: #(int int int int string) as: 
    ((Array new: 5) at: 1 put: x; at: 2 put: y; at: 3 put: w; at: 4 put: h; at: 5 put: aString).
  newInput handle: (result at: 1).
  ^ newInput 
]
