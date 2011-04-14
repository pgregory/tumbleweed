RawClass MetaIUP Class MetaObject theInstance 
Class IUP Object iup
*
Methods MetaIUP 'all'
  new
    ^ self instance.
|
  instance
    theInstance isNil ifTrue: [ theInstance <- super new. theInstance init ].
    ^ theInstance
|
  test
    | iup result |
    iup <- IUP instance.
    result <- iup IupGetColorRed: 255 Green: 0 Blue: 0.
    stderr print: ('Return: ', (result at: 1), ' Chose - r: ', (result at: 4), ' g: ', (result at: 5), ' b: ', (result at: 6)).
    ^ result
]
*
Methods IUP 'all'
  init
    iup <- FFI new: 'iup'.
    iup call: 'IupOpen' returning: #int taking: #(int int) as: #(0 0).
    ^self
|
  IupGetColorRed: r Green: g Blue: b
    ^ iup call: 'IupGetColor' returning: #int taking: #(int int charOut charOut charOut) as: #(10 10 r g b).
|
  IupMessage: m title: t
    ^ iup call: 'IupMessage' returning: #void taking: #(string string) as: #(t m).
]
