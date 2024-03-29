; Don't mind this program, it is all nonsense
; comments can be made with a ";"
Store result
input 0x0
; 0x followed by a number represents hexidecimal numbers
; not having a 0x implies that the number is decimal
exampleLabel, store x
; labels are non-keywords followed by a ,
; labels can also be a : like in normal assembly languages
exampleLabel2:
input 0X0
store y

load y
skipcond 0x000 

subt y
subt y
store y
add one
store negflag
jump loop

         store negflag
         load y
         skipcond 0x400
         jump loop 
         jump stop
loop,    load result
         add x
         store result

         load y
         subt one
         store y
	
         skipcond 0x400  
		 jump loop 

load negflag
skipcond 0x800
jump stop

load result
subt result
subt result
store result

stop,    load result
         output 0x0	
         halt 0x0

x, 0x0
y, 0x0
one, 0x1
negflag, 0x0
result, 0x0 ; testing 123
