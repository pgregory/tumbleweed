RawClass MetaIUP Class MetaObject theInstance 
Class IUP Object iup
Class Widget Object handle
Class   WidgetContainer Widget children
Class     VBox WidgetContainer
Class     HBox WidgetContainer
Class     Dialog WidgetContainer
Class   Button Widget
Class   Label Widget
*
Methods MetaIUP 'all'
  new
    ^ self instance.
|
  instance
    theInstance isNil ifTrue: [ stderr print: 'Loading IUP'].
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
    ^ iup call: 'IupGetColor' returning: #int taking: #(int int charOut charOut charOut) as: #(10 10 r g b)
|
  IupMessage: m title: t
    | args |
    args <- Array new: 2.
    args at: 1 put: t.
    args at: 2 put: m.
    ^ iup call: 'IupMessage' returning: #void taking: #(string string) as: args 
|
  IupButton: title 
    | args |
    args <- Array new: 2.
    args at: 1 put: title.
    args at: 2 put: 'DefaultButtonAction'.
    ^ iup call: 'IupButton' returning: #cObject taking: #(string string) as: args 
|
  IupVBox 
    ^ iup call: 'IupVbox' returning: #cObject taking: #(int int) as: #(0 0)
|
  IupDialog: child 
    | args |
    args <- Array new: 1.
    args at: 1 put: child.
    ^ iup call: 'IupDialog' returning: #cObject taking: #(cObject) as: args
|
  IupAppend: child to: parent 
    | args |
    args <- Array new: 2.
    args at: 1 put: parent.
    args at: 2 put: child.
    ^ iup call: 'IupAppend' returning: #cObject taking: #(cObject cObject) as: args 
|
  IupShow: dialog
    | args |
    args <- Array new: 1.
    args at: 1 put: dialog.
    ^ iup call: #IupShow returning: #int taking: #(cObject) as: args
|
  IupMainLoop
    ^ iup call: #IupMainLoop returning: #int taking: #() as: #()
]
*
Methods Widget 'all'
  new
    handle <- 0.
    ^ self
|
  handle
    ^ handle
| 
  handle: h
    handle <- h
]
*
Methods WidgetContainer 'all'
  new
    super new.
    children <- List new.
    ^ self 
|
  children
    ^ children
|
  addChild: child
    children add: child.
    IUP instance IupAppend: (child handle) to: (self handle).
    ^ self
]
*
Methods MetaButton 'all'
  new: aString
  | newButton |
  newButton <- super new.
  newButton init: aString.
  ^ newButton
]
*
Methods Button 'all'
  init: aString
    | iup result |
    iup <- IUP instance.
    result <- iup IupButton: aString.
    super handle: (result at: 1).
    ^ self
]
*
*
Methods MetaVBox 'all'
  new
  | newVBox |
  newVBox <- super new.
  newVBox init.
  ^ newVBox
]
Methods VBox 'all'
  init
    | iup result |
    iup <- IUP instance.
    result <- iup IupVBox.
    super handle: (result at: 1).
    ^ self
]
*
Methods MetaDialog 'all'
  new: child
    | newDialog |
    newDialog <- super new.
    newDialog init: child.
    ^ newDialog
]
*
Methods Dialog 'all'
  init: child
    | result |
    iup <- IUP instance.
    result <- iup IupDialog: (child handle).
    super handle: (result at: 1).
    ^ self
|
  show
    IUP instance IupShow: handle.
]
