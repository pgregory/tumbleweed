RawClass MetaModule Class MetaObject modules
Class Module Object name

Methods MetaModule 'creation'
  require: name
  | newmodule file |
    file <- File new.
    file mode: 'r'; name: ('modules/', name, '/', name, '.st').
    (file exists) ifTrue: [
      file fileIn.
      "TODO: Need to get succes/fail from the fileIn"
      file close.
      newmodule <- Module new.
      newmodule name: name.
      modules isNil ifTrue: [
        modules <- Dictionary new
      ].
      modules at: name put: newmodule.
      ^ true
    ].
    ^ false
]
*
Methods MetaModule 'misc'
  modules
    ^ modules
]
*
Methods Module 'all'
  printString
    ^('Module ', name)
|
  name: aString
    name <- aString
]
