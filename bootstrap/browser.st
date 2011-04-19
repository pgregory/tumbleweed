Class Browser Object classCatList classList methodCatList methodList debug dialog
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
    | topHBox middleHBox vBox theClass |
    controls <- Dictionary new.
    classCatList <- ListControl new; at: 'EXPAND' put: 'YES'.
    classList <- ListControl new; at: 'EXPAND' put: 'YES'; at: 'VISIBLELINES' put: '10'.
    methodCatList <- ListControl new; at: 'EXPAND' put: 'YES'.
    methodList <- ListControl new; at: 'EXPAND' put: 'YES'.
    debug <- TextControl new; at: 'EXPAND' put: 'YES'; at: 'MULTILINE' put: 'YES'.
    topHBox <- HBox new.
    middleHBox <- HBox new.
    vBox <- VBox new.
    topHBox addChild: classCatList.
    topHBox addChild: classList.
    topHBox addChild: methodCatList.
    topHBox addChild: methodList.
    vBox addChild: topHBox.
    vBox addChild: middleHBox.
    vBox addChild: debug.
    dialog <- Dialog new: vBox.
    classList setActionCallback: [ :h :className :item :state | 
      (state = 1) ifTrue: [
        theClass <- (className asSymbol) value.
        self updateMethodList: theClass 
      ]
    ].
    self updateClassList.
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
    debug insert: ('Changing to ', (theClass name asString), Char cr, Char lf).
    clearMethodList.
    index <- 1.
    (theClass methods) binaryDo: [ :x :y |
      methodList at: (index asString) put: (x asString). index <- index + 1 ].
    ^ self
|
  clearMethodList
    ^ IUP instance IupSetNullAttribute: 'REMOVEITEM' on: (methodList handle) 
|
  show
    dialog show.
]
