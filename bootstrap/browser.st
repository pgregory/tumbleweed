Class Panel Object browser widget
Class   NavigationPanel Panel panes
Class   ButtonPanel Panel buttons
Class   DefinitionPanel Panel text commands
Class Browser Object panels dialog currentClass currentMethod
*
Methods MetaPanel 'all'
  new: browser
    | newPanel |
    newPanel <- super new.
    newPanel init: browser.
    ^ newPanel
]
*
Methods Panel 'all'
  init: browser
    browser <- browser.
    super init.
    ^ self
|
  widget
    ^ widget
]
*
Methods NavigationPanel 'all'
  init: browser
    | classList methodList currentClass currentMethod |
    classList <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '200x50'.
    methodList <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '200x50'. 
    panes <- Array new: 2.
    panes at: 1 put: classList; at: 2 put: methodList.
    widget <- Split with: classList and: methodList.
    classList setActionCallback: [ :h :className :item :state | 
      (state = 1) ifTrue: [
        currentClass <- (className asSymbol) value.
        self updateMethodList: methodList with: currentClass 
      ]
    ].
    methodList setActionCallback: [ :h :methodName :item :state | 
      (state = 1) ifTrue: [
        currentMethod <- (currentClass methodNamed: (methodName asSymbol)).
        browser showMethod: currentMethod on: currentClass
      ]
    ].
    self updateClassList: classList.
    ^ self
|
  updateMethodList: list with: class
    | index |
    self clearList: list.
    index <- 1.
    (class methods) binaryDo: [ :x :y |
      list at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  updateClassList: list
    | index |
    " Initialise the class list "
    self clearList: list.
    index <- 1.
    ((classes select: [ :x | x isMeta not ]) sort: [ :x :y | (x asString) < (y asString) ] ) do: [ :x |
      list at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  clearList: list
    list at: 'REMOVEITEM' put: (cPointer new).
    ^ self
]
*
Methods ButtonPanel 'all'
  init: browser
    | button |
    widget <- HBox new.
    #(browse hierarchy variables implementors inheritance senders versions view) do: [ :name |
      button <- ButtonControl new: (name asString).
      widget addChild: button
    ].
    ^ self
]
Methods DefinitionPanel 'all'
  init: browser
    | buttons button |
    widget <- VBox new.
    text <- TextControl new; at: 'EXPAND' put: 'YES'; at: 'MULTILINE' put: 'YES'.
    buttons <- HBox new.
    #(browse hierarchy variables implementors inheritance senders versions view) do: [ :name |
      button <- ButtonControl new: (name asString); at: 'EXPAND' put: 'HORIZONTAL'.
      buttons addChild: button
    ].
    commands <- Dictionary new.
    text setCallbackFor: 'K_ANY' as: [ :h :c | self processCommand: c ] returning: #int taking: #(cObject int).
    text setCallbackFor: 'BUTTON_CB' as: [ :h :button :pressed :x :y :status | self textPopupMenuAt: x and: y ] returning: #int taking: #(cObject int int int int string).
    widget addChild: buttons.
    widget addChild: text.
    ^ self
|
  textPopupMenuAt: x and: y
    | menu item |
    menu <- Menu new.
    menu addChild: (ItemControl new: 'Accept').
    IUP instance IupPopup: (menu handle) at: x and: y
|
  showMethod: method on: class
    commands at: 595 put: [ 
      self saveMethod: method on: class 
    ].
    text at: 'SELECTION' put: 'ALL'.
    ((text at: 'SELECTION') isEmpty)
      ifTrue: [
        text insert: (method text)
      ]
      ifFalse: [
        text at: 'SELECTEDTEXT' put: (method text)
      ]
|
  processCommand: c
    ^ (commands includesKey: c) ifTrue: [
      (commands at: c) value
    ] ifFalse: [ nil ]
|
  saveMethod: method on: class
    method text: (text at: 'VALUE').
    (method compileWithClass: class)
    ifTrue: [ ^ true ]
    ifFalse: [ stderr print: 'Error compiling method' ].
    ^ false
]
*
Methods MetaBrowser 'all'
  new
    | newObj |
    newObj <- super new.
    newObj panels: newObj panels.
    newObj init.
    ^ newObj
|
  test
    Browser new; show. IUP instance IupMainLoop
]
*
Methods Browser 'all'
  init
    | container |
    container <- Split with: ((panels at: 1) widget) and: ((panels at: 3) widget); at: 'ORIENTATION' put: 'HORIZONTAL'. 
    dialog <- Dialog new: container; at: 'SIZE' put: '400x300'; at: 'SHRINK' put: 'YES'; at: 'TITLE' put: 'Tumbleweed Browser v0.1'.
    ^ self
|
  panels
    ^ Array new: 3; at: 1 put: (self navigationPanel); 
                    at: 2 put: (self buttonPanel); 
                    at: 3 put: (self definitionPanel)
|
  panels: thePanels
    panels <- thePanels.
    ^self
|
  navigationPanel
    ^ NavigationPanel new: self
|
  buttonPanel
    ^ ButtonPanel new: self
|
  definitionPanel
    ^ DefinitionPanel new: self
|
  showMethod: method on: class
    | defPnl |
    defPnl <- panels at: 3.
    defPnl showMethod: method on: class
|
  clearMethodList
    methodList at: 'REMOVEITEM' put: (cPointer new)
|
  show
    dialog show.
]
