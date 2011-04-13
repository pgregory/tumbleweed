Class IUP FFI
*
Methods MetaIUP 'all'
  new
    | newObj |
    newObj <- super new: 'iup'.
    newObj call: 'IupOpen' returning: #int taking: #(int int) as: #(0 0).
    ^newObj
|
  test
    | iup result |
    iup <- IUP new.
    result <- iup call: 'IupGetColor' returning: #int taking: #(int int charOut charOut charOut) as: #(10 10 r g b).
    stderr print: ('Return: ', (result at: 1), ' Chose - r: ', (result at: 4), ' g: ', (result at: 5), ' b: ', (result at: 6)).
    ^ result
]
