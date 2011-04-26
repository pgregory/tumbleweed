RawClass MetaIUP Class MetaObject theInstance 
Class IUP Object iup
Class Widget Object handle
Class   WidgetContainer Widget children
Class     VBox WidgetContainer
Class     HBox WidgetContainer
Class     Split WidgetContainer
Class     Dialog WidgetContainer
Class     Menu WidgetContainer
Class   ButtonControl Widget
Class   LabelControl Widget
Class   ListControl Widget
Class   TextControl Widget
Class   ItemControl Widget
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
    | args |
    args <- Array new: 2.
    args at: 1 put: (CPointer new).
    args at: 2 put: (CPointer new).
    ^ iup call: 'IupVbox' returning: #cObject taking: #(cObject cObject) as: args
|
  IupHbox 
    | args |
    args <- Array new: 2.
    args at: 1 put: (CPointer new).
    args at: 2 put: (CPointer new).
    ^ iup call: 'IupHbox' returning: #cObject taking: #(cObject cObject) as: args
|
  IupSplitWith: child1 and: child2 
    | args |
    args <- Array new: 2.
    args at: 1 put: child1.
    args at: 2 put: child2.
    ^ iup call: 'IupSplit' returning: #cObject taking: #(cObject cObject) as: args
|
  IupDialog: child 
    | args |
    args <- Array new: 1.
    args at: 1 put: child.
    ^ iup call: 'IupDialog' returning: #cObject taking: #(cObject) as: args
|
  IupPopup: child at: x and: y
    | args |
    args <- Array new: 3.
    args at: 1 put: child.
    args at: 2 put: x.
    args at: 3 put: y.
    ^ iup call: 'IupPopup' returning: #int taking: #(cObject int int) as: args
|
  IupAppend: child to: parent 
    | args |
    args <- Array new: 2.
    args at: 1 put: parent.
    args at: 2 put: child.
    ^ iup call: 'IupAppend' returning: #cObject taking: #(cObject cObject) as: args 
|
  IupInsert: child after: ref on: parent 
    | args |
    args <- Array new: 3.
    args at: 1 put: parent.
    args at: 2 put: ref.
    args at: 3 put: child.
    ^ iup call: 'IupInsert' returning: #cObject taking: #(cObject cObject cObect) as: args 
|
  IupGetChildAt: pos on: parent
    | args |
    args <- Array new: 2.
    args at: 1 put: parent.
    args at: 2 put: pos.
    ^ iup call: 'IupGetChild' returning: #cObject taking: #(cObject int) as: args
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
  IupGetAttribute: name on: handle 
    | args |
    args <- Array new: 2.
    args at: 1 put: handle.
    args at: 2 put: name.
    ^ iup call: #IupGetAttribute returning: #string taking: #(cObject string) as: args
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
|
  IupMenu
    | args |
    args <- Array new: 2.
    args at: 1 put: (CPointer new).
    args at: 2 put: (CPoitner new).
    ^ iup call: 'IupMenu' returning: #cObject taking: #(cObject cObject) as: args
|
  IupItem: title
    | args |
    args <- Array new: 2.
    args at: 1 put: title.
    args at: 2 put: 'DefaultMenuAction'.
    ^ iup call: 'IupItem' returning: #cObject taking: #(string string) as: args 
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
  at: name
    | result |
    result <- IUP instance IupGetAttribute: name on: (self handle).
    ^ result at: 1
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
Methods MetaSplit 'creation'
  with: child1 and: child2
    | newControl |
    newControl <- super new.
    newControl left: child1 right: child2.
    ^ newControl
]
*
Methods Split 'all'
  init
    | iup result |
    super init.
    iup <- IUP instance.
    result <- iup IupSplitWith: (CPointer new) and: (CPointer new).
    super handle: (result at: 1).
    ^ self
|
  left: child1 right: child2
    | gripper |
    iup <- IUP instance.
    iup IupAppend: (child1 handle) to: (self handle).
    iup IupAppend: (child2 handle) to: (self handle).
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
  count
    | result |
    result <- IUP instance IupGetAttribute: 'COUNT' on: handle.
    ^ result at: 1
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
*
Methods Menu 'all'
  init
    | iup result |
    iup <- IUP instance.
    result <- iup IupMenu.
    super handle: (result at: 1).
    ^self
]
*
Methods MetaItemControl 'all'
  new: aString
    | newItem |
    newItem <- super new.
    newItem init: aString.
    ^ newItem
]
*
Methods ItemControl 'all'
  init: aString
    | iup result |
    iup <- IUP instance.
    result <- iup IupItem: aString.
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
