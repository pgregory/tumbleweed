Class Panel Object browser widget
Class   NavigationPanel Panel panes
Class   ButtonPanel Panel buttons
Class   DefinitionPanel Panel 
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
    | classList methodList currentClass |
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
    | text |
    widget <- VBox new.
    text <- TextControl new; at: 'EXPAND' put: 'YES'.
    widget addChild: text.
    ^ self
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
    dialog <- Dialog new: container; at: 'SIZE' put: '400x100'; at: 'SHRINK' put: 'YES'.
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
    debug at: 'SELECTION' put: 'ALL'.
    ((debug at: 'SELECTION') isEmpty)
      ifTrue: [
        debug insert: (method text)
      ]
      ifFalse: [
        debug at: 'SELECTEDTEXT' put: (method text)
      ]
|
  clearMethodList
    methodList at: 'REMOVEITEM' put: (cPointer new)
|
  show
    dialog show.
]
