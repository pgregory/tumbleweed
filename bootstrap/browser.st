Class Browser Object controls dialog
*
Methods MetaBrowser 'all'
  new
    | newObj |
    newObj <- super new.
    newObj init.
    ^ newObj
]
*
Methods Browser 'all'
  init
    | topHBox middleHBox vBox index |
    controls <- Dictionary new.
    controls at: 'classCategoryList' put: ListControl new.
    controls at: 'classList' put: ListControl new.
    controls at: 'methodCategoryList' put: ListControl new.
    controls at: 'methodList' put: ListControl new.
    topHBox <- HBox new.
    middleHBox <- HBox new.
    vBox <- VBox new.
    topHBox addChild: (controls at: 'classCategoryList').
    topHBox addChild: (controls at: 'classList').
    topHBox addChild: (controls at: 'methodCategoryList').
    topHBox addChild: (controls at: 'methodList').
    (controls at: 'classCategoryList') at: 'EXPAND' put: 'YES'.
    (controls at: 'classList') at: 'EXPAND' put: 'YES'.
    (controls at: 'classList') at: 'VISIBLELINES' put: '10'.
    (controls at: 'methodCategoryList') at: 'EXPAND' put: 'YES'.
    (controls at: 'methodList') at: 'EXPAND' put: 'YES'.
    vBox addChild: topHBox.
    vBox addChild: middleHBox.
    dialog <- Dialog new: vBox.
    " Initialise the class list "
    index <- 1.
    (classes select: [ :x | x isMeta not ]) do: [ :x |
      (controls at: 'classList') at: (index asString) put: (x asString). index <- index + 1 ].
|
  show
    dialog show.
]
