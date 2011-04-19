RawClass MetaIUP Class MetaObject theInstance 
Class IUP Object iup
Class Widget Object handle
Class   WidgetContainer Widget children
Class     VBox WidgetContainer
Class     HBox WidgetContainer
Class     Dialog WidgetContainer
Class   ButtonControl Widget
Class   LabelControl Widget
Class   ListControl Widget
Class   TextControl Widget
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
    | btn vbox dialog done e |
    iup <- IUP instance.
    btn <- ButtonControl new: 'Test'.
    stderr print: (btn handle asString).
    vbox <- VBox new.
    vbox addChild: btn.
    btn setActionCallback: [ :x | stderr print: ('Hello: ', (x asString)). -3 ].
    dialog <- Dialog new: vbox.
    dialog show.
    done <- false.
    [ done ] whileFalse: [ 
      e <- iup IupLoopStep.
      [ e = -3 ] whileFalse: [ e <- iup IupLoopStep ].
      stderr print: (e asString).
      done <- true
    ]
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
  IupVbox 
    ^ iup call: 'IupVbox' returning: #cObject taking: #(int int) as: #(0 0)
|
  IupHbox 
    ^ iup call: 'IupHbox' returning: #cObject taking: #(int int) as: #(0 0)
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
  IupHide: dialog
    | args |
    args <- Array new: 1.
    args at: 1 put: dialog.
    ^ iup call: #IupHide returning: #int taking: #(cObject) as: args
|
  IupMainLoop
    ^ iup call: #IupMainLoop returning: #int taking: #() as: #()
|
  IupLoopStep
    | result |
    result <- iup call: #IupLoopStep returning: #int taking: #() as: #().
    ^ result at: 1
|
  IupStoreAttribute: name on: handle as: value
    | args |
    args <- Array new: 3.
    args at: 1 put: handle.
    args at: 2 put: name.
    args at: 3 put: value.
    ^ iup call: #IupStoreAttribute returning: #void taking: #(cObject string string) as: args
|
  IupSetNullAttribute: name on: handle
    | args |
    args <- Array new: 3.
    args at: 1 put: handle.
    args at: 2 put: name.
    args at: 3 put: CPointer new.
    ^ iup call: #IupSetAttribute returning: #void taking: #(cObject string cObject) as: args
|
  IupList
    ^ iup call: #IupList returning: #cObject taking: #(string) as: #('DefaultListAction')
|
  IupText
    ^ iup call: #IupText returning: #cObject taking: #(string) as: #('DefaultListAction')
|
  IupSetCallbackOn: handle for: action as: callback
    | args |
    args <- Array new: 3.
    args at: 1 put: handle.
    args at: 2 put: action.
    args at: 3 put: callback.
    ^ iup call: #IupSetCallback returning: #cObject taking: #(cObject string cObject) as: args
]
*
Methods MetaWidget 'all'
  new
    | newObj |
    newObj <- super new.
    newObj init.
    ^newObj
]
*
Methods Widget 'all'
  init
    handle <- 0.
    ^ self
|
  handle
    ^ handle
| 
  handle: h
    handle <- h
|
  at: name put: value
  IUP instance IupStoreAttribute: name on: (self handle) as: value
|
  setCallbackFor: action as: aBlock returning: type taking: args
  | cb |
  cb <- <183 type args aBlock>.
  ^ IUP instance IupSetCallbackOn: (self handle) for: (action asString) as: cb
]
*
Methods WidgetContainer 'all'
  init
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
Methods MetaButtonControl 'all'
  new: aString
    | newButton |
    newButton <- super new.
    newButton init: aString.
    ^ newButton
]
*
Methods ButtonControl 'all'
  init: aString
    | iup result |
    iup <- IUP instance.
    result <- iup IupButton: aString.
    super handle: (result at: 1).
    ^ self
|
  init
    ^ self init: 'UNNAMED'
|
  setActionCallback: aBlock
    ^ super setCallbackFor: 'ACTION' as: aBlock returning: #int taking: #(cObject)
]
*
Methods VBox 'all'
  init
    | iup result |
    super init.
    iup <- IUP instance.
    result <- iup IupVbox.
    super handle: (result at: 1).
    ^ self
]
*
Methods HBox 'all'
  init
    | iup result |
    super init.
    iup <- IUP instance.
    result <- iup IupHbox.
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
    | iup result |
    iup <- IUP instance.
    result <- iup IupDialog: (child handle).
    super handle: (result at: 1).
    ^ self
|
  show
    IUP instance IupShow: handle.
|
  hide
    IUP instance IupHide: handle.
]
*
Methods ListControl 'all'
  init
    | iup result |
    iup <- IUP instance.
    result <- iup IupList.
    super handle: (result at: 1).
    ^self
|
  setActionCallback: aBlock
    ^ super setCallbackFor: 'ACTION' as: aBlock returning: #int taking: #(cObject string int int)
]
*
Methods TextControl 'all'
  init
    | iup result |
    iup <- IUP instance.
    result <- iup IupText.
    super handle: (result at: 1).
    ^self
|
  insert: aString
    ^ super at: 'INSERT' put: (aString asString)
]
