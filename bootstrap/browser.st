Class Panel Object browser
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
Methods Panel 'all'
  init: browser
    browser <- browser
    super init.
    ^ self
]
Methods NavigationPanel 'all'
  init
    | hbox classList methodList currentClass |
    classList <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '200x50'.
    methodList <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '200x50'. 
    hbox <- Split with: classList and: methodList.
    classList setActionCallback: [ :h :className :item :state | 
      (state = 1) ifTrue: [
        currentClass <- (className asSymbol) value.
        self updateMethodList: methodList with: currentClass 
      ]
    ].
    ^ self
|
  updateMethodList: list with: class
    | index |
    self clearMethodList.
    index <- 1.
    (class methods) binaryDo: [ :x :y |
      list at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  clearList: list
    list at: 'REMOVEITEM' put: (cPointer new)
    ^ self
*
Methods MetaBrowser 'all'
  new
    | newObj |
    newObj <- super new.
    newObj init.
    ^ newObj
|
  test
    Browser new; show. IUP instance IupMainLoop
]
*
Methods Browser 'all'
  init
    | topHBox middleHBox vBox |
    controls <- Dictionary new.
    classCatList <- ListControl new; at: 'EXPAND' put: 'YES'.
    classList <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '200x50'.
    methodCatList <- ListControl new; at: 'EXPAND' put: 'YES'.
    methodList <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'SIZE' put: '200x50'.
    debug <- TextControl new; at: 'EXPAND' put: 'YES'; at: 'MULTILINE' put: 'YES'.
    topHBox <- Split with: classList and: methodList.
    middleHBox <- HBox new.
    vBox <- Split with: topHBox and: debug; at: 'ORIENTATION' put: 'HORIZONTAL'.
    vBox at: 'SIZE' put: '400x200'.
    dialog <- Dialog new: vBox; at: 'SIZE' put: '410x230'; at: 'TITLE' put: 'Tumbleweed Browser v0.1'.
    classList setActionCallback: [ :h :className :item :state | 
      (state = 1) ifTrue: [
        currentClass <- (className asSymbol) value.
        self updateMethodList: currentClass 
      ]
    ].
    methodList setActionCallback: [ :h :methodName :item :state | 
      (state = 1) ifTrue: [
        currentMethod <- (currentClass methodNamed: (methodName asSymbol)).
        self showMethod: currentMethod 
      ]
    ].
    self updateClassList.
|
  panels
    ^ Array new: 3; at: 1 put: navigationPanel; 
                    at: 2 put: buttonPanel; 
                    at: 3 put: definitionPanel
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
  updateClassList
    | index |
    " Initialise the class list "
    index <- 1.
    (classes select: [ :x | x isMeta not ]) do: [ :x |
      classList at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  updateMethodList: theClass
    | index |
    self clearMethodList.
    index <- 1.
    (theClass methods) binaryDo: [ :x :y |
      methodList at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
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
