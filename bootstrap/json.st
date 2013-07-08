
Class JSONDumper Object

Methods JSONDumper 'json'
  toJSON: anObject
   "I'm returning a JSON string which represents the object."
   ^anObject toJSON
]

Methods Number 'json'
  toJSON
  "I return the Number in a JSON compatible format as String."
  ^(self asFloat) printString
]

Methods Integer 'json'
  toJSON
  "I return the Integer in a JSON compatible format as String."
  ^self printString
]

Methods Dictionary 'json'
  toJSON
  "I encode my contents (key/value pairs) to a JSON object and return it as String."
   | ws f |
   ws <- ''.
   ws <- ws , ${.
   f <- true.
   self keysAndValuesDo: [ :key :val |
      f ifFalse: [ ws <- ws , $, ].
      ws <- ws , (key toJSON).
      ws <- ws , $:.
      ws <- ws , val toJSON.
      f <- false
   ].
   ws <- ws , $}.
   ^ws
]

Methods String 'json'
  toJSON
  "I will encode me as JSON String and return a String containing my encoded version."
   ^('"', self, '"')
]

Methods UndefinedObject'json'
  toJSON
  "I'm returning my corresponding value as JSON String."
  ^'null'
]

Methods True 'json'
  toJSON
  "I'm returning the JSON String for trueness."
  ^'true'
]

Methods False 'json'
  toJSON
  "I'm returning the JSON String for falseness."
  ^'false'
]
