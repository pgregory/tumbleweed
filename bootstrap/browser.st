Class Panel Object browser widget
Class   NavigationPanel Panel panes
Class   ButtonPanel Panel buttons
Class   DefinitionPanel Panel text
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
        browser showMethod: currentMethod
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
    (classes select: [ :x | x isMeta not ]) do: [ :x |
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
    widget <- HBox new.
    ^ self
]
Methods DefinitionPanel 'all'
  init: browser
    widget <- VBox new.
    text <- TextControl new; at: 'EXPAND' put: 'YES'; at: 'MULTILINE' put: 'YES'.
    widget addChild: text.
    ^ self
|
  showMethod: method
    text at: 'SELECTION' put: 'ALL'.
    ((text at: 'SELECTION') isEmpty)
      ifTrue: [
        text insert: (method text)
      ]
      ifFalse: [
        text at: 'SELECTEDTEXT' put: (method text)
      ]
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
  showMethod: method
    | defPnl |
    defPnl <- panels at: 3.
    defPnl showMethod: method
|
  clearMethodList
    methodList at: 'REMOVEITEM' put: (cPointer new)
|
  show
    dialog show.
]
